// IFT585-TP1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <bitset>
#include <mutex> 
#include <chrono>
#include <iterator>
#include <algorithm>
#include "TamponCirculaire.h"
#include "EntreeFichier.h"
#include "SortieFichier.h"
using namespace std;

#define TrameTaille 16

size_t	tailleTempon;
int		delaiTemporisation;
string	fichierACopier;
string	fichierDestination;
mutex m;		// pour interdit l'ecriture en meme entre thread emission && supportTransmission


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

//Structure contenant le début du timer et s'il à déjà été vérifier (désactive le timer)
struct Delai {
    Delai() {
        verifie = verifie = true;
    }
    chrono::high_resolution_clock::time_point tempsDebut;
    bool verifie;
};

void envoyerTrame(Connection& connection, Trame& trame, bool emetteur) {
    lock_guard<mutex> av(m);

    string tmp = emetteur ? "emmeteur" : "recepteur";
    cout << "Envoye " << tmp << endl;
    trame.print();

    connection.trame = trame;		// On met la trame à envoyer dans la connexion
    if (emetteur) {
        connection.etatLiaison = EtatLiaison::ENVOY_T1;	// On change l'état de liaison pour "trame envoye"
    }
    else {
        connection.etatLiaison = EtatLiaison::ENVOY_T2;	// On change l'état de liaison pour "trame envoye"
    }
}

Trame recevoirTrame(Connection& connection, bool emetteur) {
    lock_guard<mutex> av(m);

    string tmp = emetteur ? "emmeteur" : "recepteur";
    cout << "Recu " << tmp << endl;
    connection.trame.print();

    connection.etatLiaison = EtatLiaison::PAS_UTILISE;
    return connection.trame;
}


void EmetterRecepteur(string fichierEntree, string fichierSortie, Connection& connection, bool emetteur) {
    uint16_t maxSeq = (int)pow(2, SEQ_SIZE);							// Le max de séquence, 2^3 = 8 (0 à 8)
    TamponCirculaire tampon = TamponCirculaire(tailleTempon, maxSeq / 2);	// Le tampon, taille de tailleTampon, fenetre de 4
    Delai delais[SEQ_SIZE];												// La array pour savoir s'il y a un timeout
                                                                        // delais[2] = Delai de seq 2
    bool trameNakRecu = false;					// Si une trame NAK à été recu, donc on doit l'envoyer un priorité prochaine boucle
    bool trameDoitEtreEnvoye = false;			// Si une trame ACK doit être envoyer à l'émetteur
    uint16_t seqTrameNAKRecu;					// La séquence de la trame NAK recu, pour aller le chercher dans le tampon
    Trame trameAEnvoyer;				// La trame ACK à envoyer, pour envoyer lorsque le support physique est libre
    EntreeFichier entree = EntreeFichier(fichierEntree);
    SortieFichier sortie = SortieFichier(fichierSortie);
    int sequenceCourante = 0;					// La séquence courante, pour envoyer les trame en ordre

    // Tant qu'on à pas fini d'envoyer 
    // TODO: Pas tester et fait très rapidement, la condition du while est surement pas bonne, quant est-il du récepteur?
    // C'est quand qu'il à finit?
    while (connection.etatLiaison != EtatLiaison::FINI) {
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

        switch (connection.etatSupport) {		// Selon l'état du support
        case EtatSupport::PRET_ENVOYER:			// Si on peut envoyer sur le support
            if (trameNakRecu) {					// Si on avait recu un NAK
                Trame trame = tampon.get(seqTrameNAKRecu);		// On va chercher la trame dans le tampon
                delais[sequenceCourante].tempsDebut = chrono::high_resolution_clock::now();	// On start un timer pour la trame
                delais[sequenceCourante].verifie = false;
                //    connection.trame = trame;						// On met la trame à envoyer dans la connexion
               //     connection.etatLiaison = EtatLiaison::ENVOYE;	// On change l'état de liaison pour "trame envoye"
                envoyerTrame(connection, trame, emetteur);
                trameNakRecu = false;
            }
            else if (trameDoitEtreEnvoye) {					// Si une trame ACK doit être envoyer (la couche recepteur)
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
                    // connection.trame = trame;
                    // connection.etatLiaison = EtatLiaison::ENVOYE;
                    envoyerTrame(connection, trame, emetteur);
                    sequenceCourante = (sequenceCourante + 1) % maxSeq;	// On incrémente la séquence (doit être entre 0 et 7)*/
                }
            }
            else if (emetteur) {
                connection.etatLiaison = EtatLiaison::FINI;		// Si le fichier est fini ou non valide, on a finit d'envoyer
            }
            break;
        case EtatSupport::MESSAGE_RECU:       // Si le support à envoyer quelque chose
            Trame trame = recevoirTrame(connection, emetteur);
            if (trame.getType() == TYPE_ACK) {					// Si la trame reçu est de type ACK
                delais[trame.getSequence()].verifie = true;			// On arrête le timer
                tampon.validerTrame(trame.getSequence());			// Met le type de trame a VALIDATED
                if (tampon.estDernierDeFenetre(trame.getSequence())) {	// Si la trame est le dernier de la fenetre
                    tampon.deplacerFenetre();							// On peu deplacer la fenetre (de taille fenetre position)
                }
            }
            else if (trame.getType() == TYPE_NAK) {					// Si la trame reçu est de type NAK
                delais[trame.getSequence()].verifie = true;				// On stop le timer (on va le repartir quand on va 
                trameNakRecu = true;									// renvoyer la trame)
                seqTrameNAKRecu = trame.getSequence();					// On indique qu'à la prochaine boucle on veut renvoyer la trame
            }
            else if (trame.getType() == TYPE_DONNEES) {					// Si la trame reçu est de type DONNEES
                // TODO CORRECTION/DETECTION DERREUR selon le mode en paramètre
                if (tampon.estDebutDeFenetre(trame.getSequence())) {	// Si la trame est la trame voulu (premiere de la fenetre)
                    uint16_t lastSeq = trame.getSequence();				// La séquence du ACK à envoyer
                    uint16_t nbDeplacementFenetre = 1;					// Le nombre de position que la fenêtre doit être déplacé
                    if (sortie.isValid()) {								// Si on peut ecrire dans le fichier
                        sortie.send16Bits(trame.getDonnees());			// On envoie les données
                        // Pour les trames deja dans le tampon (quand on recoit en désordre)
                        for (Trame &t : tampon.getListeTrameNonValideDansFenetre()) {
                            t.setType(TYPE_VALIDATED);					// On met a validated pour pouvoir remplacer plus tard
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

                break;
            }
        }
    }
    cout << "FINI" << endl;
}

void supportTransmission(Connection& connectionT1, Connection& connectionT2) {

    while (connectionT1.etatLiaison != FINI) {
        if (connectionT1.etatLiaison == EtatLiaison::ENVOY_T1) {
            // Insertion d'erreur ICI
            cout << "T1 -> T2 :" << endl;
            connectionT1.trame.print();
            connectionT2.trame = connectionT1.trame;
            connectionT1.etatLiaison = EtatLiaison::RECEPTION_T2;
        }
        else if (connectionT2.etatLiaison == EtatLiaison::ENVOY_T2) {
            // Insertion d'erreur ICI
            cout << "T2 -> T1 :" << endl;
            connectionT2.trame.print();
            connectionT1.trame = connectionT2.trame;
            connectionT2.etatLiaison = EtatLiaison::RECEPTION_T1;
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
        cout << "Fichier de paramètres introuvable.";
    }

    cout << "Taille du tempon: " << tailleTempon << endl
        << "Delai de temporisation (ms): " << delaiTemporisation << endl
        << "Fichier a copier: " << fichierACopier << endl
        << "Fichier de destination: " << fichierDestination << endl << endl;

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
    //  th3.join();

    system("pause");
}