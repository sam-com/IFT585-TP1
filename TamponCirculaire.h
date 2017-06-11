#pragma once 

#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include "Trame.h"

struct Fenetre {
	int debut;
	int fin;
	int taille;
	int debutSeq;
};

class TamponCirculaire {
private :
	std::vector<Trame> v;
	int debut;
	int fin;
	int size;
	Fenetre fenetre;

public :
	TamponCirculaire();
	TamponCirculaire(int taille, int tailleFenetre);
	~TamponCirculaire();
	bool ajouter(Trame t);
	bool peutInserer(int seq);
	bool estDebutDeFenetre(int seq);
	uint16_t getSeqDebutFenetre();
	uint16_t getSeqFinFenetre();
	bool estDernierDeFenetre(int seq);
	void validerTrame(int seq);
	void deplacerFenetre();
	void deplacerFenetre(int nbPosition);
	std::vector<Trame> getListeTrameNonValideDansFenetre();
	Trame get(int seq);
	int taille();
	int tailleMax();
	void print();
};

