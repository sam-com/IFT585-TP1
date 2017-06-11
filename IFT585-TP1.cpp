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
#include <algorithm>
#include "TamponCirculaire.h"
#include "EntreeFichier.h"
#include "SortieFichier.h"
#include "Hamming.h"

using namespace std;


void afficherCommentaireEtTrame(const string s, Trame); // Utiliser afficher une string avec de la sync et une trame 
void afficher(const string s);

size_t	tailleTempon;
int		delaiTemporisation;
int numeroTrame;		//utilis�e pour identifier les trames et incorporer les erreurs
string	fichierACopier;
string	fichierDestination;
mutex m;		// pour interdit l'ecriture en meme entre thread emission && supportTransmission
mutex a;        // pour interdit l'ecriture en meme temps pour l'affichge
map<int, int> erreurs;

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

//Structure contenant le d�but du timer et s'il � d�j� �t� v�rifier (d�sactive le timer)
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

	connection.trame = trame;		// On met la trame � envoyer dans la connexion
	if (emetteur) {
		connection.etatLiaison = EtatLiaison::ENVOY_T1;	// On change l'�tat de liaison pour "trame envoye"
	}
	else {
		connection.etatLiaison = EtatLiaison::ENVOY_T2;	// On change l'�tat de liaison pour "trame envoye"
	}

	string tmp = emetteur ? "emmeteur" : "recepteur";
	afficherCommentaireEtTrame("Envoye " + tmp + " :", connection.trame);
	connection.trame.setContenu(Hamming::encoder(connection.trame.getContenu()));
}

Trame recevoirTrame(Connection& connection, bool emetteur) {
	lock_guard<mutex> av(m);

	string tmp = emetteur ? "emmeteur" : "recepteur";
	connection.trame.setContenu(Hamming::decoder(connection.trame.getContenu()));
	afficherCommentaireEtTrame("Recu " + tmp + " :", connection.trame);

	connection.etatLiaison = EtatLiaison::PAS_UTILISE;
	return connection.trame;
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

void EmetterRecepteur(string fichierEntree, string fichierSortie, Connection& connection, bool emetteur) {
	numeroTrame = 0;
	uint16_t maxSeq = (int)pow(2, SEQ_SIZE);							// Le max de s�quence, 2^3 = 8 (0 � 8)
	TamponCirculaire tampon = TamponCirculaire(tailleTempon, maxSeq / 2);	// Le tampon, taille de tailleTampon, fenetre de 4
	Delai delais[SEQ_SIZE];												// La array pour savoir s'il y a un timeout
																		// delais[2] = Delai de seq 2
	bool trameNakRecu = false;					// Si une trame NAK � �t� recu, donc on doit l'envoyer un priorit� prochaine boucle
	bool trameDoitEtreEnvoye = false;			// Si une trame ACK doit �tre envoyer � l'�metteur
	uint16_t seqTrameNAKRecu;					// La s�quence de la trame NAK recu, pour aller le chercher dans le tampon
	Trame trameAEnvoyer;				// La trame ACK � envoyer, pour envoyer lorsque le support physique est libre
	EntreeFichier entree = EntreeFichier(fichierEntree);
	SortieFichier sortie = SortieFichier(fichierSortie);
	int sequenceCourante = 0;					// La s�quence courante, pour envoyer les trame en ordre

												// Tant qu'on � pas fini d'envoyer 
												// TODO: Pas tester et fait tr�s rapidement, la condition du while est surement pas bonne, quant est-il du r�cepteur?
												// C'est quand qu'il � finit?
	while (connection.etatLiaison != EtatLiaison::FINI) {
		obtenirEtatLiaison(connection, emetteur); // mets la bonne etat de liasion dans la connection
		switch (connection.etatSupport) {		// Selon l'�tat du support
		case EtatSupport::PRET_ENVOYER:			// Si on peut envoyer sur le support
			if (trameNakRecu) {					// Si on avait recu un NAK
				Trame trame = tampon.get(seqTrameNAKRecu);		// On va chercher la trame dans le tampon
				delais[sequenceCourante].tempsDebut = chrono::high_resolution_clock::now();	// On start un timer pour la trame
				delais[sequenceCourante].verifie = false;
				envoyerTrame(connection, trame, emetteur);
				trameNakRecu = false;
			}
			else if (trameDoitEtreEnvoye) {					// Si une trame ACK doit �tre envoyer (la couche recepteur)
															//  connection.trame = trameAEnvoyer;
															//   connection.etatLiaison = EtatLiaison::ENVOYE;
				envoyerTrame(connection, trameAEnvoyer, emetteur);
				trameDoitEtreEnvoye = false;
			}
			// TIMEOUT else if ()								// Si une trame n'a pas eu de ACK apres un certain temps
			else if (entree.isValid() && !entree.finFichierAtteint()) {	// Si cet station copiait un fichier
																		// Si la fenetre nous permet d'envoyer une autre trame
				if (tampon.peutInserer(sequenceCourante)) {
					// On va chercher les 2 prochains octets dans le fichier
					Trame trame = Trame(TYPE_DONNEES, sequenceCourante, entree.getNext16Bits());
					tampon.ajouter(trame);			// On l'ajoute dans le tampon
					delais[sequenceCourante].tempsDebut = chrono::high_resolution_clock::now();	// On start un timer pour la trame
					delais[sequenceCourante].verifie = false;
					envoyerTrame(connection, trame, emetteur);
					sequenceCourante = (sequenceCourante + 1) % maxSeq;	// On incr�mente la s�quence (doit �tre entre 0 et 7)*/
				}
			}
			else if (emetteur) {
				connection.etatLiaison = EtatLiaison::FINI;		// Si le fichier est fini ou non valide, on a finit d'envoyer
			}
			break;
		case EtatSupport::MESSAGE_RECU:       // Si le support � envoyer quelque chose
			Trame trame = recevoirTrame(connection, emetteur);
			if (trame.getType() == TYPE_ACK) {					// Si la trame re�u est de type ACK
				delais[trame.getSequence()].verifie = true;			// On arr�te le timer
				tampon.validerTrame(trame.getSequence());			// Met le type de trame a VALIDATED
				cout << "Envoi de ack " << trame.getSequence() << " par recepteur";
				if (tampon.estDernierDeFenetre(trame.getSequence())) {	// Si la trame est le dernier de la fenetre
					tampon.deplacerFenetre();							// On peu deplacer la fenetre (de taille fenetre position)
				}
			}
			else if (trame.getType() == TYPE_NAK) {					// Si la trame re�u est de type NAK
				delais[trame.getSequence()].verifie = true;				// On stop le timer (on va le repartir quand on va 
				trameNakRecu = true;									// renvoyer la trame)
				seqTrameNAKRecu = trame.getSequence();					// On indique qu'� la prochaine boucle on veut renvoyer la trame
			}
			else if (trame.getType() == TYPE_DONNEES) {					// Si la trame re�u est de type DONNEES
																		// TODO CORRECTION/DETECTION DERREUR selon le mode en param�tre
				if (tampon.estDebutDeFenetre(trame.getSequence())) {	// Si la trame est la trame voulu (premiere de la fenetre)
					uint16_t lastSeq = trame.getSequence();				// La s�quence du ACK � envoyer
					uint16_t nbDeplacementFenetre = 1;					// Le nombre de position que la fen�tre doit �tre d�plac�
					if (sortie.isValid()) {								// Si on peut ecrire dans le fichier
						sortie.send16Bits(trame.getDonnees());			// On envoie les donn�es
																		// Pour les trames deja dans le tampon (quand on recoit en d�sordre)
						for (Trame &t : tampon.getListeTrameNonValideDansFenetre()) {
							t.setType(TYPE_VALIDATED);					// On met a validated pour pouvoir remplacer plus tard
							sortie.send16Bits(t.getDonnees());			// On envoi les donn�es � la couche sup�rieur
							lastSeq = t.getSequence();					// On prend la bonne s�quence pour le ACK
							nbDeplacementFenetre++;
						}
					}

					tampon.deplacerFenetre(nbDeplacementFenetre);
					trameDoitEtreEnvoye = true;
					trameAEnvoyer = Trame(TYPE_ACK, lastSeq, 0);
				}
				// Si c'est pas la trame qu'on voulait, on doit envoyer un NAK
				else {
					// On indique qu'une trame doit �tre envoyer quand le support va etre libre
					trameDoitEtreEnvoye = true;
					trameAEnvoyer = Trame(TYPE_NAK, tampon.getSeqDebutFenetre(), 0);
					// Si on pouvait l'ins�rer dans la fenetre, on l'ins�re
					if (tampon.peutInserer(trame.getSequence())) {
						tampon.ajouter(trame);
					}
				}

				break;
			}
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
			afficherCommentaireEtTrame("T1->T2 :", connectionT1.trame);

			if (erreurs.count(numeroTrame) != 0) {
				connectionT1.trame.flipBitAPosition(erreurs.at(numeroTrame));
				string logErreur = "insertion d'une erreur � la trame " + std::to_string(numeroTrame) + " a la position " + std::to_string(erreurs.at(numeroTrame));
				afficher(logErreur);
			}

			connectionT2.trame = connectionT1.trame;
			connectionT1.etatLiaison = EtatLiaison::RECEPTION_T2;

			numeroTrame += 1;			// On incr�mente le num�ro de trame pour l'injection d'erreurs sur le support de transmission

		}
		else if (connectionT2.etatLiaison == EtatLiaison::ENVOY_T2) {

			lock_guard<mutex> av(m);
			afficherCommentaireEtTrame("T2->T1 :", connectionT2.trame);

			if (erreurs.count(numeroTrame) != 0) {
				connectionT2.trame.flipBitAPosition(erreurs.at(numeroTrame));
				string logErreur = "insertion d'une erreur � la trame " + std::to_string(numeroTrame) + " a la position " + std::to_string(erreurs.at(numeroTrame));
				afficher(logErreur);
			}

			numeroTrame += 1;			// On incr�mente le num�ro de trame pour l'injection d'erreurs sur le support de transmission

			connectionT1.trame = connectionT2.trame;
			connectionT1.etatLiaison = EtatLiaison::RECEPTION_T1;
		}
	}
}

int main()
{
	ifstream fichierParametres;
	fichierParametres.open("parametres.txt");
	Trame trame = Trame(TYPE_DONNEES, 1, 8);
	trame.print();

	bool valide = true;
	string line;
	if (fichierParametres.is_open()) {
		tailleTempon = getline(fichierParametres, line) ? atoi(line.c_str()) : 0;
		delaiTemporisation = getline(fichierParametres, line) ? atoi(line.c_str()) : -1;
		fichierACopier = getline(fichierParametres, line) ? line : "";
		fichierDestination = getline(fichierParametres, line) ? line.c_str() : "";

		if (tailleTempon < 1 || delaiTemporisation <= 0 || fichierACopier.empty() || fichierDestination.empty()) {
			valide = false;
		}
	}
	else {
		cout << "Fichier de param�tres introuvable.";
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

	EntreeFichier entreeFichier = EntreeFichier(fichierACopier);

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
	cout << ", donees :"
		<< trame.getDonnees()
		<< ")"
		<< endl;
}

void afficher(const string s) {
	lock_guard<mutex> vr{ a };
	cout << s << endl;
}