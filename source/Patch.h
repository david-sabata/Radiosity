#pragma once

#include <vector>
#include <cmath>
#include "Vector.h"


#include <iostream>

using namespace std;


class Patch {
	
	public:
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4);
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Color4f color);
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Color4f color, Color4f energy);
		~Patch(void);

		vector<Patch*>* divide(double area);	// rozdeli sam sebe na mensi plosky a vraci jejich vektor
		vector<float> getVerticesCoords();	// vraci vsechny souradnice vrcholu nasypane v jedinem poli

		Vector3f getCenter();	// vraci bod v prostredu patche (pro umisteni kamery)
		Vector3f getNormal();	// vraci normalu
		Vector3f getUp();		// vraci pomocny Up vector; slouzi jako referencni bod pri otaceni pohledu
		Color4f  getColor();	// vraci vlastni barvu patche
		Color4f  getReflectivity();	// vraci odrazivost povrchu

		Color4f radiosity;	// radiozita - zarivost povrchu
		Color4f energy;		// take emissivity - vlastni zarivost plosky (pouze svetla)

	protected:
		Vector3f vec1, vec2, vec3, vec4;
		Color4f color;		// vychozi barva povrchu - pouzita pro color bleeding
		Color4f reflectivity;	// odrazivost plosky - pro vypocet kolik energie se pohlti/odrazi
		
};

