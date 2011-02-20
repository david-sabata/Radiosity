
#include "Colors.h"


/**
 * Vychozi hodnoty kroku
 */
float Colors::step[3] = { 0.0f, 0.0f, 0.0f };

/**
 * Vychozi hodnoty bitovych rozsahu
 */
#ifndef LIMIT_INTERNAL_COLORS
unsigned int Colors::bits[3] = { 8, 8, 8 };
#else
unsigned int Colors::bits[3] = { 1, 1, 1 };
#endif

/**
 * Pozadovany pocet unikatnich barev
 * Pokud je rozsah vyssi, dynamicky se snizi
 */
unsigned long Colors::neededColors = 0;


/**
 * Nastavi pozadovany pocet unikatnich barev. Pouze pro optimalizaci.
 * Pokud je rozsah vyssi nez potrebny pocet barev, zkusi se rozsah snizit.
 * Kdyby rozsah byl nizsi, generator sceny jej pouzije vicekrat
 */
void Colors::setNeededColors(unsigned long colors) {
	neededColors = colors;
}


/**
 * Snizi bitovy rozsah jedne barvy (te s nejvyssim rozsahem) o jeden bit,
 * cimz snizi celkovy rozsah unikatnich barev na polovinu.
 * Vraci bool podle toho, jestli probehlo snizeni rozsahu nektere barvy
 */
bool Colors::decreaseBits() {
	unsigned int m = max(bits[RED], max(bits[GREEN], bits[BLUE]));
	if (m == 1)
		return false; // nechceme se dostat na rozsah 0 bitu

	// vynuti pregenerovani kroku, a tim i rozsahu
	step[0] = step[1] = step[2] = 0;

	if (m == bits[RED]) {
		bits[RED]--;
		return true;
	} else if (m == bits[GREEN]) {
		bits[GREEN]--;
		return true;
	} else {
		bits[BLUE]--;
		return true;
	}
}


/**
 * Vraci bitovy rozsah bufferu pro pozadovanou barvu.
 * Pripadne dynamicky snizeny podle neededColors
 */
unsigned int Colors::getBits(unsigned int color) {
	if (bits[color] == 0) {
		// volat OpenGL funkci pro zjisteni poctu bitu barvy v bufferu
		return 1;
	} else
		return bits[color];
}



/**
 * Vraci nejmensi rozlisitelny krok v barve, v zavislosti na hloubce bufferu
 */
float Colors::getStep(unsigned int color) {
	if (step[color] > 0.0)
		return step[color];

	step[color] = float(1 / pow(2.0, int(getBits(color)) ));
	if (step[color] == numeric_limits<float>::infinity())
		step[color] = 0.000001f;

	return step[color];
}



/**
 * Vraci pocet unikatnich barev snizeny o jednicku (cernou nepocitame), ktery lze zobrazit
 */
unsigned long Colors::getColorRange() {	
	long r = long(1 / getStep(RED)) + 1; // +1 reprezentuje hodnotu barvy "0"
	long g = long(1 / getStep(GREEN)) + 1;
	long b = long(1 / getStep(BLUE)) + 1;
	
	unsigned long range = r * g * b - 1;
	// pokud je aktualni rozsah vice nez dvojnasobek pozadovaneho, muzeme jej snizit
	if (neededColors > 0 && range > neededColors * 2) {
		if (decreaseBits())
			return getColorRange();
	}

	return range;
}


/**
 * Vraci pole floatu kde kazde tri hodnoty reprezentuji jednu barvu
 * Pocet barev odpovida getColorRange(), hodnot je colorRange * 3 (barvy)
 */
float* Colors::getUniqueColors() {	
	unsigned long range = getColorRange();
	float* colors = new float[range * 3]; // rgb = 3 hodnoty	
	
	// predchozi volani getColorRange zarucuje, ze step[] uz budou naplnene
	int i = 0;
	for (float r = 0; r <= 1.0f; r += step[RED]) {
		for (float g = 0; g <= 1.0f; g += step[GREEN]) {
			for (float b = 0; b <= 1.0f; b += step[BLUE]) {	
				if (r > 0 || g > 0 || b > 0) {
					colors[i] = r;
					colors[i+1] = g;
					colors[i+2] = b;
					i += 3;					
				}
			}
		}
	}
	
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