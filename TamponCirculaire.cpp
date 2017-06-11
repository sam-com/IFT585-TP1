#include "TamponCirculaire.h"
#include <iostream>

using namespace std;

TamponCirculaire::TamponCirculaire() : v(0), debut(0), fin(0), size(0) {
	fenetre = Fenetre();
	fenetre.debut = 0;
	fenetre.fin = 0;
	fenetre.taille = 0;
	fenetre.debutSeq = 0;
}
TamponCirculaire::TamponCirculaire(int taille, int tailleFenetre) : v(taille), debut(0), fin(0), size(0) {
    fenetre = Fenetre();
    fenetre.debut = 0;
    fenetre.fin = tailleFenetre;
    fenetre.taille = tailleFenetre;
	fenetre.debutSeq = 0;
}

/*void TamponCirculaire::augmenter(int& i) {
    i == v.size() - 1 ? i = 0 : ++i;
}*/

bool TamponCirculaire::peutInserer(int seq) {
	uint16_t debut = fenetre.debutSeq;
	uint16_t fin = (fenetre.debutSeq + fenetre.taille) % ((int)std::pow(2, SEQ_SIZE));
    return (((debut <= seq) && (seq < fin)) ||
        ((fin < debut) && (debut <= seq)) ||
        ((seq < fin) && (fin < debut)));
}

bool TamponCirculaire::ajouter(Trame t) {
    bool succes = false;
    if (peutInserer(t.getSequence())) {
        uint16_t seq = t.getSequence();
        if (peutInserer(seq) && v.at(seq).getType() == TYPE_VALIDATED) {
            v.at(seq) = t;
            succes = true;
        }
    }

    return succes;
}

/*void TamponCirculaire::enlever() {
    if (estVide()) {
        throw exception{};
    }

    augmenter(debut);
    --size;
}*/

/*bool TamponCirculaire::estPlein() {
    return debut == fin && size == v.size();
}*/

/*bool TamponCirculaire::estVide() {
    return debut == fin && size != v.size();
}*/

bool TamponCirculaire::estDebutDeFenetre(int seq) {
    return fenetre.debutSeq == seq;
}

uint16_t TamponCirculaire::getSeqDebutFenetre() {
    return fenetre.debutSeq;
}

bool TamponCirculaire::estDernierDeFenetre(int seq) {
	uint16_t maxSeq = (int)std::pow(2, SEQ_SIZE);
	uint16_t fin = (fenetre.debutSeq + fenetre.taille) % maxSeq;
    if (fin == 0) {
        return seq == (maxSeq - 1);
    }
    else {
        return seq == fin - 1;
    }
}

void TamponCirculaire::validerTrame(int seq) {
    v.at(seq).setType(TYPE_VALIDATED);
}

void TamponCirculaire::deplacerFenetre() {
    fenetre.debut = (fenetre.debut + fenetre.taille) % v.size();
    fenetre.fin = (fenetre.fin + fenetre.taille) % v.size();
	fenetre.debutSeq = (fenetre.debutSeq + fenetre.taille) % ((int) std::pow(2, SEQ_SIZE));

}
void TamponCirculaire::deplacerFenetre(int nbPosition) {
    fenetre.debut = (fenetre.debut + nbPosition) % v.size();
    fenetre.fin = (fenetre.fin + nbPosition) % v.size();
	fenetre.debutSeq = (fenetre.debutSeq + nbPosition) % ((int)std::pow(2, SEQ_SIZE));
}

vector<Trame> TamponCirculaire::getListeTrameNonValideDansFenetre() {
    vector<Trame> tmp = vector<Trame>();
    if (fenetre.debut < fenetre.fin) {
        for (int i = fenetre.debut; i < std::min((int)v.size(), fenetre.fin); i++) {
            if (v.at(i).getType() == TYPE_DONNEES) {
                tmp.push_back(v.at(i));
            }
        }
    }
    else {
        for (int i = fenetre.debut; i < v.size(); i++) {
            if (v.at(i).getType() == TYPE_DONNEES) {
                tmp.push_back(v.at(i));
            }
        }
        for (int i = fenetre.fin; i < fenetre.debut; i++) {
            if (v.at(i).getType() == TYPE_DONNEES) {
                tmp.push_back(v.at(i));
            }
        }
    }

    return tmp;
}
Trame TamponCirculaire::get(int seq) {
    return v.at(seq);
}


int TamponCirculaire::taille() {
    return size;
}

int TamponCirculaire::tailleMax() {
    return v.size();
}

void TamponCirculaire::print() {
    std::cout << "Taille fenetre: " << fenetre.taille << ", Debut: " << fenetre.debut << ", Fin: " << fenetre.fin << std::endl;
    for (int i = 0; i < v.size(); i++) {
        std::cout << v.at(i).getSequence();
    }

    std::cout << std::endl;
}