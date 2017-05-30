// IFT585-TP1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <string>
#include <thread>

using namespace std;

size_t	tailleTempon;
int		delaiTemporisation;
string	fichierACopier;
string	fichierDestination;


void emission() {

}

void reception() {

}

void supportTransmission() {

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

	ifstream entree;
	entree.open(fichierACopier);

	if (valide && entree.is_open()) {

	}
	else {
		cout << "Paramètres invalides ou fichier a copier introuvable.";
	}
	cin.get();
	return 0;
}
