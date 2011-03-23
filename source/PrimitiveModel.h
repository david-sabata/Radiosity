#pragma once

#include "model.h"

using namespace std;


class PrimitiveModel : public Model {
	
	public:
		PrimitiveModel(int type);
		~PrimitiveModel(void);
		
		vector<Patch*>* getPatches(double area = 0); // vraci pole plosek modelu (rozdelenych na max. obsah "area")

		const enum {
			ROOM,
			ROOMCLOSURE,
			CUBE,
			BLOCK
		};

	private:		
		int type;	// typ modelu - hodnota z enumu; pouze docasne
		static const float room[];
		static const float roomClosure[];
		static const float cube[];
		static const float block[];
		static const int roomIndices[];
		static const int roomClosureIndices[];
		static const int cubeIndices[];
		static const int blockIndices[];

		// barvy materialu - vzdy jedna (tri floaty) barva pro ctyri po sobe jdouci vrcholy
		static const float roomColors[];
		static const float roomClosureColors[];
		static const float cubeColors[];
		static const float blockColors[];
};

