
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

// rozmery hemicube, resp. pohledove textury
const unsigned int HEMICUBE_W = 256;
const unsigned int HEMICUBE_H = 256;

const unsigned int PATCHVIEW_TEX_W = unsigned int(HEMICUBE_W * 2);
const unsigned int PATCHVIEW_TEX_H = unsigned int(HEMICUBE_H * 1.5);
const unsigned int PATCHVIEW_TEX_RES = unsigned int(PATCHVIEW_TEX_W * PATCHVIEW_TEX_H);


using namespace std;


/**
 * Predpocita form factory a naplni jimi pole p_hemicube_formfactors tak,
 * ze strukturou odpovidaji rozlozeni pohledu v texture (detail v FormFactors.cpp)
 * Rozmery textury jsou pro urychleni vypoctu pevne dane
 *
 * Vraci ukazatel na p_hemicube_formfactors
 */
float* precomputeHemicubeFormFactors();





struct BMP {
    unsigned __int32 *bytes;
    int n_Width, n_Height;
};

int	Save_TrueColor_BMP(char *filename, BMP *bmp);
