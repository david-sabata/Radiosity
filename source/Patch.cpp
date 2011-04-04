#include "Patch.h"


Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(Vector3f(0.0f, 0.0f, 0.0f)), radiosity(Vector3f(0.0f, 0.0f, 0.0f)), illumination(Vector3f(0.0f, 0.0f, 0.0f))  {}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(color), radiosity(Vector3f(0.0f, 0.0f, 0.0f)), illumination(Vector3f(0.0f, 0.0f, 0.0f)) {}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color, Vector3f illumination)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(color), illumination(illumination), radiosity(Vector3f(0.0f, 0.0f, 0.0f)) {}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color, Vector3f illumination, Vector3f radiosity)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(color), illumination(illumination), radiosity(radiosity) {}


Patch::~Patch(void) {
}



/**
 * Vraci dynamicky alokovany vektor plosek, maji polovicni
 * delku hrany. Pokud je obsah plosky mensi nebo roven zadanemu,
 * vraci NULL
 */
vector<Patch*>* Patch::divide(double area) {	

	// deleni bude probihat pouze pokud patch nema pocatecni vlastni energii
	//if (radiosity.x > 0 || radiosity.y > 0 || radiosity.z > 0)
	//	return NULL;


	/*
		vec4	vec3			D	CD	C
						-->		DA	E	BC
		vec1	vec2			A	AB	B
	*/
	
	//cout << "====================" << endl;
	//cout << "Deleni do plochy " << area << endl;

	Vector3f A = vec1;
	Vector3f B = vec2;
	Vector3f C = vec3;
	Vector3f D = vec4;
	
	/*
	cout << "A: " << A.x << "\t" << A.y << "\t" << A.z << endl;
	cout << "B: " << B.x << "\t" << B.y << "\t" << B.z << endl;
	cout << "C: " << C.x << "\t" << C.y << "\t" << C.z << endl;
	cout << "D: " << D.x << "\t" << D.y << "\t" << D.z << endl;
	*/

	// uhlopricky
	Vector3f u1 = A - C;
	Vector3f u2 = B - D;

	//cout << "u1: " << u1.x << " ; " << u1.y << " ; " << u1.z << endl;
	//cout << "u2: " << u2.x << " ; " << u2.y << " ; " << u2.z << endl;

	// uhel mezi uhloprickami
	double phi = acos( u1.f_Dot(u2) / (u1.f_Length() * u2.f_Length()) );
	//cout << "phi: " << phi << endl;

	// obsah patche
	double S = 0.5f * u1.f_Length() * u2.f_Length() * sin(phi);
	//cout << "S: " << S << endl;

	if (S <= area)
		return NULL;	

	/*
	// spocitat nove ctvrtinove patche
	Vector3f AB = (A + B) / 2;
	Vector3f BC = (B + C) / 2;
	Vector3f CD = (C + D) / 2;
	Vector3f DA = (D + A) / 2;
	Vector3f E  = (A + C) / 2;

	vector<Patch*>* patches = new vector<Patch*>;

	patches->push_back( new Patch(A, AB, E, DA, this->color, this->illumination, this->radiosity) );
	patches->push_back( new Patch(AB, B, BC, E, this->color, this->illumination, this->radiosity) );
	patches->push_back( new Patch(BC, C, CD, E, this->color, this->illumination, this->radiosity) );
	patches->push_back( new Patch(CD, D, DA, E, this->color, this->illumination, this->radiosity) );

	//cout << "====================" << endl;
	*/

	double w = (B - A).f_Length(); // sirka puvodniho patche
	double h = (C - B).f_Length(); // vyska puvodniho patche

	double a = sqrt(area); // idealni delka strany rozdeleneho patche (za predpokladu ze je ctvercovy)

	unsigned int kx = unsigned int( floor(w / a) ); // pocet dilku na ktere se bude delit v sirce
	unsigned int ky = unsigned int( floor(h / a) ); // pocet dilku na ktere se bude delit v delce
	
	double px = (w / kx) / w; // pomerna cast noveho dilku k sirce puvodniho
	double py = (h / ky) / h; // pomerna cast noveho dilku k delce puvodniho

	//Vector3f dx( (B - A) * float(px) ); // sirka noveho dilku
	//Vector3f dy( (D - A) * py ); // delka noveho dilku



	vector<Patch*>* patches = new vector<Patch*>;

	// [0, 0] je vlevo dole (odpovida bodu A)
	for (unsigned int col = 0; col < kx; col++) {
		for (unsigned int row = 0; row < ky; row++) {

			/*
			// souradnice rohu, relativne k A
			Vector3f x0; // X-ova souradnice leve strany patche
			Vector3f x1; // X-ova souradnice prave strany patche
			Vector3f y0; // Y-ova souradnice dolni strany patche
			Vector3f y1; // Y-ova souradnice horni strany patche
			
			// vypocist nove souradnice rohu
			if (col + 1 < kx) {
				x0 = Vector3f( dx * col );
				x1 = Vector3f( dx * (col + 1) );
			} else {
				x0 = Vector3f( dx * col );
				x1 = Vector3f( B - A );
			}

			if (row + 1 < ky) {
				y0 = Vector3f( dy * row );
				y1 = Vector3f( dy * (row + 1) );
			} else {
				y0 = Vector3f( dy * row );
				y1 = Vector3f( D - A );
			}
			*/

			Vector3f baseAB0 = Vector3f( (B - A) * px * col );
			Vector3f baseAB1 = Vector3f( (B - A) * px * (col + 1) );

			Vector3f baseBC0 = Vector3f( (C - B) * px * row );
			Vector3f baseBC1 = Vector3f( (C - B) * px * (row + 1) );

			Vector3f baseCD0 = Vector3f( (D - C) * px * col );
			Vector3f baseCD1 = Vector3f( (D - C) * px * (col + 1) );

			Vector3f baseAD0 = Vector3f( (D - A) * px * row );
			Vector3f baseAD1 = Vector3f( (D - A) * px * (row + 1) );


			Patch* p = new Patch( 
							/*(A + x0 + y0), (A + x1 + y0), (A + x1 + y1), (A + x0 + y1), */
							(A + baseAB0 + baseAD0), (A + baseAB1 + baseBC0), (A + baseCD1 + baseBC1), (A + baseCD0 + baseAD1), 
							this->color, this->illumination, this->radiosity 
						);

			patches->push_back(p);
		}
	}


	return patches;
}


/**
 * Vraci souradnice vrcholu patche nasypane v jedinem vektoru
 */
vector<float> Patch::getVerticesCoords() {
	vector<float> v;

	v.push_back(vec1.x);
	v.push_back(vec1.y);
	v.push_back(vec1.z);

	v.push_back(vec2.x);
	v.push_back(vec2.y);
	v.push_back(vec2.z);

	v.push_back(vec3.x);
	v.push_back(vec3.y);
	v.push_back(vec3.z);

	v.push_back(vec4.x);
	v.push_back(vec4.y);
	v.push_back(vec4.z);

	return v;
}

/**
 * Vraci Up vektor
 */
Vector3f Patch::getUp() {
	return Vector3f(vec4 - vec1);
}


/**
 * Vraci stred patche
 */
Vector3f Patch::getCenter() {
	return Vector3f(
		(vec1.x + vec2.x + vec3.x + vec4.x) / 4.0f,
		(vec1.y + vec2.y + vec3.y + vec4.y) / 4.0f,
		(vec1.z + vec2.z + vec3.z + vec4.z) / 4.0f
	);
}

/**
 * Vraci normalu patche
 */
Vector3f Patch::getNormal() {
	Vector3f A = vec2 - vec1;
	Vector3f B = vec4 - vec1;
	return A.Cross(B);
}

/**
 * Vraci vlastni barvu patche
 */
Vector3f Patch::getColor() {
	return color;
}

/**
 * Vraci odrazivost povrchu
 */
float Patch::getReflectivity() {
	return REFLECTIVITY;
	//return reflectivity;
}