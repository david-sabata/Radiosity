#include "Config.h"

bool Config::frozen = false;

// nastavovano zvenci
unsigned int	Config::hemicubeSide = 128;
unsigned int	Config::oclWorkitemsX = 4;
unsigned int	Config::shootsPerCycle = 500;
double			Config::maxPatchArea = 0.5;
unsigned int	Config::hemicubesCount = 10;


// nastavovano vnitrne
unsigned int _HEMICUBE_W = 0;
unsigned int _HEMICUBE_H = 0;
unsigned int _PATCHVIEW_TEX_W = 0;
unsigned int _PATCHVIEW_TEX_H = 0;
unsigned int _PATCHVIEW_TEX_RES = 0;

double _MAX_PATCH_AREA = 0;

unsigned int _OCL_WORKITEMS_X = 0;
unsigned int _OCL_WORKITEMS_Y = 0;
unsigned int _OCL_SPANLENGTH = 0;


/**
 * @brief zmrazi objekt a nastavi potrebna data
 */
void Config::freeze() {
	frozen = true;


	_HEMICUBE_W = hemicubeSide;
	_HEMICUBE_H = hemicubeSide;

	_PATCHVIEW_TEX_W = unsigned int(_HEMICUBE_W * 2);
	_PATCHVIEW_TEX_H = unsigned int(_HEMICUBE_H * 1.5);
	_PATCHVIEW_TEX_RES = unsigned int(_PATCHVIEW_TEX_W * _PATCHVIEW_TEX_H);

	_MAX_PATCH_AREA = maxPatchArea;

	_OCL_WORKITEMS_X = min(oclWorkitemsX, _PATCHVIEW_TEX_W);
	_OCL_WORKITEMS_Y = _PATCHVIEW_TEX_H * hemicubesCount;
	_OCL_SPANLENGTH = _PATCHVIEW_TEX_W / _OCL_WORKITEMS_X;


}


/**
 * @brief nastavi delku strany hemicube; mela by byt mocninou 2
 */
void Config::setHemicubeSide(unsigned int n) {
	if (frozen) {
		cerr << "Error: Trying to modify frozen configuration" << endl;
		return;
	}
		
	hemicubeSide = n;
}

/**
 * @brief nastavi horizontalni pocet instanci OpenCL kernelu, ktere budou zpracovavat jeden radek textury; idealne mocnina 2
 */
void Config::setOCLWorkitemsX(unsigned int n) {
	if (frozen) {
		cerr << "Error: Trying to modify frozen configuration" << endl;
		return;
	}

	oclWorkitemsX = n;
}


/**
 * @brief nastavi nejvyssi moznou plochu patche pro subdivision
 */
void Config::setMaxPatchArea(double n) {
	if (frozen) {
		cerr << "Error: Trying to modify frozen configuration" << endl;
		return;
	}

	maxPatchArea = n;
}


/**
 * @brief nastavuje pocet 'vystrelu' radiosity behem jednoho kresliciho cyklu
 */
void Config::setShootsPerCycle(unsigned int n) {
	if (frozen) {
		cerr << "Error: Trying to modify frozen configuration" << endl;
		return;
	}

	shootsPerCycle = n;
}


/**
 * @brief nastavuje pocet 'vystrelu' radiosity behem jednoho kresliciho cyklu
 */
void Config::setHemicubesCount(unsigned int n) {
	if (frozen) {
		cerr << "Error: Trying to modify frozen configuration" << endl;
		return;
	}

	hemicubesCount = n;
}


unsigned int Config::HEMICUBE_W() {
	return _HEMICUBE_W;
}

unsigned int Config::HEMICUBE_H() {
	return _HEMICUBE_H;
}

unsigned int Config::PATCHVIEW_TEX_W() {
	return _PATCHVIEW_TEX_W;
}

unsigned int Config::PATCHVIEW_TEX_H() {
	return _PATCHVIEW_TEX_H;
}

unsigned int Config::PATCHVIEW_TEX_RES() {
	return _PATCHVIEW_TEX_RES;
}

double Config::MAX_PATCH_AREA() {
	return _MAX_PATCH_AREA;
}

unsigned int Config::OCL_WORKITEMS_X() {
	return _OCL_WORKITEMS_X;
}

unsigned int Config::OCL_WORKITEMS_Y() {
	return _OCL_WORKITEMS_Y;
}

unsigned int Config::SHOOTS_PER_CYCLE() {
	return shootsPerCycle;
}

unsigned int Config::HEMICUBES_CNT() {
	return hemicubesCount;
}