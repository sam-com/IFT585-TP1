// IFT585-TP1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <thread>
#include <bitset>
#include <mutex> 
#include <chrono>
#include <iterator>
#include <math.h>
#include <algorithm>
#include "TamponCirculaire.h"
#include "EntreeFichier.h"
#include "SortieFichier.h"
#include "Hamming.h"

using namespace std;


void afficherCommentaireEtTrame(const string s, Trame); // Utiliser afficher une string avec de la sync et une trame 
void afficher(const string s);

size_t	tailleTempon;
long long	delaiTemporisation;
int numeroTrame;		//utilisée pour identifier les trames et incorporer les erreurs
string	fichierACopier;
string	fichierDestination;
mutex m;		// pour interdit l'ecriture en meme entre thread emission && supportTransmission
mutex a;        // pour interdit l'ecriture en meme temps pour l'affichge
TamponCirculaire tamponE;
TamponCirculaire tamponR;
map<int, int> erreurs;

enum ModeHamming {
	DETECTION, CORRECTION
};

enum EtatLiaison {
	PAS_UTILISE, FINI, ENVOY_T1, ENVOY_T2, RECEPTION_T1, RECEPTION_T2
};

enum EtatSupport {
	MESSAGE_RECU, PRET_ENVOYER, ATTENTE
};

struct Connection {
	EtatLiaison& etatLiaison;
	EtatSupport& etatSupport;
	Trame& trame;
};

ModeHamming modeHamming;
//Structure contenant le début du timer et s'il à déjà été vérifier (désactive le timer)
struct Delai {
	Delai() {
		verifie = verifie = true;
	}
	chrono::high_resolution_clock::time_point tempsDebut;
	bool verifie;
};

map<int, int> lireFichierErreurs(const string& nomFichierErreurs) {
	map<int, int> listeErreurs;
	ifstream ifs{ nomFichierErreurs };

for (string s; ifs >> s; ) {
	std::pair<int, int> erreur = std::pair<int, int>(std::stoi(s.substr(0, s.find(":"))), std::stoi(s.substr(s.find(":") + 1, s.length())));
	listeErreurs.emplace(erreur);
}
return listeErreurs;
}

void envoyerTrame(Connection& connection, Trame& trame, bool emetteur) {
	lock_guard<mutex> av(m);

	connection.trame = trame;		// On met la trame à envoyer dans la connexion
	if (emetteur) {
		connection.etatLiaison = EtatLiaison::ENVOY_T1;	// On change l'état de liaison pour "trame envoye"
	}
	else {
		connection.etatLiaison = EtatLiaison::ENVOY_T2;	// On change l'état de liaison pour "trame envoye"
	}

	string tmp = emetteur ? "emmeteur" : "recepteur";
	afficherCommentaireEtTrame("Envoye " + tmp, connection.trame);

	connection.trame.setContenu(Hamming::encoder(connection.trame.getContenu()));
}

Trame recevoirTrame(Connection& connection) {
	lock_guard<mutex> av(m);

	connection.etatLiaison = EtatLiaison::PAS_UTILISE;
	return connection.trame;
}


bool detecter(Trame& trame) {
	lock_guard<mutex> av(m);

	return Hamming::detecter(trame.getContenu());
}

void corriger(Trame& trame) {
	lock_guard<mutex> av(m);

	trame.setContenu(Hamming::corriger(trame.getContenu()));
}

void decoder(Trame& trame) {
	lock_guard<mutex> av(m);

	trame.setContenu(Hamming::decoder(trame.getContenu()));
}

void obtenirEtatLiaison(Connection& connection, bool emetteur) {
	lock_guard<mutex> av(m);
	if (connection.etatLiaison == EtatLiaison::PAS_UTILISE) {    // pret a envoyer
		connection.etatSupport = EtatSupport::PRET_ENVOYER;
	}
	else if (emetteur && connection.etatLiaison == EtatLiaison::RECEPTION_T1) {
		connection.etatSupport = EtatSupport::MESSAGE_RECU;
	}
	else if (!emetteur && connection.etatLiaison == EtatLiaison::RECEPTION_T2) {
		connection.etatSupport = EtatSupport::MESSAGE_RECU;
	}
	else { // sinon il peut juste attendre .. 
		connection.etatSupport = EtatSupport::ATTENTE;
	}
}

int delaiEchu(vector<Delai> d) {
	int count = 0;
	for (Delai delai : d) {
		if (!delai.verifie && 
			chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - delai.tempsDebut).count() > delaiTemporisation) {
			return count;
		}
		count++;
	}
	return -1;
}

void validateTrames(TamponCirculaire& tampon, vector<Delai>& delais, uint16_t seq) {
	uint16_t debut = tampon.getSeqDebutFenetre();
	for (int i = debut; i <= seq; i++) {
		tampon.validerTrame(i);
		delais[i].verifie = true;
	}
}

void EmetterRecepteur(string fichierEntree, string fichierSortie, Connection& connection, bool emetteur) {
	numeroTrame = 0;
	uint16_t maxSeq = (int)pow(2, SEQ_SIZE);
	TamponCirculaire tampon = TamponCirculaire(tailleTempon, maxSeq / 2);	// Le tampon, taille de tailleTampon, fenetre de 4
	vector<Delai> delais = vector<Delai>();		// La array pour savoir s'il y a un timeout
	delais.resize((int)pow(2, SEQ_SIZE));		// delais[2] = Delai de seq 2

	bool trameNakRecu = false;					// Si une trame NAK à été recu, donc on doit l'envoyer un priorité prochaine boucle
	bool trameDoitEtreEnvoye = false;			// Si une trame ACK doit être envoyer à l'émetteur
	bool nakEnvoye = false;
	uint16_t seqTrameNAKRecu;					// La séquence de la trame NAK recu, pour aller le chercher dans le tampon
	Trame trameAEnvoyer;						// La trame ACK à envoyer, pour envoyer lorsque le support physique est libre
	EntreeFichier entree = EntreeFichier(fichierEntree);
	SortieFichier sortie = SortieFichier(fichierSortie);
	int sequenceCourante = 0;					// La séquence courante, pour envoyer les trame en ordre

	// Tant qu'on à pas fini d'envoyer 
	while (connection.etatLiaison != EtatLiaison::FINI) {
		obtenirEtatLiaison(connection, emetteur); // mets la bonne etat de liasion dans la connection
		switch (connection.etatSupport) {		// Selon l'état du support
		case EtatSupport::PRET_ENVOYER:			// Si on peut envoyer sur le support
			if (trameNakRecu) {					// Si on avait recu un NAK
				Trame trame = tampon.get(seqTrameNAKRecu);		// On va chercher la trame dans le tampon
				delais[sequenceCourante].tempsDebut = chrono::high_resolution_clock::now();	// On start un timer pour la trame
				delais[sequenceCourante].verifie = false;
				envoyerTrame(connection, trame, emetteur);
				trameNakRecu = false;
			}
			else if (trameDoitEtreEnvoye) {					// Si une trame ACK doit être envoyer (la couche recepteur)
				envoyerTrame(connection, trameAEnvoyer, emetteur);
				trameDoitEtreEnvoye = false;
			}
			else if (delaiEchu(delais) != -1) {
				uint16_t seqEchu = delaiEchu(delais);
				Trame trame = tampon.get(seqEchu);								// On va chercher la trame dans le tampon
				delais[seqEchu].tempsDebut = chrono::high_resolution_clock::now();
				envoyerTrame(connection, trame, emetteur);
			}
			else if (entree.isValid() && !entree.finFichierAtteint()) {	// Si cet station copiait un fichier
																		// Si la fenetre nous permet d'envoyer une autre trame
				if (tampon.peutInserer(sequenceCourante)) {
					// On va chercher les 2 prochains octets dans le fichier
					Trame trame = Trame(TYPE_DONNEES, sequenceCourante, entree.getNext16Bits());
					tampon.ajouter(trame);			// On l'ajoute dans le tampon
					delais[sequenceCourante].tempsDebut = chrono::high_resolution_clock::now();	// On start un timer pour la trame
					delais[sequenceCourante].verifie = false;
					envoyerTrame(connection, trame, emetteur);
					sequenceCourante = (sequenceCourante + 1) % maxSeq;	// On incrémente la séquence (doit être entre 0 et 7)
				}
			}
			break;
		case EtatSupport::MESSAGE_RECU:       // Si le support à envoyer quelque chose
			Trame trame = recevoirTrame(connection);
			string tmp = emetteur ? "emmeteur" : "recepteur";
			bool erreurDetecter = detecter(trame);
			if (!nakEnvoye && erreurDetecter && modeHamming == DETECTION) {
				afficher("Recu " + tmp + ", erreur detecte");
				trameDoitEtreEnvoye = true;
				trameAEnvoyer = Trame(TYPE_NAK, tampon.getSeqDebutFenetre(), 0);
				nakEnvoye = true;
			}
			else {
				if (!nakEnvoye && erreurDetecter && modeHamming == CORRECTION) {
					afficher("Recu " + tmp + ", erreur detecte et corrige");
					corriger(trame);
				}
				decoder(trame);
				afficherCommentaireEtTrame("Recu " + tmp + " :", trame);
				if ((!erreurDetecter || modeHamming == CORRECTION) && trame.getType() == TYPE_ACK) {					// Si la trame reçu est de type ACK
					validateTrames(tampon, delais, trame.getSequence());
					
					if (entree.isValid() && entree.finFichierAtteint()) {
						connection.etatLiaison = EtatLiaison::FINI;
					}
					else if (tampon.estDernierDeFenetre(trame.getSequence())) {	// Si la trame est le dernier de la fenetre
						tampon.deplacerFenetre();								// On peu deplacer la fenetre (de taille fenetre position)
					}
				}
				else if ((!erreurDetecter || modeHamming == CORRECTION) && trame.getType() == TYPE_NAK) {					// Si la trame reçu est de type NAK
					delais[trame.getSequence()].verifie = true;				// On stop le timer (on va le repartir quand on va 
					trameNakRecu = true;									// renvoyer la trame)
					seqTrameNAKRecu = trame.getSequence();					// On indique qu'à la prochaine boucle on veut renvoyer la trame
				}
				else if ((!erreurDetecter || modeHamming == CORRECTION) && trame.getType() == TYPE_DONNEES) {					// Si la trame reçu est de type DONNEES
					int c = trame.getSequence();
					if (tampon.estDebutDeFenetre(trame.getSequence())) {	// Si la trame est la trame voulu (premiere de la fenetre)
						nakEnvoye = false;
						uint16_t lastSeq = trame.getSequence();				// La séquence du ACK à envoyer
						uint16_t nbDeplacementFenetre = 1;					// Le nombre de position que la fenêtre doit être déplacé
						if (sortie.isValid()) {								// Si on peut ecrire dans le fichier
							sortie.send16Bits(trame.getDonnees());			// On envoie les données
																			// Pour les trames deja dans le tampon (quand on recoit en désordre)
							for (Trame& t : tampon.getListeTrameNonValideDansFenetre()) {
								tampon.validerTrame(t.getSequence());		// On met a validated pour pouvoir remplacer plus tard
								sortie.send16Bits(t.getDonnees());			// On envoi les données à la couche supérieur
								lastSeq = t.getSequence();					// On prend la bonne séquence pour le ACK
								nbDeplacementFenetre++;
							}
						}

						tampon.deplacerFenetre(nbDeplacementFenetre);
						trameDoitEtreEnvoye = true;
						trameAEnvoyer = Trame(TYPE_ACK, lastSeq, 0);
					}
					// Si c'est pas la trame qu'on voulait, on doit envoyer un NAK
					else {
						// On indique qu'une trame doit être envoyer quand le support va etre libre
						trameDoitEtreEnvoye = true;
						trameAEnvoyer = Trame(TYPE_NAK, tampon.getSeqDebutFenetre(), 0);
						// Si on pouvait l'insérer dans la fenetre, on l'insère
						if (tampon.peutInserer(trame.getSequence())) {
							tampon.ajouter(trame);
						}
					}
				}
			}
			break;
		}
	}

	entree.close();
	sortie.close();
	cout << "FINI" << endl;
}

void supportTransmission(Connection& connectionT1, Connection& connectionT2) {

	while (connectionT1.etatLiaison != FINI) {
		if (connectionT1.etatLiaison == EtatLiaison::ENVOY_T1) {

			lock_guard<mutex> av(m);
			//afficherCommentaireEtTrame("T1->T2 :", connectionT1.trame);

			if (erreurs.count(numeroTrame) != 0) {
				connectionT1.trame.flipBitAPosition(erreurs.at(numeroTrame));
				string logErreur = "insertion d'une erreur à la trame " + std::to_string(numeroTrame) + " a la position " + std::to_string(erreurs.at(numeroTrame));
				afficher(logErreur);
			}

			connectionT2.trame = connectionT1.trame;
			connectionT1.etatLiaison = EtatLiaison::RECEPTION_T2;

			numeroTrame += 1;			// On incrémente le numéro de trame pour l'injection d'erreurs sur le support de transmission

		}
		else if (connectionT2.etatLiaison == EtatLiaison::ENVOY_T2) {

			lock_guard<mutex> av(m);
			//afficherCommentaireEtTrame("T2->T1 :", connectionT2.trame);

			if (erreurs.count(numeroTrame) != 0) {
				connectionT2.trame.flipBitAPosition(erreurs.at(numeroTrame));
				string logErreur = "insertion d'une erreur à la trame " + std::to_string(numeroTrame) + " a la position " + std::to_string(erreurs.at(numeroTrame));
				afficher(logErreur);
			}

			numeroTrame += 1;			// On incrémente le numéro de trame pour l'injection d'erreurs sur le support de transmission

			connectionT1.trame = connectionT2.trame;
			connectionT1.etatLiaison = EtatLiaison::RECEPTION_T1;
		}
	}
}

int main()
{
	ifstream fichierParametres;
	fichierParametres.open("parametres.txt");

	bool valide = true;
	string line;
	if (fichierParametres.is_open()) {
		tailleTempon = getline(fichierParametres, line) ? atoi(line.c_str()) : 0;
		delaiTemporisation = getline(fichierParametres, line) ? atoi(line.c_str()) : -1;
		fichierACopier = getline(fichierParametres, line) ? line : "";
		fichierDestination = getline(fichierParametres, line) ? line.c_str() : "";
		modeHamming = getline(fichierParametres, line) ? (line == "correction" ? CORRECTION : DETECTION) : DETECTION;

		if (tailleTempon < 1 || delaiTemporisation <= 0 || fichierACopier.empty() || fichierDestination.empty()) {
			valide = false;
		}
	}
	else {
		cout << "Fichier de paramètres introuvable.";
	}

	cout << "Taille du tempon: " << tailleTempon << endl
		<< "Delai de temporisation (ms): " << delaiTemporisation << endl
		<< "Fichier a copier: " << fichierACopier << endl
		<< "Fichier de destination: " << fichierDestination << endl << endl;

	erreurs = lireFichierErreurs("erreurs.txt");

	// Un seul etat de liaison pour tout le programme (pour la sync) 
	// Est-ce que sa fait vraiment du sense que cette varaible soit dans la struct connection ... 
	EtatLiaison etatL = EtatLiaison::PAS_UTILISE;

	EtatSupport etatS1 = EtatSupport::PRET_ENVOYER;
	EtatSupport etatS2 = EtatSupport::PRET_ENVOYER;
	Connection connection1 = { etatL, etatS1, Trame(TYPE_VALIDATED, 0, 0) };
	Connection connection2 = { etatL, etatS2, Trame(TYPE_VALIDATED, 0, 0) };

	std::thread th1(EmetterRecepteur, fichierACopier, "", connection1, true);
	std::thread th2(supportTransmission, connection1, connection2);
	std::thread th3(EmetterRecepteur, "", fichierDestination, connection2, false);

	th1.join();
	th2.join();
	th3.join();

	system("pause");
}

void afficherCommentaireEtTrame(const string s, Trame trame) {
	lock_guard<mutex> vr{ a };
	cout << s << " ( sequence :" << trame.getSequence() << ", type :";
	if (trame.getType() == TYPE_ACK) {
		cout << "ACK";
	}
	else if (trame.getType() == TYPE_NAK) {
		cout << "NAK";
	}
	else {
		cout << "DONNEES";
	}
	cout << ", donnees :";
	uint16_t donnees = trame.getDonnees();
	cout	<< (char) (donnees >> 8)
			<< (char) donnees
			<< endl;
}

void afficher(const string s) {
	lock_guard<mutex> vr{ a };
	cout << s << endl;
}