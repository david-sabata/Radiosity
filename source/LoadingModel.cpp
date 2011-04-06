#include "LoadingModel.h"


LoadingModel::LoadingModel(Patch* data, unsigned long count) {
	// zkopirovat dodane patche do vnitrniho uloziste
	for (unsigned long i=0; i < count; i++) {
		patches->push_back(new Patch(data[i]));
	}
}



vector<Patch*>* LoadingModel::getPatches(double area) {
	if (area > 0)
		subdivide(area);

	return patches;
}