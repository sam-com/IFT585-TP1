
#ifndef VetorCirculaire_H
#define VetorCirculaire_H

#include <vector>
#include <string>

template <class T>
class VectorCirculaire {
private :
	std::vector<T> v;
	int debut;
	int fin;
	int size;
	void augmenter(int&);
public :
	VectorCirculaire(int taille);
	void ajouter(T t);
	void enlever();
	bool estPlein();
	bool estVide();
	int taille();
	int tailleMax();
};

#endif
