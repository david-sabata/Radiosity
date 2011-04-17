#include "Patch.h"

Patch::Patch() {
	for (int i = 0; i < 8; i++)
		neighbours[i] = NULL;
}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(Vector3f(0.0f, 0.0f, 0.0f)), radiosity(Vector3f(0.0f, 0.0f, 0.0f)), illumination(Vector3f(0.0f, 0.0f, 0.0f))  {
	for (int i = 0; i < 8; i++)
		neighbours[i] = NULL;
}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(color), radiosity(Vector3f(0.0f, 0.0f, 0.0f)), illumination(Vector3f(0.0f, 0.0f, 0.0f)) {
	for (int i = 0; i < 8; i++)
		neighbours[i] = NULL;
}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color, Vector3f illumination)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(color), illumination(illumination), radiosity(Vector3f(0.0f, 0.0f, 0.0f)) {
	for (int i = 0; i < 8; i++)
		neighbours[i] = NULL;
}

Patch::Patch(Vector3f vec1, Vector3f vec2, Vector3f vec3, Vector3f vec4, Vector3f color, Vector3f illumination, Vector3f radiosity)
		: vec1(vec1), vec2(vec2), vec3(vec3), vec4(vec4), 
			color(color), illumination(illumination), radiosity(radiosity) {
	for (int i = 0; i < 8; i++)
		neighbours[i] = NULL;
}


Patch::~Patch(void) {
}



/**
 * Vraci dynamicky alokovany vektor plosek, maji polovicni
 * delku hrany. Pokud je obsah plosky mensi nebo roven zadanemu,
 * vraci NULL
 */
vector<Patch*>* Patch::divide(double area) {	
	
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


	double a = sqrt(area); // idealni delka strany rozdeleneho patche (za predpokladu ze je ctvercovy)	
	
	// predpokladame, ze patch bude vicemene ctvercovy (prip. obdelnikovy), tedy ze se 
	// vzdy dve a dve strany budou delit stejnym poctem
	unsigned int kx = unsigned int( ceil((B - A).f_Length() / a) ); // pocet dilku na ktere se bude delit v sirce
	unsigned int ky = unsigned int( ceil((C - B).f_Length() / a) ); // pocet dilku na ktere se bude delit v delce

	// pomerne casti hran puvodniho patche
	Vector3f pCD = (C - D) / float(kx); 
	Vector3f pAB = (B - A) / float(kx); 

	vector<Patch*>* patches = new vector<Patch*>;

	// [0, 0] je vlevo dole (odpovida bodu A)		
	for (unsigned int i = 0; i < kx * ky; i++) {

			unsigned int col = i % kx;
			unsigned int row = i / kx;

			Vector3f bCD = pCD * float(col) + D; // pozice rezu na horni strane
			Vector3f bAB = pAB * float(col) + A; // pozice rezu na dolni strane
			Vector3f nA = bAB + (( bCD - bAB ) / float(ky)) * float(row);

			Vector3f bCD1 = pCD * float(col + 1) + D; // pozice rezu na horni strane
			Vector3f bAB1 = pAB * float(col + 1) + A; // pozice rezu na dolni strane
			Vector3f nB = bAB1 + (( bCD1 - bAB1 ) / float(ky)) * float(row);

			Vector3f bCD2 = pCD * float(col + 1) + D; // pozice rezu na horni strane
			Vector3f bAB2 = pAB * float(col + 1) + A; // pozice rezu na dolni strane
			Vector3f nC = bAB2 + (( bCD2 - bAB2 ) / float(ky)) * float(row + 1);

			Vector3f bCD3 = pCD * float(col) + D; // pozice rezu na horni strane
			Vector3f bAB3 = pAB * float(col) + A; // pozice rezu na dolni strane
			Vector3f nD = bAB3 + (( bCD3 - bAB3 ) / float(ky)) * float(row + 1);
			
			Patch* p = new Patch( 							
							nA, nB, nC, nD,						
							this->color, this->illumination, this->radiosity 
						);
			
			patches->push_back(p);		
	}


	// dopocitat patchum sousedy
	for (unsigned int i = 0; i < patches->size(); i++) {
		Patch* p = patches->at(i);

		unsigned int col = i % kx;
		unsigned int row = i / kx;

		// 0 - levy horni
		if ( row + 1 >= ky ) { // prekroceni nahoru
			if (col > 0) { 
				p->neighbours[0] = patches->at( row * kx + (col - 1)); 				
			} else { 
				p->neighbours[0] = p; 
			}
		} 
		else if ( col == 0 ) { // prekroceni doleva
			if ( row + 1 < ky ) { 
				p->neighbours[0] = patches->at( (row + 1) * kx + col ); 
			} else { 
				p->neighbours[0] = p; 
			}
		}
		else { p->neighbours[0] = patches->at( (row + 1) * kx + (col - 1) ); }

		// 1 - horni
		if ( row + 1 >= ky ) { p->neighbours[1] = p; }
		else { p->neighbours[1] = patches->at( (row + 1) * kx + col ); }

		// 2 - pravy horni
		if ( row + 1 >= ky ) { // prekroceni nahoru
			if ( col + 1 < kx ) { 
				p->neighbours[2] = patches->at( row * kx + (col + 1) ); 
			} else { 
				p->neighbours[2] = p; 
			}
		} 
		else if ( col + 1 >= kx ) { // prekroceni doprava
			if ( row + 1 < ky ) { 
				p->neighbours[2] = patches->at( (row + 1) * kx + col ); 
			} else { 
				p->neighbours[2] = p;
			}
		} 
		else { p->neighbours[2] = patches->at( (row + 1) * kx + (col + 1) ); }

		// 3 - pravy
		if (col + 1 >= kx) { p->neighbours[3] = p; }
		else { p->neighbours[3] = patches->at( row * kx + (col + 1) ); }

		// 4 - pravy dolni
		if (row == 0) { // prekroceni dolu
			if ( col + 1 < kx ) { 
				p->neighbours[4] = patches->at( row * kx + (col + 1) ); 
			} else { 
				p->neighbours[4] = p; 
			}
		}
		else if ( col + 1 >= kx ) { // prekroceni doprava
			if ( row > 0 ) { 
				p->neighbours[4] = patches->at( (row - 1) * kx + col ); 
			} else { 
				p->neighbours[4] = p;
			}
		}
		else { p->neighbours[4] = patches->at( (row - 1) * kx + (col + 1) ); }

		// 5 - dolni
		if (row == 0) { p->neighbours[5] = p; }
		else { p->neighbours[5] = patches->at( (row - 1) * kx + col ); }

		// 6 - levy dolni
		if (row == 0) { // prekroceni dolu
			if ( col > 0 ) { 
				p->neighbours[6] = patches->at( row * kx + (col - 1) ); 
			} else { 
				p->neighbours[6] = p; 
			}
		}
		else if ( col == 0 ) { // prekroceni doleva
			if ( row > 0 ) { 
				p->neighbours[6] = patches->at( (row - 1) * kx + col ); 
			} else { 
				p->neighbours[6] = p; 
			}
		}
		else { p->neighbours[6] = patches->at( (row - 1) * kx + (col - 1) ); }

		// 7 - levy
		if (col == 0) { p->neighbours[7] = p; }
		else { p->neighbours[7] = patches->at( row * kx + (col - 1) ); }
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