
#include <vector>
#include "model.h"

/**
 * Falesny model, pouzivany pro nacitani sceny ze souboru
 * V konstruktoru se pouze naplni patchi ktere jiz muzou mit nejake energie. ModelContainer z nich 
 * potom rekonstruuje celou scenu.
 */
class LoadingModel : public Model {
	
	public:
		LoadingModel(Patch* data, unsigned long count);

		vector<Patch*>* getPatches(double area);
};

