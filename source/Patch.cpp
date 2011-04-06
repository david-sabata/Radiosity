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
	
	// tolerance 0.1% - pri deleni muzou vznikat miniaturni chyby, ktere by zbytecne 
	if (S <= (area * 1.01) )
		return NULL;	

	/* puvodni deleni na ctvrtiny - nutne volat rekurzivne
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

	double a = sqrt(area); // idealni delka strany rozdeleneho patche (za predpokladu ze je ctvercovy)	
	
	// predpokladame, ze patch bude vicemene ctvercovy (prip. obdelnikovy), tedy ze se 
	// vzdy dve a dve strany budou delit stejnym poctem
	unsigned int kx = unsigned int( ceil((B - A).f_Length() / a) ); // pocet dilku na ktere se bude delit v sirce
	unsigned int ky = unsigned int( ceil((C - B).f_Length() / a) ); // pocet dilku na ktere se bude delit v delce

	// pomerne casti hran puvodniho patche
	Vector3f pCD = (C - D) / kx; 
	Vector3f pAB = (B - A) / kx; 

	//cout << "kx: " << kx << "\t\tky:" << ky << endl;

	vector<Patch*>* patches = new vector<Patch*>;

	// [0, 0] je vlevo dole (odpovida bodu A)
	for (unsigned int col = 0; col < kx; col++) {
		for (unsigned int row = 0; row < ky; row++) {			

			Vector3f bCD = pCD * col + D; // pozice rezu na horni strane
			Vector3f bAB = pAB * col + A; // pozice rezu na dolni strane
			Vector3f nA = bAB + (( bCD - bAB ) / ky) * row;

			Vector3f bCD1 = pCD * (col + 1) + D; // pozice rezu na horni strane
			Vector3f bAB1 = pAB * (col + 1) + A; // pozice rezu na dolni strane
			Vector3f nB = bAB1 + (( bCD1 - bAB1 ) / ky) * row;

			Vector3f bCD2 = pCD * (col + 1) + D; // pozice rezu na horni strane
			Vector3f bAB2 = pAB * (col + 1) + A; // pozice rezu na dolni strane
			Vector3f nC = bAB2 + (( bCD2 - bAB2 ) / ky) * (row + 1);

			Vector3f bCD3 = pCD * col + D; // pozice rezu na horni strane
			Vector3f bAB3 = pAB * col + A; // pozice rezu na dolni strane
			Vector3f nD = bAB3 + (( bCD3 - bAB3 ) / ky) * (row + 1);

			
			/*
			cout << "col " << col << "\t\trow" << row << endl;
			cout << "A\t\t" << A.x << "\t" << A.y << "\t" << A.z << endl;
			cout << "B\t\t" << B.x << "\t" << B.y << "\t" << B.z << endl;
			cout << "C\t\t" << C.x << "\t" << C.y << "\t" << C.z << endl;
			cout << "D\t\t" << D.x << "\t" << D.y << "\t" << D.z << endl;
			cout << "B-A\t\t" << (B-A).x << "\t" << (B-A).y << "\t" << (B-A).z << endl;
			cout << "(B-A)/kx\t" << ((B-A)/kx).x << "\t" << ((B-A)/kx).y << "\t" << ((B-A)/kx).z << endl;
			cout << "((B-A)/kx)*col\t" << (((B-A)/kx)*col).x << "\t" << (((B-A)/kx)*col).y << "\t" << (((B-A)/kx)*col).z << endl;
			cout << "C-D\t\t" << (C-D).x << "\t" << (C-D).y << "\t" << (C-D).z << endl;
			cout << "(C-D)/kx\t" << ((C-D)/kx).x << "\t" << ((C-D)/kx).y << "\t" << ((C-D)/kx).z << endl;
			cout << "((C-D)/kx)*col\t" << (((C-D)/kx)*col).x << "\t" << (((C-D)/kx)*col).y << "\t" << (((C-D)/kx)*col).z << endl;
			cout << "nA\t\t" << nA.x << "\t" << nA.y << "\t" << nA.z << endl;						
			cout << endl;
			*/
			
			Patch* p = new Patch( 							
							nA, nB, nC, nD,						
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