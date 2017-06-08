#include "Hamming.h"
#include <math.h>
#include "Constantes.h"

#include <bitset>
#include <iostream>

namespace Hamming {
	bool getBit(uint16_t donnees, int position) // position in range 0-7
	{
		return (donnees >> position) & 0x1;
	}

	// Retourne la valeur que devrait être le bit de contrôle à une position donnée
	bool getValeurBitControle(uint16_t donnees, int positionBitControle) {
		int sizeCheck = positionBitControle + 1;
		int position = positionBitControle;
		bool valeurBitControle;

		valeurBitControle = false;
		sizeCheck = positionBitControle + 1;

		// Pour tous les positions dans les données
		while (position < TRAME_SIZE) {

			// Pour tous les bit collés qu'on doit regarder
			for (int i = 0; i < sizeCheck; i++) {
				valeurBitControle = getBit(donnees, position + i) ? !valeurBitControle : valeurBitControle;
			}
			position += (sizeCheck * 2); // On skip des prochains bits
		}

		return valeurBitControle;
	}

	// Détecte s'il y a une erreur dans la données avec Hamming
	int getValeurDesBitsControle(uint16_t donnees) {
		int puissance = 0;
		int positionBitControle = 0;
		int resultat = 0;
		bool valeur = false;

		// Pour tous les bits de contrôle
		while (positionBitControle < (TRAME_SIZE - 1)) {
			positionBitControle = pow(2, puissance) - 1;
			valeur = getValeurBitControle(donnees, positionBitControle);
			resultat += valeur << puissance;
			puissance++;
		}

		// Si la valeur est différent de zéro, il y a une erreur
		return resultat;
	}

	// Détecte s'il y a une erreur dans la données avec Hamming
	bool detecter(uint16_t donnees) {
		return getValeurDesBitsControle(donnees) != 0;
	}

	uint16_t corriger(uint16_t donnees) {
		return donnees ^= 1 << (getValeurDesBitsControle(donnees) - 1);
	}

	// Ajoute les bits de contrôle aux données et retourne les nouvelles données résultantes
	uint16_t encoder(uint16_t donnees) {
		uint16_t donneeAvecBitControle = 0;
		int sizeCheck = 1;				// Le nombre de bit collé qu'on regarde pour calculer le bit de controlle
		int puissance = 0;				// La puissance courrante de deux
		int position = 0;				// La position courrante du bit dans les donnes avec bit de controlle
		int positionOriginal = 0;		// La position du bit dans les données originaux
		int positionBitControle = 0;	// La position du bit de contrôle
		bool valeurBitControle;			// Valeur du bit de contrôle

		// Ajoute des 0 où un bit de contrôle devrait aller
		while (position < TRAME_SIZE) {
			if ((position + 1) != pow(2, puissance)) {
				donneeAvecBitControle += getBit(donnees, positionOriginal) << position;
				positionOriginal++;
			}
			else {
				puissance++;
			}
			position++;
		}

		puissance = 0;
		position = 0;

		// Assigne la bonne valeur au bit de controle
		// Pour tous les bit de controlle qui doivent être mis
		while (puissance < NB_BIT_CONTROLE) {
			positionBitControle = pow(2, puissance) - 1;
			bool val = getValeurBitControle(donneeAvecBitControle, positionBitControle);
			donneeAvecBitControle += val << positionBitControle;
			puissance++;
		}

		return donneeAvecBitControle;
	}

	// Enlève les bits de contrôle aux données et retourne la trame original
	uint16_t decoder(uint16_t donnees) {
		uint16_t donneeSansBitControle = 0;
		int puissance = 0;				// La puissance courrante de deux
		int position = 0;				// La position courrante du bit dans les donnes avec bit de controlle
		int positionOriginal = 0;		// La position du bit dans les données originaux

		// Construit les données originals en ignorant les position qui sont une puissance de (1, 2, 4, 8, ...)
		while (position < TRAME_SIZE) {
			if ((position + 1) != pow(2, puissance)) {
				donneeSansBitControle += getBit(donnees, position) << positionOriginal;
				positionOriginal++;
			}
			else {
				puissance++;
			}
			position++;
		}

		return donneeSansBitControle;
	}


}