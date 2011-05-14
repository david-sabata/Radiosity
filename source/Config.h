#pragma once

#include <iostream>
using namespace std;

class Config {

	public:
		static void setHemicubeSide(unsigned int n); // nastavi delku strany hemicube; mela by byt mocninou 2
		static void setOCLWorkitemsX(unsigned int n); // nastavi horizontalni pocet instanci OpenCL kernelu, ktere budou zpracovavat jeden radek textury; idealne mocnina 2
		static void setMaxPatchArea(double n); // nastavi nejvyssi moznou plochu patche pro subdivision
		static void setShootsPerCycle(unsigned int n); // nastavi pocet 'vystrelu' radiosity behem jednoho pruchodu kreslici smycky

		static void freeze(); // zmrazi objekt a naalokuje potrebne struktury

		static unsigned int HEMICUBE_W();
		static unsigned int HEMICUBE_H();
		static unsigned int PATCHVIEW_TEX_W();
		static unsigned int PATCHVIEW_TEX_H();
		static unsigned int PATCHVIEW_TEX_RES();
		static double MAX_PATCH_AREA();
		static unsigned int OCL_WORKITEMS_X();
		static unsigned int OCL_WORKITEMS_Y();
		static unsigned int SHOOTS_PER_CYCLE();

	private:
		static bool frozen;

		static unsigned int hemicubeSide;
		static unsigned int oclWorkitemsX;
		static double maxPatchArea;
		static unsigned int shootsPerCycle;

};

