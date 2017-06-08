#pragma once
#include <stdint.h> 
namespace Hamming
{
	bool detecter(uint16_t donnees);
	uint16_t corriger(uint16_t donnees);
	uint16_t encoder(uint16_t donnees);
	uint16_t decoder(uint16_t donnees);
}