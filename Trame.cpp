#include "Trame.h"


uint16_t Trame::getType() { 
	return contenu >> TYPE_POS; 
}
uint16_t Trame::getSequence() { 
	return (contenu - (getType() << TYPE_POS)) >> SEQ_POS; 
}
uint16_t Trame::getDonnees() { 
	return contenu - (getType() << TYPE_POS) - (getSequence() << SEQ_POS); 
}

void Trame::setType(uint16_t type) { 
	contenu = (type << TYPE_POS) + (getSequence() << SEQ_POS) + getDonnees(); 
}
void Trame::setSequence(uint16_t sequence) { 
	contenu = (getType() << TYPE_POS) + (sequence << SEQ_POS) + getDonnees(); 
}
void Trame::setDonnees(uint16_t donnees) { 
	contenu = (getType() << TYPE_POS) + (getSequence() << SEQ_POS) + donnees; 
}

void Trame::print() {
	std::bitset<TRAME_SIZE> c(contenu);
	std::cout << std::endl << c;
}

void Trame::printDetaille() {
	std::bitset<TRAME_SIZE> c(contenu);

	std::cout << std::endl << " " << c[TYPE_POS + 1] << c[TYPE_POS] << " ";
	for (int i = TYPE_POS - 2; i >= SEQ_POS; i--) {
		std::cout << c[i];
	}
	std::cout << " ";
	for (int i = SEQ_POS - 1; i >= 0; i--) {
		std::cout << c[i];
	}
	std::cout << std::endl;
	std::cout << "  |     |      |" << std::endl;
	std::cout << "Type   Seq   Donnees";
}