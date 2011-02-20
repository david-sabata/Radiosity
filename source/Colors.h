
// pro ucely testovani omezi rozsahy barev pri vnitrnim vykreslovani
//#define LIMIT_INTERNAL_COLORS


#include <limits>
#include <cmath>
#include <algorithm>

using namespace std;


class Colors {

	private:				
		static float step[3];	
		static unsigned int bits[3];

		static float getStep(unsigned int color);
		static unsigned int getBits(unsigned int color);

		static bool decreaseBits();
		static unsigned long neededColors;

		enum {
			RED = 0,
			GREEN,
			BLUE
		};
		
	public:
		static void setNeededColors(unsigned long colors);
		static unsigned long getColorRange();
		static float* getUniqueColors();
		static float* getIndicesColors();

};


