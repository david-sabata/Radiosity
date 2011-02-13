#include "Model.h"

extern vector<Patch*>* divide(double a);

Model::Model(void) {
	// alokovat novou pamet pro vektor plosek
	// plosky budou dynamicky alokovany pri generovani (prevod modelu ze zdrojove
	// formy na plosky) a uvolnovany v destruktoru ~Model
	patches = new vector<Patch*>();
}


Model::~Model(void) {
	// uvolnit z pameti plosky a nasledne cely vektor
	for (vector<Patch *>::iterator it = patches->begin(); it != patches->end(); it++)
		delete (*it);

	delete patches;
}


/**
 * Jednopruchodove deleni plosek modelu. Vysledkem
 * by mel byt vektor plosek naplneny ctvercovymi ploskami o maximalnim
 * obsahu "area". Samotne deleni zajistuje sama trida Patch
 */
void Model::subdivide(double area) {

	// prevest vektor na deque pro efektivnejsi praci s prvky
	deque<Patch*>* deq = new deque<Patch*>();
	deq->assign( patches->begin(), patches->end() ); 

	// postupne brat prvky ze zacatku, delit je a vkladat na konec
	int toBeChecked = deq->size();

	//int failsafe = 1000;

	while (toBeChecked > 0 /*&& --failsafe > 0*/) {
		Patch* p = deq->front();
		deq->pop_front();

		// deleni zjistuje sam patch
		vector<Patch*>* newPatches = p->divide(area);
		
		// jestlize je vysledkem deleni vektor, pripojit ho na konec fronty a puvodni patch odstranit
		if (newPatches != NULL) {
			// pripojit nove patche na konec fronty
			deq->insert(deq->end(), newPatches->begin(), newPatches->end() );

			// upravit pocet prvku ke zkontrolovani: odecist odebrany prvek a pricist jeho rozdelene casti
			toBeChecked = toBeChecked - 1 + newPatches->size();

			delete p;	// puvodni patch se nadobro zrusi
			delete newPatches;	// uvolnit i vektor; ukazatele na patche uz jsou ve fronte
		}
		// jinak pripojime odebrany patch na konec a povazujeme ho za zkontrolovany
		else {
			deq->push_back(p);
			toBeChecked--;
		}
	}

	// prevest zpet na vektor
	patches->reserve(deq->size());
	patches->assign( deq->begin(), deq->end() );
	delete deq;
}