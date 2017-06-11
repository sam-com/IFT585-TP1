#pragma once
#include <string>
#include <fstream>

class SortieFichier
{
private:
	std::ofstream fichier;
	bool valide;
public:
	SortieFichier(std::string source);
	SortieFichier(const SortieFichier&);
	~SortieFichier();

	bool isValid();
	void send16Bits(uint16_t bits);
	void close();
};

