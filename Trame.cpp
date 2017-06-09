#include "Trame.h"

Trame::Trame() {
	setType(TYPE_VALIDATED);
	setSequence(0);
	setDonnees(0);
}
Trame::Trame(uint32_t c) {
	contenu = c;
}

Trame::Trame(uint16_t type, uint16_t sequence, uint16_t donnees) {
	setType(type);
	setSequence(sequence);
	setDonnees(donnees);
}

uint16_t Trame::getType() { 
	return (contenu & ((UINT32_MAX >> (TRAME_SIZE - TYPE_SIZE)) << TYPE_POS)) >> TYPE_POS;
}

uint16_t Trame::getSequence() {
	return (contenu & ((UINT32_MAX >> (TRAME_SIZE - SEQ_SIZE)) << SEQ_POS)) >> SEQ_POS;
}

uint16_t Trame::getDonnees() { 
	return (contenu & ((UINT32_MAX >> (TRAME_SIZE - DONNEES_SIZE)) << DONNEES_POS)) >> DONNEES_POS;
}

uint32_t Trame::getContenu() {
	return contenu;
}

void Trame::setType(uint32_t type) {
	type = type % (int) pow(2, TYPE_SIZE);
	contenu = (type << TYPE_POS) + (getSequence() << SEQ_POS) + (getDonnees() << DONNEES_POS); 
}

void Trame::setSequence(uint32_t sequence) { 
	sequence = sequence % (int) pow(2, SEQ_SIZE);
	contenu = (getType() << TYPE_POS) + (sequence << SEQ_POS) + (getDonnees() << DONNEES_POS);
}

void Trame::setDonnees(uint32_t donnees) { 
	donnees = donnees % (int)pow(2, MESSAGE_SIZE_MAX);
	contenu = (getType() << TYPE_POS) + (getSequence() << SEQ_POS) + (donnees << DONNEES_POS);
}

void Trame::setContenu(uint32_t c) {
	c = c % (int) pow(2, TRAME_SIZE);
	contenu = c;
}

void Trame::flipBitAPosition(int position) {
	assert(position < TRAME_SIZE);
	contenu ^= 1 << position;
}

void Trame::print() {
	std::bitset<TRAME_SIZE> c(contenu);

	for (int i = TRAME_SIZE - 1; i >= DONNEES_POS; i--) {
		std::cout << c[i];
	}
	std::cout << " ";

	for (int i = DONNEES_POS - 1; i >= SEQ_POS; i--) {
		std::cout << c[i];
	}

	std::cout << " ";
	for (int i = SEQ_POS - 1; i >= 0; i--) {
		std::cout << c[i];
	}
	std::cout << std::endl;
}