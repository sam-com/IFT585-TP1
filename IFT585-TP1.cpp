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
#include "VectorCirculaire.cpp"

using namespace std;

#define TrameTaille 16

size_t	tailleTempon;
int		delaiTemporisation;
string	fichierACopier;
string	fichierDestination;
mutex mutexT1;		// pour interdit l'ecriture en meme entre thread emission && supportTransmission
mutex mutexT3;		// pour interdit l'ecriture en meme entrethread reception && supportTransmission



enum EtatTransmission {
	ENVOYER_T1, ENVOYER_T2, VIDE, UTILISER, PAS_UTILISER, FINI
};

struct Connection {
	EtatTransmission& etat;
	uint32_t trame;
};

void emission(Connection connection) {

}

void reception(Connection connection) {

}

void supportTransmission(Connection& connectionT1,Connection& connectionT2) {
	uint32_t transport; // variable temporaire utile juste pour l'insertion d'erreur
	while (connectionT1.etat != ENVOYER_T1 || connectionT2.etat != ENVOYER_T2 || connectionT1.etat != FINI) {
		this_thread::sleep_for(std::chrono::seconds(1));
	} 

	if (connectionT1.etat == ENVOYER_T1) {
		connectionT1.etat = UTILISER;
		{
			lock_guard<mutex> av(mutexT1);
			transport = connectionT1.trame;
		}
		// Insertion d'erreur ICI
		{
			lock_guard<mutex> av(mutexT3);
			connectionT2.trame = transport;
		}
	}
	else if (connectionT1.etat == ENVOYER_T2) {
		connectionT1.etat = UTILISER;
		{
			lock_guard<mutex> av(mutexT1);
			transport = connectionT2.trame;
		}
		// Insertion d'erreur ICI
		{
			lock_guard<mutex> av(mutexT3);
			connectionT1.trame = transport;
		}
	}
	else {
		// quand connection.etat == FINI
		return;
	}
	connectionT1.etat = PAS_UTILISER;
	supportTransmission(connectionT1,connectionT2);
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

	EtatTransmission etat = VIDE;

	Connection t1{ etat ,uint32_t {} }, t2{ etat , uint32_t {} };

	std::thread th1(emission,t1);
	std::thread th2(supportTransmission,t1,t2);

	reception(t2);

	th1.join();
	th2.join();

	system("pause");
}