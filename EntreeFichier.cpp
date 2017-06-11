#include "EntreeFichier.h"
#include <iostream>

using namespace std;

EntreeFichier::EntreeFichier(const EntreeFichier&) {

}

EntreeFichier::EntreeFichier(string source)
{
	fichier.open(source);

	if (!fichier.is_open()) {
		valide = false;
	}
}


EntreeFichier::~EntreeFichier()
{
}

bool EntreeFichier::isValid() {
	return valide;
}
int EntreeFichier::getNext16Bits() {
	if (!valide || finFichierAtteint()) return 0;

	uint16_t retour = 0;

	char c;

	fichier.get(c);

	if (!fichier.eof()) {
		retour = c << 8;
		fichier.get(c);
		if (!fichier.eof()) {
			retour += c;
		}
	}
	bool fi = fichier.eof();
	return retour;
}

bool EntreeFichier::finFichierAtteint() {
	bool fi = fichier.eof();
	return !valide || fichier.eof();
}

void EntreeFichier::close() {
	if (isValid()) {
		fichier.close();
	}
}
