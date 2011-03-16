
#include <crtdbg.h>
#include <limits>
#include <cmath>
#include <algorithm>
#include <stdint.h>
#include <math.h>
#include "Vector.h"

using namespace std;


class Colors {

	private:				

		static short bits[3]; // bitove rozsahy pro kazdou z barev		
		static short shift[3]; // posun pro vykousnuti barvy z namapovaneho intu
		static unsigned int mask[3]; // maska pro vykousnuti barvy z namapovaneho intu

		static unsigned int range;

		enum {
			RED = 0,
			GREEN,
			BLUE
		};		
		
		
	public:
		static void setBits(unsigned short r, unsigned short g, unsigned short b);
		static void setNeededColors(unsigned int colors);

		static unsigned int getColorRange();
		static uint32_t* getUniqueColors();
		static uint32_t* getIndicesColors();

		static uint32_t packColor(Vector3f color);	// vygeneruje barvu z indexu do GL_UNSIGNED_INT_2_10_10_10_REV
		static Vector3f unpackColor(uint32_t color); // prevede barvu z GL_UNSIGNED_INT_2_10_10_10_REV do vektoru

		static uint32_t color(size_t colorIndex); // vraci GL_UNSIGNED_INT_2_10_10_10_REV barvu odpovidajici indexu
		static size_t index(uint32_t color); // vraci index odpovidajici GL_UNSIGNED_INT_2_10_10_10_REV zabalene barve
};


