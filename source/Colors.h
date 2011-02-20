
// pro ucely testovani omezi rozsahy barev pri vnitrnim vykreslovani
//#define LIMIT_INTERNAL_COLORS


#include <limits>
#include <cmath>

using namespace std;


class Colors {

	private:				
		static float step_r;
		static float step_g;
		static float step_b;

		static float getStepRed();
		static float getStepGreen();
		static float getStepBlue();
		
	public:
		static unsigned long getColorRange();
		static float* getUniqueColors();
		static float* getIndicesColors();

};


