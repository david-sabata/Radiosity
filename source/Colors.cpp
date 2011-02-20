
#include "Colors.h"


/**
 * Trojice funkci vracejici bitove rozsahy pro ulozeni barev v bufferech
 * Inline v cpp => nelze pouzivat zvenci
 */
inline int getRedBits() {
#ifdef LIMIT_INTERNAL_COLORS
	return 2;
#endif
#ifndef LIMIT_INTERNAL_COLORS
	return 8;
#endif
}
inline int getGreenBits() {
#ifdef LIMIT_INTERNAL_COLORS
	return 2;
#endif
#ifndef LIMIT_INTERNAL_COLORS
	return 8;
#endif
}
inline int getBlueBits() {
#ifdef LIMIT_INTERNAL_COLORS
	return 2;
#endif
#ifndef LIMIT_INTERNAL_COLORS
	return 8;
#endif
}


/**
 * Vychozi hodnoty kroku
 */
float Colors::step_r = 0.0f;
float Colors::step_g = 0.0f;
float Colors::step_b = 0.0f;

/**
 * Vraci nejmensi rozlisitelny krok v barve, v zavislosti na hloubce bufferu
 */
float Colors::getStepRed() {
	if (step_r > 0.0)
		return step_r;

	step_r = float(1 / pow(2.0, getRedBits()));
	if (step_r == numeric_limits<float>::infinity())
		step_r = 0.000001f;

	return step_r;
}

/**
 * Vraci nejmensi rozlisitelny krok v barve, v zavislosti na hloubce bufferu
 */
float Colors::getStepGreen() {
	if (step_g > 0.0)
		return step_g;

	step_g = float(1 / pow(2.0, getGreenBits()));
	if (step_g == numeric_limits<float>::infinity())
		step_g = 0.000001f;

	return step_g;
}

/**
 * Vraci nejmensi rozlisitelny krok v barve, v zavislosti na hloubce bufferu
 */
float Colors::getStepBlue() {
	if (step_b > 0.0)
		return step_b;

	step_b = float(1 / pow(2.0, getBlueBits()));
	if (step_b == numeric_limits<float>::infinity())
		step_b = 0.000001f;

	return step_b;
}



/**
 * Vraci pocet unikatnich barev snizeny o jednicku (cernou nepocitame), ktery lze zobrazit
 */
unsigned long Colors::getColorRange() {	
	long r = long(1 / getStepRed()) + 1; // +1 reprezentuje hodnotu barvy "0"
	long g = long(1 / getStepGreen()) + 1;
	long b = long(1 / getStepBlue()) + 1;
	return r * g * b - 1;
}


/**
 * Vraci pole floatu kde kazde tri hodnoty reprezentuji jednu barvu
 * Pocet barev odpovida getColorRange(), hodnot je colorRange * 3 (barvy)
 */
float* Colors::getUniqueColors() {	
	unsigned long range = getColorRange();
	float* colors = new float[range * 3]; // rgb = 3 hodnoty	
	
	// predchozi volani getColorRange zarucuje, ze step_x uz budou naplnene
	int i = 0;
	for (float r = 0; r <= 1.0f; r += step_r) {
		for (float g = 0; g <= 1.0f; g += step_g) {
			for (float b = 0; b <= 1.0f; b += step_b) {	
				if (r > 0 || g > 0 || b > 0) {
					colors[i] = r;
					colors[i+1] = g;
					colors[i+2] = b;
					i += 3;					
				}
			}
		}
	}

	/*
	int i = 0;
	for (float r = 0.0f; r <= 1.0f; r += 0.5f) {
		for (float g = 0.0f; g <= 1.0f; g += 0.5f) {
			for (float b = 0.0f; b <= 1.0f; b += 0.5f) {
				if (r != 0.0f || g != 0.0f || b != 0.0f) {																			
					colors[i] = r;
					colors[i+1] = g;
					colors[i+2] = b;
					i += 3;										
				}
			}
		}
	}	
	*/

	return colors;
}

/**
 * Vraci pole floatu, kde kazde tri hodnoty tvori barvu a vzdy
 * 6 po sobe jdoucich barev je stejnych. Patch se sklada ze dvou
 * trojuhelniku, tj. 4 (unikatnich) vrcholu, ktere museji mit stejnou barvu
 */
float* Colors::getIndicesColors() {
	unsigned int indices = 4; // pocet indexu se stejnou barvou
	unsigned long range = getColorRange();
	float* uniqueColors = getUniqueColors();
	float* indicesColors = new float[range * 3 * indices];
	
	for (unsigned int i = 0; i < range * 3; i += 3) { // i = prvni slozka aktualne klonovane barvy
		
		for (unsigned int j = 0; j < indices; j++) { // j = cislo vrcholu se stejnou barvou
			unsigned int offset = i * indices + j * 3; // prvni slozka vystupni barvy ve vyslednem poli
			indicesColors[offset    ]     = uniqueColors[i];
			indicesColors[offset + 1] = uniqueColors[i + 1];
			indicesColors[offset + 2] = uniqueColors[i + 2];		
		}

	}

	delete[] uniqueColors;

	return indicesColors;
}