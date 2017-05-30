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

	string line;
	if ( fichierParametres.is_open() ) {
		if ( getline(fichierParametres, line) ) {
			tailleTempon = atoi(line.c_str());
		}

		if (getline(fichierParametres, line)) {
			delaiTemporisation = atoi(line.c_str());
		}

		if (getline(fichierParametres, line)) {
			fichierACopier = line;
		}

		if (getline(fichierParametres, line)) {
			fichierDestination = line;
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

	if (entree.is_open()) {
		
	}
	else {
		cout << "Fichier a copier introuvable.";
	}
	cin.get();
    return 0;
}
