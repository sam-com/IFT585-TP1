#pragma once
#include <stdint.h> 
namespace Hamming
{
	bool detecter(uint32_t donnees);
	uint32_t corriger(uint32_t donnees);
	uint32_t encoder(uint32_t donnees);
	uint32_t decoder(uint32_t donnees);
}