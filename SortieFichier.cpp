#include "SortieFichier.h"

using namespace std;

SortieFichier::SortieFichier(const SortieFichier&) {

}

SortieFichier::SortieFichier(string destination)
{
	if (destination != "") {
		fichier.open(destination);
	}
	else {
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
	fichier << c;
}