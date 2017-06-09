#pragma once
#include "Constantes.h"
#include <iostream>
#include <bitset>
#include <stdint.h> 
#include <assert.h>
#include <math.h>

/*	Une trame
	Grosseur de deux octets (N = 2), peut �tre chang� via Constante.h, mais il faut aussi changer 
	uint32_t par la grosseur voulu (ex: uint32_t pour 4 octets)

	Les deux premiers bit d�finit le type de la trame (DONNEES, ACK ou NAK)
	Les 6 prochains contient la s�quence (le num�ro de trame, pouvant aller de 0 � 63)
	Le reste sont les donn�es (un octet dans le cas de N = 2)
*/
class Trame {
private:
	uint32_t contenu;
public:
	Trame();
	Trame(uint32_t contenu);
	Trame(uint16_t type, uint16_t sequence, uint16_t donnees);
	uint16_t getType();						// Retourne le type (0 = DONNEES, 1 = ACK ou 2 = NAK)
	uint16_t getSequence();					// Retourne la s�quence de la trame
	uint16_t getDonnees();					// Retourne les donn�es
	uint32_t getContenu();					// Retourne la totalit� du contenu

	void setType(uint32_t type);			// Affecte le type de la trame
	void setSequence(uint32_t sequence);	// Affecte la s�quence
	void setDonnees(uint32_t donnees);		// Affecte les donn�es
	void setContenu(uint32_t c);			// Affecte tous le contenu de la trame
	void flipBitAPosition(int position);	// Flip un bit � la position en param�tre

	void print();							// Print le contenu dans la console
};