
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "Config.h"
#include "OpenGL30Drv.h"

using namespace std;


/**
 * Predpocita form factory a naplni jimi pole p_hemicube_formfactors tak,
 * ze strukturou odpovidaji rozlozeni pohledu v texture (detail v FormFactors.cpp)
 * Rozmery textury jsou pro urychleni vypoctu pevne dane
 *
 * Vraci ukazatel na p_hemicube_formfactors
 */
float* precomputeHemicubeFormFactors();


void FBO2BMP();


struct BMP {
    unsigned __int32 *bytes;
    int n_Width, n_Height;
};

int	Save_TrueColor_BMP(char *filename, BMP *bmp);
