#include "Hamming.h"
#include <math.h>
#include "Constantes.h"

#include <bitset>
#include <iostream>

namespace Hamming {
	bool getBit(uint32_t donnees, int position) // position in range 0-7
	{
		return (donnees >> position) & 0x1;
	}

	// Retourne la valeur que devrait �tre le bit de contr�le � une position donn�e
	bool getValeurBitControle(uint32_t donnees, int positionBitControle) {
		int sizeCheck = positionBitControle + 1;
		int position = positionBitControle;
		bool valeurBitControle;

		valeurBitControle = false;
		sizeCheck = positionBitControle + 1;

		// Pour tous les positions dans les donn�es
		while (position < TRAME_SIZE) {

			// Pour tous les bit coll�s qu'on doit regarder
			for (int i = 0; i < sizeCheck; i++) {
				valeurBitControle = getBit(donnees, position + i) ? !valeurBitControle : valeurBitControle;
			}
			position += (sizeCheck * 2); // On skip des prochains bits
		}

		return valeurBitControle;
	}

	// D�tecte s'il y a une erreur dans la donn�es avec Hamming
	int getValeurDesBitsControle(uint32_t donnees) {
		int puissance = 0;
		int positionBitControle = 0;
		int resultat = 0;
		bool valeur = false;

		// Pour tous les bits de contr�le
		while (positionBitControle < (TRAME_SIZE - 1)) {
			positionBitControle = pow(2, puissance) - 1;
			valeur = getValeurBitControle(donnees, positionBitControle);
			resultat += valeur << puissance;
			puissance++;
		}

		// Si la valeur est diff�rent de z�ro, il y a une erreur
		return resultat;
	}

	// D�tecte s'il y a une erreur dans la donn�es avec Hamming
	bool detecter(uint32_t donnees) {
		return getValeurDesBitsControle(donnees) != 0;
	}

	uint32_t corriger(uint32_t donnees) {
		return donnees ^= 1 << (getValeurDesBitsControle(donnees) - 1);
	}

	// Ajoute les bits de contr�le aux donn�es et retourne les nouvelles donn�es r�sultantes
	uint32_t encoder(uint32_t donnees) {
		uint32_t donneeAvecBitControle = 0;
		int sizeCheck = 1;				// Le nombre de bit coll� qu'on regarde pour calculer le bit de controlle
		int puissance = 0;				// La puissance courrante de deux
		int position = 0;				// La position courrante du bit dans les donnes avec bit de controlle
		int positionOriginal = 0;		// La position du bit dans les donn�es originaux
		int positionBitControle = 0;	// La position du bit de contr�le
		bool valeurBitControle;			// Valeur du bit de contr�le

		// Ajoute des 0 o� un bit de contr�le devrait aller
		while (position < TRAME_SIZE) {
			if ((position + 1) != pow(2, puissance) || (position + 1) >= pow(2, NB_BIT_CONTROLE)) {
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
		// Pour tous les bit de controlle qui doivent �tre mis
		while (puissance < NB_BIT_CONTROLE) {
			positionBitControle = pow(2, puissance) - 1;
			bool val = getValeurBitControle(donneeAvecBitControle, positionBitControle);
			donneeAvecBitControle += val << positionBitControle;
			puissance++;
		}

		return donneeAvecBitControle;
	}

	// Enl�ve les bits de contr�le aux donn�es et retourne la trame original
	uint32_t decoder(uint32_t donnees) {
		uint32_t donneeSansBitControle = 0;
		int puissance = 0;				// La puissance courrante de deux
		int position = 0;				// La position courrante du bit dans les donnes avec bit de controlle
		int positionOriginal = 0;		// La position du bit dans les donn�es originaux

		// Construit les donn�es originals en ignorant les position qui sont une puissance de (1, 2, 4, 8, ...)
		while (position < TRAME_SIZE) {
			if ((position + 1) != pow(2, puissance) || (position + 1) >= pow(2, NB_BIT_CONTROLE)) {
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