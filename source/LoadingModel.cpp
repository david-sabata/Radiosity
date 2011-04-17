#include "LoadingModel.h"


LoadingModel::LoadingModel(Patch* data, unsigned long count) {
	// zkopirovat dodane patche do vnitrniho uloziste
	for (unsigned long i = 0; i < count; i++) {
		patches->push_back(new Patch(data[i]));
	}

	// nahradit relativni sousedy ukazateli pro rychlejsi pristup pri kresleni
	for (unsigned long i = 0; i < count; i++) {
		Patch* p = patches->at(i);
		for (unsigned int n = 0; n < 8; n++) { // cele osmiokoli			
			p->neighbours[n] = patches->at( p->relativeNeighbours[n] );
		}
	}
}



vector<Patch*>* LoadingModel::getPatches(double area) {
	//if (area > 0)
	//	subdivide(area);

	return patches;
}