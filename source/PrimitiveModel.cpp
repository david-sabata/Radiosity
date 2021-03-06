#include "PrimitiveModel.h"


PrimitiveModel::PrimitiveModel(int type) {
	PrimitiveModel::type = type;
	
	// nacist plosky z preddefinovanych poli: kazde 3 hodnoty jsou vertex, kazde 4 vertexy tvori patch, tj. 12 hodnot
	switch(type) {
		case ROOM:
			{
				for (int i=0; i < 60; i += 12) {
					Vector3f vec1 = Vector3f(room[i], room[i+1], room[i+2]);
					Vector3f vec2 = Vector3f(room[i+3], room[i+4], room[i+5]);
					Vector3f vec3 = Vector3f(room[i+6], room[i+7], room[i+8]);
					Vector3f vec4 = Vector3f(room[i+9], room[i+10], room[i+11]);
					int offset = 3 * (i / 12);
					Vector3f col = Vector3f(roomColors[offset], roomColors[offset + 1], roomColors[offset + 2]);
					patches->push_back( new Patch(vec1, vec2, vec3, vec4, col) );
				}
				// svetlo
				{
					Vector3f vec1 = Vector3f(3.430f, 5.485f, 2.270f);
					Vector3f vec2 = Vector3f(2.130f, 5.485f, 2.270f);					
					Vector3f vec3 = Vector3f(2.130f, 5.485f, 3.320f);
					Vector3f vec4 = Vector3f(3.430f, 5.485f, 3.320f);					
					Vector3f col = Vector3f(1.0f, 1.0f, 1.0f);
					Vector3f energy = Vector3f(1.0f, 1.0f, 1.0f);
					patches->push_back( new Patch(vec1, vec2, vec3, vec4, col, energy, energy * 100) ); // radiativni i iluminativni
				}
			}
			break;
		case ROOMCLOSURE:			
			{
				int i = 0;
				Vector3f vec1 = Vector3f(roomClosure[i], roomClosure[i+1], roomClosure[i+2]);
				Vector3f vec2 = Vector3f(roomClosure[i+3], roomClosure[i+4], roomClosure[i+5]);
				Vector3f vec3 = Vector3f(roomClosure[i+6], roomClosure[i+7], roomClosure[i+8]);
				Vector3f vec4 = Vector3f(roomClosure[i+9], roomClosure[i+10], roomClosure[i+11]);
				Vector3f col = Vector3f(roomClosureColors[0], roomClosureColors[1], roomClosureColors[2]);
				patches->push_back( new Patch(vec1, vec2, vec3, vec4, col) );			
			}
			break;
		case CUBE:
			for (int i=0; i < 60; i += 12) {
				Vector3f vec1 = Vector3f(cube[i], cube[i+1], cube[i+2]);
				Vector3f vec2 = Vector3f(cube[i+3], cube[i+4], cube[i+5]);
				Vector3f vec3 = Vector3f(cube[i+6], cube[i+7], cube[i+8]);
				Vector3f vec4 = Vector3f(cube[i+9], cube[i+10], cube[i+11]);
				int offset = 3 * (i / 12);
				Vector3f col = Vector3f(cubeColors[offset], cubeColors[offset + 1], cubeColors[offset + 2]);
				patches->push_back( new Patch(vec1, vec2, vec3, vec4, col) );
			}
			break;
		case BLOCK:
			for (int i=0; i < 60; i += 12) {
				Vector3f vec1 = Vector3f(block[i], block[i+1], block[i+2]);
				Vector3f vec2 = Vector3f(block[i+3], block[i+4], block[i+5]);
				Vector3f vec3 = Vector3f(block[i+6], block[i+7], block[i+8]);
				Vector3f vec4 = Vector3f(block[i+9], block[i+10], block[i+11]);
				int offset = 3 * (i / 12);
				Vector3f col = Vector3f(blockColors[offset], blockColors[offset + 1], blockColors[offset + 2]);
				patches->push_back( new Patch(vec1, vec2, vec3, vec4, col) );
			}
			break;
	}
	
}


PrimitiveModel::~PrimitiveModel(void) {	
}


/**
 * Vraci vektor ctvercovych plosek (patchu) modelu. 
 * Nepovinny parametr "a" udava maximalni delku hrany plosky. Pokud je
 * vetsi nez nula, zpusobi to vnitrni volani fce subdivision()
 */
vector<Patch*>* PrimitiveModel::getPatches(double area) {
	if (area > 0)
		subdivide(area);

	return patches;
}



#define COLOR_WHITE 1.0f, 1.0f, 1.0f
#define COLOR_RED 1.0f, 0.0f, 0.0f
#define COLOR_GREEN 0.0f, 1.0f, 0.0f

const float PrimitiveModel::room[] = {
	
	0.0f,	0.0f,	5.592f,  
	5.496f,	0.0f,	5.592f,		// 0 zadni stena		
	5.560f,	5.488f,	5.592f,
	0.0f,	5.488f,	5.592f,  
	

	0.0f,	5.488f,	0.0f,		// 4 horni stena 
	0.0f,	5.488f,	5.592f,
	5.560f,	5.488f,	5.592f,  
	5.560f,	5.488f,	0.0f,			
	
	0.0f,	0.0f,	0.0f,		// 8 dolni stena 
	5.528f,	0.0f,	0.0f,			
	5.496f,	0.0f,	5.592f, 
	0.0f,	0.0f,	5.592f,	
		 		 
	0.0f,	0.0f,	0.0f,		// 12 prava stena 	
	0.0f,	0.0f,	5.592f,		
	0.0f,	5.488f,	5.593f,
	0.0f,	5.488f,	0.0f,  
		 
	5.496f,	0.0f,	5.592f,		// 16 leva stena	
	5.528f,	0.0f,	0.0f,		
	5.560f,	5.488f,	0.0f,	
	5.560f,	5.488f,	5.592f,   	  	
};

const float PrimitiveModel::roomColors[] = {
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_RED,
	COLOR_GREEN
};


const float PrimitiveModel::roomClosure[] = {	
	5.528f,	0.0f,	0.0f,	// predni stena uzavirajici krabici
	0.0f,	0.0f,	0.0f,
	0.0f,	5.488f,	0.0f,
	5.560f,	5.488f,	0.0f,
};

const float PrimitiveModel::roomClosureColors[] = {
	COLOR_WHITE
};


const float PrimitiveModel::cube[] = { // krychle
	
	1.3f,	1.65f,	0.65f,		// 0 strop
	2.9f,	1.65f,	1.14f,	
	2.4f,	1.65f,	2.72f,
	0.82f,	1.65f,	2.25f,

	2.9f,	0.0f,	1.14f,		// 4	 
	2.4f,	0.0f,	2.72f,	
	2.4f,	1.65f,	2.72f,
	2.9f,	1.65f,	1.14f,   

	1.3f,	0.0f,	0.65f,		// 8	
	2.9f,	0.0f,	1.14f,
	2.9f,	1.65f,	1.14f,    
	1.3f,	1.65f,	0.65f,   	

	0.82f,	0.0f,	2.25f,		// 12	
	1.3f,	0.0f,	0.65f,
	1.3f,	1.65f,	0.65f,    
	0.82f,	1.65f,	2.25f,   	

	2.4f,	0.0f,	2.72f,		// 16	
	0.82f,	0.0f,	2.25f,
	0.82f,	1.65f,	2.25f,    
	2.4f,	1.65f,	2.72f,   	
};

const float PrimitiveModel::cubeColors[] = {
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE
};



const float PrimitiveModel::block[] = {
	4.23f,	3.3f,	2.47f,	// 0 kvadr	
	4.72f,	3.3f,	4.06f,	
	3.14f,	3.3f,	4.56f,    
	2.65f,	3.3f,	2.96f,   	

	4.23f,	0.0f,	2.47f,	// 4	
	4.72f,	0.0f,	4.06f,
	4.72f,	3.3f,	4.06f,    
	4.23f,	3.3f,	2.47f,   	

	4.72f,	0.0f,	4.06f,	// 8	
	3.14f,	0.0f,	4.56f,
	3.14f,	3.3f,	4.56f,    
	4.72f,	3.3f,	4.06f,   	

	3.14f,	0.0f,	4.56f,	// 12	
	2.65f,	0.0f,	2.96f,
	2.65f,	3.3f,	2.96f,    
	3.14f,	3.3f,	4.56f,   	

	2.65f,	0.0f,	2.96f,	// 16	
	4.23f,	0.0f,	2.47f,
	4.23f,	3.3f,	2.47f,    
	2.65f,	3.3f,	2.96f,   	
};

const float PrimitiveModel::blockColors[] = {
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE,
	COLOR_WHITE
};
