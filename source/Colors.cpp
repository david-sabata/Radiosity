
#include "Colors.h"

// vyrobi masku pro b bitu; (bere ohled na masku pres vsechny bity - rozdeli masku na dve casti, prvni bit a zbytek)
#define MASK32(b) (((1 << ((b) - 1)) - 1) | (1 << ((b) - 1)))


short Colors::bits[3] = { 10, 10, 10 }; // vychozi hodnota, lze nastavit Colors::setBits
short Colors::shift[3] = {};
unsigned int Colors::mask[3] = {};
unsigned int Colors::revMask[3] = {};

unsigned int Colors::correction = 0;

unsigned int Colors::range = 0;

/**
 * Nastavi bitove rozsahy barev. Nutno nastavovat zvenci, trida Colors nema pristup
 * k framebufferu a nemuze na nem volat glGetIntegerv
 */
void Colors::setBits(unsigned short r, unsigned short g, unsigned short b) {
	bits[RED] = r;
	bits[GREEN] = g;
	bits[BLUE] = b;
}


/**
 * Na zaklade pozadovaneho poctu barev nastavi parametry mapovani barev do intu
 */
void Colors::setNeededColors(unsigned int colors) {
	++ colors;
	// cerna neni pouzita

	short m = int( ceil( log((double)colors) / log(2.0) ) ); // potrebny pocet bitu pro ulozeni 'colors' barev
	_ASSERT( (unsigned int)(1 << m) >= colors);
	
	short totbits = bits[RED] + bits[GREEN] + bits[BLUE]; // pocet bitu co mame
	short z = totbits - m; // pocet bitu co chceme zahodit	

	// pokud je pozadovany rozsah vyssi nez mame, orizneme jej
	if (z < 0) 
		z = 0;
		
	// ulozime si rozsah, at pak vime, kolik barev budeme generovat
	range = min(colors, (unsigned int)pow(2.0, totbits) );

	// pocet bitu co chci zahodit v kazde slozce
	short zr = z / 3;
	short zg = zr;
	short zb = z - 2 * zr; 

	// shifty jednotlivych slozek
	shift[RED] = zr;
	shift[GREEN] = zg + bits[RED];
	shift[BLUE] = zb + bits[RED] + bits[GREEN]; 

	// spocita pozici bitu s barvou v celkovem rozsahu barvy
	mask[RED] = MASK32(bits[RED] - zr);
	mask[GREEN] = MASK32(bits[GREEN] - zg);
	mask[BLUE] = MASK32(bits[BLUE] - zb);	

	// ted masky a shifty umoznuji vykosunout bity primo z cisla barvy	
	mask[GREEN] <<= bits[RED] - zr;
	mask[BLUE] <<= bits[RED] - zr + bits[GREEN] - zg;
	shift[GREEN] -= bits[RED] - zr;
	shift[BLUE] -= bits[RED] - zr + bits[GREEN] - zg;		

	// masky pro zpetny prevod barva=>id
	revMask[RED] = mask[RED] << shift[RED];
	revMask[GREEN] = mask[GREEN] << shift[GREEN];
	revMask[BLUE] = mask[BLUE] << shift[BLUE];

	// Pri zpetnem prevodu barvy na index se projevuje chyba kterou do barvy
	// zanasi GL tim, ze pouziva vnitrni reprezentaci ve floatech
	unsigned int tmp = 1 + (1 << bits[RED]) + (1 << (bits[RED] + bits[GREEN]));
	correction = (1 << (zr - 1)) | (1 << (bits[RED] + zg - 1)) | (1 << (bits[RED] + bits[GREEN] + zb - 1));
	correction -= tmp;
}






/**
 * Vraci pocet unikatnich barev snizeny o jednicku (cernou nepocitame), ktery lze zobrazit.
 * Jde tedy o velikost pole int-u generovaneho funkci getUniqueColors
 */
unsigned int Colors::getColorRange() {	
	return range - 1;
}


/**
 * Vraci barvu odpovidajiciho cisla (indexovane barvy) mapovanou do 32b unsigned intu
 */
uint32_t Colors::color(size_t colorIndex) {
	_ASSERT(colorIndex < range); // shifty a masky se vzdy vazi k danemu rozsahu; pro vyssi indexy nebudou barvy spravne
	return ((colorIndex & mask[RED]) << shift[RED]) | ((colorIndex & mask[GREEN]) << shift[GREEN]) | ((colorIndex & mask[BLUE]) << shift[BLUE]);	
}

/**
 * Vraci index odpovidajici zabalene barve - opacna operace k Colors::color()
 */
size_t Colors::index(uint32_t color) {	
	// ! pricist korekci (kompenzuje to, ze GL pracuje vnitrne s floatovymi barvami a pri prevodech vznikaji nepresnosti)
	color += correction;
	return ( ((color & revMask[RED]) >> shift[RED]) | ((color & revMask[GREEN]) >> shift[GREEN]) | ((color & revMask[BLUE]) >> shift[BLUE]) );
}

/**
 * Zabali barvu do 32b uintu (GL_UNSIGNED_INT_2_10_10_10_REV) - pouze RGB slozky
 */
uint32_t Colors::packColor(Vector3f color) {
	uint32_t r = unsigned int(color.x * 1023);
	uint32_t g = unsigned int(color.y * 1023);
	uint32_t b = unsigned int(color.z * 1023);
	uint32_t rg = r | (g << 10);
	uint32_t rgb = rg | (b << 20);

	return rgb;
}

/**
 * Rozbali barvu do vektoru z 32b uintu (GL_UNSIGNED_INT_2_10_10_10_REV) - pouze RGB slozky
 */
Vector3f Colors::unpackColor(uint32_t color) {
	Vector3f out;

	out.x = float(color &  MASK32(10)) / 1023;
	out.y = float(color & (MASK32(10) << 10)) / 1023;
	out.z = float(color & (MASK32(10) << 20)) / 1023;

	return out;
}


/**
 * Vraci pole int-u kde kazdy int obsahuje 'zabalene' tri slozky jedne barvy
 * Pocet barev (prvku pole) odpovida getColorRange()
 */
uint32_t* Colors::getUniqueColors() {	
	unsigned int range = getColorRange();
	uint32_t* colors = new uint32_t[range]; // 1 hodnota = 1 barva obsahujici zabalene 3 slozky
	
	for (unsigned int i = 0; i < range; i++)
		colors[i] = color(i+1);

	return colors;
}

/**
 * Vraci pole floatu, kde kazde tri hodnoty tvori barvu a vzdy
 * 6 po sobe jdoucich barev je stejnych. Patch se sklada ze dvou
 * trojuhelniku, tj. 4 (unikatnich) vrcholu, ktere museji mit stejnou barvu
 */
uint32_t* Colors::getIndicesColors() {
	unsigned int indices = 4; // pocet indexu se stejnou barvou
	unsigned long range = getColorRange();
	uint32_t* uniqueColors = getUniqueColors();
	uint32_t* indicesColors = new uint32_t[range * indices];
	
	for (unsigned int i = 0; i < range; i++) { // i = aktualne klonovana barva		
		for (unsigned int j = 0; j < indices; j++) { // j = cislo vrcholu se stejnou barvou
			indicesColors[i * indices + j] = uniqueColors[i];		
		}
	}

	delete[] uniqueColors;

	return indicesColors;
}