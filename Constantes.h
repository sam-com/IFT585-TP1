#pragma once

// Le nombre de bits disponible dans une trame
#define TRAME_SIZE 32											

// Le nombre de bit de contr�le � ajouter pour Hamming (� recalculer si on modifie TRAME_SIZE)
#define NB_BIT_CONTROLE 5										

// Le nombre de bit allou� pour sp�cifier le type de la trame
#define TYPE_SIZE 2

// Le nombre de bit allou� pour sp�cifier  la s�quence de la trame
#define SEQ_SIZE 3

// La nombre de bit allou� pour la donn�e dans la trame
#define DONNEES_SIZE (TRAME_SIZE - TYPE_SIZE - SEQ_SIZE)

// La position de d�part des bits qui sp�cifie le type de la trame
#define TYPE_POS 0

// La position de d�part des bits qui sp�cifie la s�quence de la trame
#define SEQ_POS TYPE_SIZE

// La position de d�part des bits qui sp�cifie les donn�es
#define DONNEES_POS (TYPE_SIZE + SEQ_SIZE)

// Le nombre de bit maximum pour seulement pour le message, pour donner de la place aux bits de controles qui s'ajouterons
#define MESSAGE_SIZE_MAX (DONNEES_SIZE - NB_BIT_CONTROLE)

/*
	Les diff�rents types de trames
*/
#define TYPE_DONNEES 0
#define TYPE_ACK 1
#define TYPE_NAK 2
#define TYPE_VALIDATED 3