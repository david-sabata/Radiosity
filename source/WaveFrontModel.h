#pragma once
#include "model.h"
#include "Patch.h"
#include "Vector.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

class WaveFrontModel : public Model {
	
	public:
		WaveFrontModel(string filename);
		bool WaveFrontModel::parse(string filename);
		std::vector<Patch*>* getPatches(double area = 0);


};

