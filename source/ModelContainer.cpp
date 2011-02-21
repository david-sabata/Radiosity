
#include "ModelContainer.h"


ModelContainer::ModelContainer(void) {
	vertices = NULL;
	indices = NULL;
	verticesCount = 0;
	indicesCount = 0;
	
	maxPatchArea = 0; // defaultne bez deleni

	needRefresh = false;
}


ModelContainer::~ModelContainer(void) {
	// vyprazdnit vektor modelu
	for ( std::vector<Model*>::iterator it = models.begin(); it < models.end(); it++ )
		delete (*it);

	// vyprazdnit pole vrcholu a indexu
	delete [] vertices;
	delete [] indices;
}


/**
 * Docasna funkce pro naplneni sceny modely
 */
void ModelContainer::load() {	

	if (1) {
		Model* room = new PrimitiveModel( PrimitiveModel::ROOM );
		Model* cube = new PrimitiveModel( PrimitiveModel::CUBE );
		Model* block = new PrimitiveModel( PrimitiveModel::BLOCK );

		addModel(room);
		addModel(cube);
		addModel(block);
	}

	if (0) {
		//Model* test = new WaveFrontModel("../models/simple_box.obj");
		Model* test = new WaveFrontModel("../models/chair.obj");
		//Model* test = new WaveFrontModel("../models/fleurOptonl.obj");
		addModel(test);
	}
}


/**
 * Prida do sceny novy objekt a vraci jeho index
 */
int ModelContainer::addModel(Model *m) {
	needRefresh = true;
	models.push_back(m);
	return models.size() - 1;
}

/**
 * Odebere ze sceny model
 */
void ModelContainer::removeModel(int i) {
	needRefresh = true;
	delete models[i];
	models.erase( models.begin() + i );
}


/**
 * Naplni vnitrni promenne pro pocty a pole vrcholu/indexu aktualnimi hodnotami
 */
void ModelContainer::updateData() {
	
	// seskladat data z jednotlivych modelu do jedineho (pomocneho) vektoru
	vector<float>* tmpVertices = new vector<float>();
	vector<int>* tmpIndices = new vector<int>();

	// nove pocty vrcholu, indexu a patchu
	verticesCount = 0;
	indicesCount = 0;
	patchesCount = 0;

	int offset = 0; // pocet jiz vlozenych floatu - pro spravne provazani indexu a vrcholu

	for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++) {
		
		vector<Patch*>* patches = (*it)->getPatches( maxPatchArea );
		int ptchCnt = patches->size();
		patchesCount += ptchCnt;
		verticesCount += ptchCnt * 4 * 3; // ctvercove plosky = 4 vrcholy * 3 souradnice		
		indicesCount += ptchCnt * 6; // ploska = dva trojuhelniky = 6 indexu
		
		for (vector<Patch*>::iterator itP = patches->begin(); itP != patches->end(); itP++) {
			
			// zkopirovat souradnice
			vector<float> vCoords = (*itP)->getVerticesCoords();
			tmpVertices->insert(tmpVertices->end(), vCoords.begin(), vCoords.end());
						
			int baseOffset = offset;

			// vytvorit indexy
			tmpIndices->push_back(baseOffset);
			tmpIndices->push_back(baseOffset + 1);
			tmpIndices->push_back(baseOffset + 2);
			tmpIndices->push_back(baseOffset);
			tmpIndices->push_back(baseOffset + 2);
			tmpIndices->push_back(baseOffset + 3);

			offset += 4;
		}		
	}
		

	// obnovit pole vrcholu a indexu
	if (vertices != NULL)
		delete [] vertices;
	vertices = new float[verticesCount];

	if (indices != NULL)
		delete [] indices;
	indices = new int[indicesCount];

	// presunout data z pomocneho vektoru do pole
	copy(tmpVertices->begin(), tmpVertices->end(), vertices);
	copy(tmpIndices->begin(), tmpIndices->end(), indices);
	
	
	if (0) {
		for (int i = 0; i < indicesCount; i+=6)
			printf("%i %i %i | %i %i %i \n", indices[i], indices[i+1], indices[i+2], indices[i+3], indices[i+4], indices[i+5]);

		printf("\n");

		for (int i = 0; i < verticesCount; i+=3)
			printf("%f %f %f\n", vertices[i], vertices[i+1], vertices[i+2]);
	}


	//printf("scene vertices: %i\n scene indices: %i\n", verticesCount/3, indicesCount);
	printf("scene patches: %i\n", patchesCount);
	

	delete tmpVertices;
	delete tmpIndices;

	needRefresh = false;
}


/**
 * Vraci ukazatel na prvni prvek pole indexu
 */
int* ModelContainer::getIndices() {
	if (needRefresh == true)
		updateData();

	return indices;
}

/**
 * Vraci ukazatel na prvni prvek pole vrcholu
 */
float* ModelContainer::getVertices() {
	if (needRefresh == true)
		updateData();

	return vertices;
}

/**
 * Vraci pocet indexu v poli
 */
int ModelContainer::getIndicesCount() {
	if (needRefresh == true)
		updateData();

	return indicesCount;
}

/**
 * Vraci pocet vrcholu v poli
 */
int ModelContainer::getVerticesCount() {
	if (needRefresh == true)
		updateData();

	return verticesCount;
}


/**
 * Vraci pocet patchu ve scene
 */
unsigned int ModelContainer::getPatchesCount() {
	if (needRefresh == true)
		updateData();

	return patchesCount;
}

/**
 * Vraci referenci na i-tou plosku ve scene
 * Pouziva se zejmena pri vypoctech radiozity a pozicovani kamery
 */
Patch& ModelContainer::getPatch(unsigned int i) {
	unsigned int offset = 0; // cislovani patchu napric modely

	for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++) {		
		vector<Patch*>* patches = (*it)->getPatches( maxPatchArea );
		if (offset + patches->size() > i) {
			Patch* p = patches->at(i - offset);
			return (*p);
		}
		offset += patches->size();
	}

}