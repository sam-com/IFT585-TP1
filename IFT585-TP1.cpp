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
mutex m1;					// pour interdit l'ecriture en meme entre thread emission && supportTransmission
mutex m2;					// pour interdit l'ecriture en meme entrethread reception && supportTransmission
bool ready1 = false;		// thread emission a envoyer des donnees
bool ready2 = false;		// thread reception a envoyer des donnees
bool reception1 = false;	// reception disponible pour thread emission
bool reception2 = false;	// reception disponible pour thread reception
bitset<TrameTaille> b1;		// donnees entre thread emission && supportTransmission
bitset<TrameTaille> b2;		// donnees entre thread reception && supportTransmission
bool finiEmission;			
bool finiEmissionTransmission;		
bool finiReception;


enum LogContexte { SEND, RECEIVE };
void log(LogContexte LogContexte, bitset<TrameTaille> bits);
string lireFichier(const string& nomFichier);
void ecrireFichier(const string& nomFichier, const string& s);
string extractTrame(string& fichier);

void emission() {
	string fichier = lireFichier(fichierACopier);
	VectorCirculaire<bitset<TrameTaille>> vector(tailleTempon);

	while (!fichier.empty()) {
		bitset<TrameTaille> trame(string(extractTrame(fichier)));
		
		log(SEND, trame);
		while (ready1) {
			this_thread::sleep_for(std::chrono::seconds(1));
		}

		{
			lock_guard<mutex> av(m1);
			b1 = trame;
			ready1 = true;
		}
	}
	finiEmission = true;
}

void reception() {
	bitset<TrameTaille> reception;
	string res;
	while (!finiEmissionTransmission || reception2) {
		while (!reception2) {
			this_thread::sleep_for(std::chrono::seconds(1));
		}
		{
			lock_guard<mutex> av(m2);
			reception = b2;
			reception2 = false;
		}
		log(RECEIVE, reception);
		res += reception.to_string();
	}
	finiReception = true;
	ecrireFichier(fichierDestination, res);
}

void supportTransmission() {
	bitset<TrameTaille> transport;
	while (!ready1 && !ready2 && (!finiEmission || !finiReception)) {
		this_thread::sleep_for(std::chrono::seconds(1));
	}
	if (ready1) {
		{
			lock_guard<mutex> av(m1);
			transport = b1;
		}
		// TODO inserer des erreurs dans les bits 

		while (reception2) {
			this_thread::sleep_for(std::chrono::seconds(1));
		}
		{
			lock_guard<mutex> av(m2);
			b2 = transport;
			reception2 = true;
		}
		if (finiEmission) {
			finiEmissionTransmission = true;
		}
		ready1 = false;
		supportTransmission();
	} else if (ready2) {
		// TODO
		ready2 = false;
		supportTransmission();
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

	std::thread th1(emission);
	std::thread th2(supportTransmission);

	reception();

	th1.join();
	th2.join();

	system("pause");
}

string extractTrame(string& fichier) {
	string retour;
	if (fichier.size() == TrameTaille) {
		retour = fichier;
		fichier = "";
	} else if (fichier.size() < TrameTaille) {
		// TODO s'assurer de toujous avoir une trame de 32 bit
	} else {
		retour = fichier.substr(0, TrameTaille);
		fichier = fichier.substr(TrameTaille + 1);
	}
	return retour;
}

string lireFichier(const string& nomFichier) {
	string s;
	copy(istreambuf_iterator<char>(std::ifstream{ nomFichier }),
		istreambuf_iterator<char>{},
		back_inserter(s));
	return s;
}

void ecrireFichier(const string& nomFichier, const string& s) {
	copy(begin(s),end(s),ostream_iterator<char>{ std::ofstream{ nomFichier } });
}

void log(LogContexte LogContexte, bitset<TrameTaille> bits) {
	if (LogContexte == SEND) {
		cout << "Send ";
	}
	else if (LogContexte == RECEIVE) {
		cout << "Receive ";
	}
	cout << "Trame :" << bits << endl;
}