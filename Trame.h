#pragma once
#include "Constantes.h"
#include <iostream>
#include <bitset>
#include <stdint.h> 

/*	Une trame
	Grosseur de deux octets (N = 2), peut être changé via Constante.h, mais il faut aussi changer 
	uint16_t par la grosseur voulu (ex: uint32_t pour 4 octets)

	Les deux premiers bit définit le type de la trame (DONNEES, ACK ou NAK)
	Les 6 prochains contient la séquence (le numéro de trame, pouvant aller de 0 à 63)
	Le reste sont les données (un octet dans le cas de N = 2)
*/
class Trame {
private:
	uint16_t contenu;
public:
	uint16_t getType();						// Retourne le type (0 = DONNEES, 1 = ACK ou 2 = NAK)
	uint16_t getSequence();					// Retourne la séquence de la trame
	uint16_t getDonnees();					// Retourne les données

	void setType(uint16_t type);			// Affecte le type de la trame
	void setSequence(uint16_t sequence);	// Affecte la séquence
	void setDonnees(uint16_t donnees);		// Affecte les données

	void print();
	void printDetaille();
};