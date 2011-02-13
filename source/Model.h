#pragma once

#include <vector>
#include <deque>
#include "Patch.h"
#include "Vector.h"

class Model {
	
	public:
		Model(void);
		~Model(void);
				
		virtual vector<Patch*>* getPatches(double area = 0) = 0;	// vraci vektor patchu

	protected:	

		void subdivide(double area);	// provede nad modelem subdivision

		vector<Patch*>* patches;	// dynamicky alokovany vektor plosek modelu
};

