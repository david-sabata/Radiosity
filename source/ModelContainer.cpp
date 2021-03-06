
#include "ModelContainer.h"


ModelContainer::ModelContainer(void) {
	vertices = NULL;
	indices = NULL;
	verticesCount = 0;
	indicesCount = 0;
	
	patches = NULL;
	patchesCount = 0;

	maxPatchArea = 0; // defaultne bez deleni

	needRefresh = false;
}


ModelContainer::~ModelContainer(void) {
	// vyprazdnit vektor modelu
	for ( std::vector<Model*>::iterator it = models.begin(); it < models.end(); it++ )
		delete (*it);

	// uklidit
	delete [] vertices;
	delete [] indices;
	delete [] patches;
}


/**
 * Docasna funkce pro naplneni sceny modely
 */
void ModelContainer::load() {	

	if (1) {
		Model* room = new PrimitiveModel( PrimitiveModel::ROOM );
		Model* cube = new PrimitiveModel( PrimitiveModel::CUBE );
		Model* block = new PrimitiveModel( PrimitiveModel::BLOCK );

		Model* roomClosure = new PrimitiveModel( PrimitiveModel::ROOMCLOSURE );

		addModel(room);
		addModel(roomClosure);
		addModel(cube);
		addModel(block);
	}

	else {
		//Model* test = new WaveFrontModel("../models/simple_box.obj");
		//Model* test = new WaveFrontModel("../models/chair.obj");
		Model* test = new WaveFrontModel("../models/VUT_final.obj");
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
	vector<Patch*>* tmpPatches = new vector<Patch*>();

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

			// overit, zda existuji sousedi
			for (unsigned j = 0; j < 8; j++) {
				if ((*itP)->neighbours[j] == NULL) // osetreni pro nacitani, kde jsou patche jiz rozdelene a sousedy znaji
					(*itP)->neighbours[j] = (*itP);
			}

			// zkopirovat patch
			tmpPatches->push_back((*itP));

			offset += 4;
		}		
	}		

	// obnovit pole vrcholu, indexu a patchu
	if (vertices != NULL)
		delete [] vertices;
	vertices = new float[verticesCount];

	if (indices != NULL)
		delete [] indices;
	indices = new int[indicesCount];

	if (patches != NULL)
		delete [] patches;
	patches = new Patch*[patchesCount];

	// presunout data z pomocneho vektoru do pole
	copy(tmpVertices->begin(), tmpVertices->end(), vertices);
	copy(tmpIndices->begin(), tmpIndices->end(), indices);
	copy(tmpPatches->begin(), tmpPatches->end(), patches);
	
	delete tmpVertices;
	delete tmpIndices;
	delete tmpPatches;

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
unsigned int ModelContainer::getIndicesCount() {
	if (needRefresh == true)
		updateData();

	return indicesCount;
}

/**
 * Vraci pocet vrcholu v poli
 */
unsigned int ModelContainer::getVerticesCount() {
	if (needRefresh == true)
		updateData();

	return verticesCount;
}

/**
 * Vraci pole ukazatelu na patche o delce patchesCount
 */
Patch** ModelContainer::getPatches() {
	if (needRefresh == true)
		updateData();

	return patches;
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
 * Vraci ID patche s nejvyssi radiositou
 */
unsigned int ModelContainer::getHighestRadiosityPatchId() {
	if (needRefresh == true)
		updateData();

	unsigned int maxI = 0;
	Patch* max = NULL;

	for (unsigned int pi = 0; pi < patchesCount; pi++) {
		if (max == NULL || patches[pi]->radiosity.f_Length2() > max->radiosity.f_Length2()) {
			max = patches[pi];
			maxI = pi;
		}
	}

	return maxI;
}


/**
 * Srovnavaci funkce pro ID patchu na zaklade jejich energii
 */
bool ModelContainer::compareEnergies(unsigned int a, unsigned int b) {
	return patches[a]->radiosity.f_Length2() < patches[b]->radiosity.f_Length2();
}


class Comparator {
	public:
		Patch** patches;
		bool Comparator::operator()(unsigned int a, unsigned int b) { 
			return patches[a]->radiosity.f_Length2() < patches[b]->radiosity.f_Length2();
		}
};

/**
 * Naplni pole ID a ukazatelu daty 'count' patchu s nejvetsi energii
 */
void ModelContainer::getHighestRadiosityPatchesId(unsigned int count, Patch** p_emitters, unsigned int* p_emitters_ids) {
	if (needRefresh == true)
		updateData();

	list<unsigned int> tops;
	Comparator c = Comparator();
	c.patches = patches;
	
	for (unsigned int pi = 0; pi < patchesCount; pi++) {
		if (
			tops.empty() || 
			(patches[pi]->radiosity.f_Length2() > 0 && patches[tops.back()]->radiosity.f_Length2() <= patches[pi]->radiosity.f_Length2()) 
		) {
			tops.push_back(pi);
			tops.sort(c);
			tops.reverse(); // seradit od nejvyssiho k nejmensimu

			if (tops.size() > count) {
				list<unsigned int>::iterator it = tops.begin();
				for (unsigned int i = 0; i < count; i++)
					it++;
				tops.erase(it, tops.end());
			}
		}
	}

	// zkopirovat data na vystup
	list<unsigned int>::iterator it = tops.begin();
	for (unsigned int i = 0; i < count; i++) {
		if (it == tops.end()) {
			p_emitters_ids[i] = 0;
			p_emitters[i] = NULL;
			continue;
		}

		p_emitters_ids[i] = (*it);
		p_emitters[i] = patches[ p_emitters_ids[i] ];
		it++;
	}
	
}


