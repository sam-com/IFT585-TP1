#include "SortieFichier.h"

using namespace std;

SortieFichier::SortieFichier(const SortieFichier&) {

}

SortieFichier::SortieFichier(string destination)
{
	if (destination != "") {
		fichier.open(destination);
	}

	if (!fichier.is_open()) {
		valide = false;
	}
}


SortieFichier::~SortieFichier()
{
}
bool SortieFichier::isValid() {
	return valide;
}

void SortieFichier::send16Bits(uint16_t bits) {
	uint8_t c = bits >> 8;
	fichier << c;
	c = bits;
	if (c != 0) {
		fichier << c;
	}
}

void SortieFichier::close() {
	fichier.close();
}