
#include <vector>
#include <stdint.h>
#include "PrimitiveModel.h"
#include "WaveFrontModel.h"
#include "Vector.h"
#include "Timer.h"

using namespace std;

class ModelContainer {
	
	public:
		ModelContainer(void);
		~ModelContainer(void);

		void load();	// pomocna funkce pro naplneni sceny; TODO: nahradit obecnym nacitanim + lepe OOP

		int addModel(Model* m);	// prida model do sceny a vraci jeho index pro moznost pristupu
		void removeModel(int i);	// odebere ze sceny model s danym indexem	
		void updateData();	// naplni vnitrni promenne s vrcholy/idexy aktualnimi hodnotami

		float*	getVertices();	// vraci pole vrcholu patchu
		unsigned int	getVerticesCount();	// vraci delku pole vrcholu

		int*	getIndices();	// vraci pole vazeb mezi vrcholy
		unsigned int	getIndicesCount();	// vraci delku pole vazeb

		Patch**	getPatches(); // vraci pole vsech patchu ve scene (pokud je scena frozen, je vzdy konstantni)
		unsigned int	getPatchesCount(); // vraci pocet patchu ve scene
		unsigned int	getHighestRadiosityPatchId(); // vraci cislo patche s nejvetsi radiativni energii

		double maxPatchArea; // maximalni obsah plosek (pokud je vetsi nez 0, deli se plosky dokud neni plocha mensi)

	protected:
		bool needRefresh;	// pocty vrcholu a indexu a obsahy kontejneru nejsou aktualni

		std::vector<Model *> models; // pole modelu ve scene
		
		Patch** patches; // pole ukazatelu na patche ve scene o delce getPatchesCount, indexovano cislem patche
		unsigned int patchesCount;	// celkovy pocet patchu ve scene

		float* vertices;	// pole vrcholu, dynamicky alokovane
		unsigned int verticesCount;	// velikost pole vrcholu (pocet hodnot)

		int* indices;	// pole indexu souvisejicich vrcholu, dynamicky alokovane
		unsigned int indicesCount;	// velikost pole indexu (pocet hodnot)
};

