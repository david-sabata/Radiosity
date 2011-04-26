#pragma once

#include <vector>
#include <cmath>
#include "Vector.h"


#include <iostream>

using namespace std;

// odrazivost povrchu
#define REFLECTIVITY 0.3f


class Patch {
	
	/*
	iluminativni energie - jiz vyzarena energie
	radiativni energie - energie smerujici do patche
						svetla maji ve vychozim stavu radiativni energii nastavenou + iluminativni prazdnou

	pri prenosu se vezme radiativni energie smerujici do patche, cast jde do iluminativni (osvetli patch)
	a cast	

	*/

	public:
		Patch::Patch();
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4);
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color);
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color, Vector3f illumination);
		Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color, Vector3f illumination, Vector3f radiosity);
		~Patch(void);

		vector<Patch*>* divide(double area);	// rozdeli sam sebe na mensi plosky a vraci jejich vektor
		vector<float> getVerticesCoords();	// vraci vsechny souradnice vrcholu nasypane v jedinem poli

		Vector3f getCenter();	// vraci bod v prostredu patche (pro umisteni kamery)
		Vector3f getNormal();	// vraci normalu
		Vector3f getUp();		// vraci pomocny Up vector; slouzi jako referencni bod pri otaceni pohledu
		Vector3f getColor();	// vraci vlastni barvu patche
		float    getReflectivity();	// vraci odrazivost povrchu
		Patch**  getNeighbours(); // vraci pole ukazatelu na sousedici patche

		Vector3f radiosity;	// radiozita - energie vyzarena z povrchu
		Vector3f illumination;	// osvetlenost plosky (zde se scitaji svetla ktera dopadla na plosku)

		unsigned int relativeNeighbours[8]; // relativni odkazy na sousedici patche, cislovano v ramci sceny; pouziva se pouze pri ulozeni jako nahrada ukazatelu
		Patch* neighbours[8]; // ukazatele na sousedici patche - plni se az pri skladani sceny; cislovano z leveho horniho rohu; pokud soused neni, ukazuje na sebe		

	protected:
		Vector3f vec1, vec2, vec3, vec4;
		Vector3f color;		// vychozi barva povrchu - pouzita pro color bleeding
		//float reflectivity;	// odrazivost plosky - pro vypocet kolik energie se pohlti/odrazi				
};


/**
 * Vraci odrazivost povrchu
 */
inline float Patch::getReflectivity() {
	return REFLECTIVITY;
	//return reflectivity;
}

/**
 * Vraci vlastni barvu patche
 */
inline Vector3f Patch::getColor() {
	return color;
}