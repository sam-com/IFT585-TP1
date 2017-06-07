#pragma once

#define TRAME_SIZE 16
#define NB_BIT_CONTROLLEUR 4
#define TYPE_POS TRAME_SIZE - 2
#define SEQ_POS TRAME_SIZE - 8
#define DONNEES_POS 0
#define DONNEES_SIZE TRAME_SIZE - 8
#define MAX_MESSAGE_SIZE DONNEES_SIZE - NB_BIT_CONTROLLEUR
#define DONNEES 0
#define ACK 1
#define NAK 2