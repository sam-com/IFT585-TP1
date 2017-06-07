#include "Hamming.h"
#include <math.h>
#include "Constantes.h"

namespace Hamming {
	bool getBit(uint16_t donnees, int position) // position in range 0-7
	{
		return (donnees >> position) & 0x1;
	}

	// Ajoute les bits de contr�le aux donn�es et retourne les nouvelles donn�es r�sultantes
	uint16_t constructHamming(uint16_t donnees) {
		uint16_t donneeAvecBitControle = 0;
		int sizeCheck = 1;				// Le nombre de bit coll� qu'on regarde pour calculer le bit de controlle
		int puissance = 0;				// La puissance courrante de deux
		int position = 0;				// La position courrante du bit dans les donnes avec bit de controlle
		int positionOriginal = 0;		// La position du bit dans les donn�es originaux
		int positionBitControle = 0;	// La position du bit de contr�le
		bool valeurBitControle;			// Valeur du bit de contr�le

		// Ajoute des 0 o� un bit de contr�le devrait aller
		while (position < DONNEES_SIZE) {
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
		// Pour tous les bit de controlle qui doivent �tre mis
		while (puissance < NB_BIT_CONTROLLEUR) {
			valeurBitControle = false;
			sizeCheck = pow(2, puissance);
			positionBitControle = position = sizeCheck - 1;

			// Pour tous les positions dans les nouvelles donn�es
			while (position < DONNEES_SIZE) {

				// Pour tous les bit coll�s qu'on doit regarder
				for (int i = 0; i < sizeCheck; i++) {
					int bit = getBit(donneeAvecBitControle, position);
					valeurBitControle = getBit(donneeAvecBitControle, position + i) ? !valeurBitControle : valeurBitControle;
				}
				position += (sizeCheck * 2); // On skip des prochains bits
			}
			donneeAvecBitControle += valeurBitControle << positionBitControle;	// On change la valeur du bit
			puissance++;

		}

		return donneeAvecBitControle;
	}
}