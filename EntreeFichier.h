#pragma once
#include <string>
#include <fstream>
class EntreeFichier
{
private:
	std::ifstream fichier;
	bool valide;
public:
	EntreeFichier(std::string source);
	EntreeFichier(const EntreeFichier&);
	~EntreeFichier();

	bool isValid();
	int getNext16Bits();
	bool finFichierAtteint();
};

