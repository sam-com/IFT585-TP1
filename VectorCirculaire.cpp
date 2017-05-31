#include "VectorCirculaire.h"
#include <iostream>

using namespace std;

template <class T>
VectorCirculaire<T>::VectorCirculaire(int taille) : v(taille), debut(0), fin(0), size(0) {
}

template <class T>
void VectorCirculaire<T>::augmenter(int& i) {
	i == v.size() - 1 ? i = 0 : ++i;
}

template <class T>
void VectorCirculaire<T>::ajouter(T t) {
	if (estPlein()) {
		throw exception{};
	}

	v.at(fin) = t;
	augmenter(fin);
	++size;
}

template <class T>
void VectorCirculaire<T>::enlever() {
	if (estVide()) {
		throw exception{};
	}

	augmenter(debut);
	--size;
}

template <class T>
bool VectorCirculaire<T>::estPlein() {
	return debut == fin && size == v.size();
}

template <class T>
bool VectorCirculaire<T>::estVide() {
	return debut == fin && size != v.size();
}

template <class T>
int VectorCirculaire<T>::taille() {
	return size;
}

template <class T>
int VectorCirculaire<T>::tailleMax() {
	return v.size();
}