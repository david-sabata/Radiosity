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

	// Brat nerozdelene patche a vysledky jejich deleni pripojovat na konec vektoru.
	// Pokud se patch nerozdeli, proste se zkopiruje. Nakonec se zacatek pole (puvodni patche) odrizne
	unsigned int n_original_size = patches->size();	

	for (unsigned int i = 0; i < n_original_size; i++) {

		Patch* p = patches->at(i);
		vector<Patch*>* p_new_patches = p->divide(area);

		if (p_new_patches == NULL) {
			Patch* n = new Patch(*p);

			// patch se nerozdelil, proto nezna sve sousedy (pouzivane pro interpolaci kreslenych barev); nastavit sebe sama
			for (unsigned j = 0; j < 8; j++) {
				if (n->neighbours[j] == NULL) // osetreni pro nacitani, kde jsou patche jiz rozdelene a sousedy znaji
					n->neighbours[j] = n;
			}

			patches->push_back(n);
		} else {
			patches->insert(patches->end(), p_new_patches->begin(), p_new_patches->end());
		}

		delete p_new_patches;
	}

	// uvolnit puvodni patche
	for (vector<Patch*>::iterator it = patches->begin(); it != patches->begin() + n_original_size; it++) {
		delete (*it);
	}
	patches->erase(patches->begin(), patches->begin() + n_original_size);
}