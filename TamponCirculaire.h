#pragma once 

#include <vector>
#include <string>
#include <algorithm>
#include "Trame.h"

struct Fenetre {
	int debut;
	int fin;
	int taille;
};

class TamponCirculaire {
private :
	std::vector<Trame> v;
	int debut;
	int fin;
	int size;
	void augmenter(int&);
	Fenetre fenetre;

public :
	TamponCirculaire(int taille, int tailleFenetre);
	bool ajouter(Trame t);
	void enlever();
	bool estPlein();
	bool estVide();
	bool peutInserer(int seq);
	//bool fenetrePlein();
	//bool fenetreVide();
	bool estDebutDeFenetre(int seq);
	uint16_t getSeqDebutFenetre();
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
