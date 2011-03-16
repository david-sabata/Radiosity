#pragma once

#include <vector>
#include "PrimitiveModel.h"
#include "WaveFrontModel.h"
#include "Vector.h"


// TODO:	rozsirit tridu na Scene s moznosti nacitani a funkce pro sireni radiozity
//			a tim uplne zapouzdrit praci s modely/patchi ?  Nebo alespon pridat moznost
//			nad nimi iterovat?

// TODO:	duplicitni hodnoty v modelech a v kontejneru!

// TODO:	rozsirit addModel o pozici nebo rovnou transformacni matici?

class ModelContainer {
	
	public:
		ModelContainer(void);
		~ModelContainer(void);

		void load();	// pomocna funkce pro naplneni sceny; TODO: nahradit obecnym nacitanim + lepe OOP

		int addModel(Model* m);	// prida model do sceny a vraci jeho index pro moznost pristupu
		void removeModel(int i);	// odebere ze sceny model s danym indexem	
		void updateData();	// naplni vnitrni promenne s vrcholy/idexy aktualnimi hodnotami

		float*	getVertices();	// vraci pole vrcholu patchu
		int		getVerticesCount();	// vraci delku pole vrcholu
		int*	getIndices();	// vraci pole vazeb mezi vrcholy
		int		getIndicesCount();	// vraci delku pole vazeb
		unsigned int getPatchesCount(); // vraci pocet patchu ve scene
		Patch& getPatch(unsigned int i); // vraci referenci na i-tou plosku ve scene
		unsigned int getHighestRadiosityPatchId(); // vraci cislo patche s nejvetsi energii

		double maxPatchArea; // maximalni obsah plosek (pokud je vetsi nez 0, deli se plosky dokud neni plocha mensi)

	protected:
		bool needRefresh;	// pocty vrcholu a indexu a obsahy kontejneru nejsou aktualni

		std::vector<Model *> models; // pole modelu ve scene
		
		float* vertices;	// pole vrcholu, dynamicky alokovane
		int verticesCount;	// velikost pole vrcholu (pocet hodnot)
		int* indices;	// pole indexu souvisejicich vrcholu, dynamicky alokovane
		int indicesCount;	// velikost pole indexu (pocet hodnot)
		unsigned int patchesCount;	// celkovy pocet patchu ve scene
};

