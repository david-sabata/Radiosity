#pragma once

#include <vector>
#include <cmath>
#include "Vector.h"


#include <iostream>

using namespace std;

// TODO:	nepouzit rovnou trojuhelniky? jaka je interpretace polygonu na vystupu modelovacich sw?

class Patch {
	
	public:
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4);
		~Patch(void);

		vector<Patch*>* divide(double area);	// rozdeli sam sebe na mensi plosky a vraci jejich vektor

		vector<float> getVerticesCoords();	// vraci vsechny souradnice vrcholu nasypane v jedinem poli

	protected:
		Vector3f vec1, vec2, vec3, vec4;
		Color4f color;
		float radiosity;	// radiozita - zarivost povrchu
		float energy;		// take emissivity - vyzarovana energie plosky
		float reflectivity;	// odrazivost plosky
		
};

