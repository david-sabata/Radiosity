
#include "WaveFrontModel.h"


WaveFrontModel::WaveFrontModel(string filename) {
	if (!parse(filename))
		cerr << "Unable to load model '" << filename << "'" << endl;
}


/**
 * Vlastni parsovani souboru - primo vklada patche do vnitrniho uloziste
 * @todo normaly? textury? groupy?
 */
bool WaveFrontModel::parse(string filename) {	
	ifstream f(filename, ifstream::in);

	// pro nacitani jednotlivych radku
	string buffer;

	// citac radku, jen pro hezci vypis chyb
	unsigned int line = 0;

	// pro udrzeni vertexu, dokud se z nich nebudou faces
	vector<Vector3f>* vertices = new vector<Vector3f>();

	bool ignoreGroup = false;

	// parsovat po radcich
	while (f.good()) {
		getline(f, buffer);
		line++;

		// group
		if (false && buffer.find("g ") == 0) {
			if (buffer.find("_idle") != string::npos)
				ignoreGroup = true;
			else
				ignoreGroup = false;
		}

		if (ignoreGroup)
			continue;

		// vertex
		if (buffer.find("v ") == 0) {
			buffer.erase(0, 2);

			// pro podrzeni tri bodu vrcholu
			vector<float> points;

			// rozsekat na body a prevest
			istringstream str(buffer);
			float x;
			while (str >> x) {
				points.push_back(x);				
			}
			
			// zkontrolovat
			if (points.size() < 3) {
				cerr << "Bad vertex definition in '" << filename << "', line " << line << endl;
				return false;
			}
			if (points.size() > 3) {
				cerr << "Warning: Ignoring [w] coordinate in '" << filename << "', line " << line << endl;
			}

			vertices->push_back(Vector3f(points[0]/1000, points[1]/1000, points[2]/1000));
			continue;
		}

		// face
		if (buffer.find("f ") == 0) {
			buffer.erase(0, 2);

			// pro podrzeni cisel ctyr vertexu
			vector<unsigned int> faceVertices;

			// rozsekat na body a prevest
			while (buffer.size() > 0 && faceVertices.size() <= 4) {
				size_t pos = buffer.find_first_of(' ');				
				string vertex = buffer.substr(0, pos);					
				 
				// prevod na cislo odmazne lomitko a cokoliv za nim - textury ani normaly zatim nejsou implementovane
				istringstream str(vertex);
				int vertexI;
				if (str >> vertexI)
					faceVertices.push_back(vertexI - 1); // vrcholy v souboru jsou cislovane od 1 !

				// odmazat prevedenou cast
				if (pos == string::npos)
					buffer.erase();
				else
					buffer.erase(0, pos+1);
			}			

			// zkontrolovat
			if (faceVertices.size() != 4) {

				// duplikovat posledni vrchol - degenerovany ctverec
				//faceVertices.push_back( faceVertices.back() );

				//cerr << "Warning: Triangle face ignored in '" << filename << "', line " << line << endl;
				continue;
			}

			//cout << faceVertices[0] << "\t" << faceVertices[1] << "\t" << faceVertices[2] << "\t" << faceVertices[3] << endl;

			try {
			// vytvorit patch			
			Patch* patch = new Patch( vertices->at(faceVertices[0]), vertices->at(faceVertices[1]), vertices->at(faceVertices[2]), vertices->at(faceVertices[3]) );
			patches->push_back(patch);
			} catch (out_of_range e) {

			}
			continue;
		}
		
		// vse ostatni ignorovat

		//cout << buffer << endl;
	}

	/*
	// dump vertices
	int i = 0;
	for (vector<Vector3f>::iterator it = vertices->begin(); it != vertices->end(); it++) {
		cout << i << ":\t" << (*it).x << "\t" << (*it).y << "\t" << (*it).z << endl;
		i++;
	}
	*/
	
	// uklidit
	delete vertices;

	// uzavrit soubor
	f.close();
	
	return true;
}


vector<Patch*>* WaveFrontModel::getPatches(double area) {
	if (area > 0)
		subdivide(area);

	return patches;
}
