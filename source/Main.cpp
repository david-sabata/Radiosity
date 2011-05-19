#pragma warning(disable:4996)

#include "Main.h"

//std::vector<Patch*> lightPatches;
std::vector<Vector3f> initialEnergies;

unsigned int lightPatchID_min = UINT_MAX;
unsigned int lightPatchID_max = 0;
std::vector<unsigned int> lightPatchIDs;
std::vector<Vector3f> lightPatchCoords, lightPatchCoordsCopy;

/**
 *	@brief vytvori vsechny OpenGL objekty, potrebne pro kresleni
 *	@return vraci true pri uspechu, false pri neuspechu
 */
bool InitGLObjects() {		
	glEnable(GL_CULL_FACE);

	// vyrobime buffer objekt, do ktereho zkopirujeme geometrii naseho modelu 
	// (do jednoho bufferu ulozime vsechny souradnice, tzn. texturovaci / normaly / barvy / pozice, atp. 
	// jeden buffer muze obsahovat data pro jeden nebo i vice modelu)
	glGenBuffers(1, &n_vertex_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, n_vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, scene.getVerticesCount() * sizeof(float), scene.getVertices(), GL_STATIC_DRAW);	

	// vytvorime druhy buffer objekt, do ktereho zkopirujeme indexy vrcholu naseho modelu (jedna se o jiny typ bufferu)
	glGenBuffers(1, &n_index_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, n_index_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene.getIndicesCount() * sizeof(int), scene.getIndices(), GL_STATIC_DRAW);		
	

	// VBO pro ulozeni barev podle ID patchu - bude obsahovat jeden interval barev (kazda barva zabalena do intu)
	glGenBuffers(1, &n_id_color_buffer_object);	
	glBindBuffer(GL_ARRAY_BUFFER, n_id_color_buffer_object);	
	// naplnit unikatnimi barvami
	{
		Colors::setNeededColors(scene.getPatchesCount()); // idealni rozsah barev - pro optimalizaci generovani
		uint32_t* colorData = Colors::getIndicesColors(); // colorRange * 4 indexy
		unsigned int colorRange = Colors::getColorRange();		
		glBufferData(GL_ARRAY_BUFFER, colorRange * 4 * sizeof(uint32_t), colorData, GL_STATIC_DRAW);		
		delete[] colorData;	// data jsou uz zkopirovana ve VBO
	}


	// VBO pro ulozeni zobrazovanych barev patchu (iluminativni energie)
	glGenBuffers(1, &n_patch_color_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
	// na zacatku maji iluminativni energii pouze svetla
	{
		Patch** patches = scene.getPatches();
		unsigned int indCnt = scene.getPatchesCount() * 4; // pocet neopakujicich se indexu
		float* colorData = new float[indCnt * 3]; // kazdy vrchol ma 3 barevne slozky
		for (unsigned int i = 0; i < indCnt * 3; i += 3) {
			//colorData[i] = Colors::packColor( patches[i/4]->illumination * patches[i/4]->getColor() ); // vychozi illuminance patchu (maji jen svetla)
			Vector3f col = patches[i/12]->illumination * patches[i/12]->getColor();
			colorData[i] = col.x;
			colorData[i+1] = col.y;
			colorData[i+2] = col.z;
		}
		glBufferData(GL_ARRAY_BUFFER, indCnt * 3 * sizeof(float), colorData, GL_DYNAMIC_DRAW);
		delete[] colorData;
	}

	// VBO pro ulozeni radiativnich energii patchu 
	glGenBuffers(1, &n_patch_radiative_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, n_patch_radiative_buffer_object);
	{
		Patch** patches = scene.getPatches();
		unsigned int indCnt = scene.getPatchesCount() * 4; // pocet neopakujicich se indexu
		float* colorData = new float[indCnt * 3]; // kazdy vrchol ma 3 barevne slozky
		for (unsigned int i=0; i < indCnt * 3; i += 3) {
			//colorData[i] = Colors::packColor( patches[i/4]->radiosity );
			Vector3f col = patches[i/12]->radiosity;
			colorData[i] = col.x;
			colorData[i+1] = col.y;
			colorData[i+2] = col.z;
		}
		glBufferData(GL_ARRAY_BUFFER, indCnt * 3 * sizeof(uint32_t), colorData, GL_DYNAMIC_DRAW);
		delete[] colorData;
	}


	// VBO pro ulozeni souradnic textur - TODO: resit uz pri nacitani
	glGenBuffers(1, &n_tex_buffer_object);	
	glBindBuffer(GL_ARRAY_BUFFER, n_tex_buffer_object);	
	float* texCoords = new float[(scene.getVerticesCount() / 3) * 2];
	unsigned int offset = 0; // offset v texCoords
	for (unsigned int i = 0; i < scene.getVerticesCount() / 3; i++) {
		switch( i % 4 ) {
			case 0:
				texCoords[offset] = 0.0f;
				texCoords[offset + 1] = 0.0f;
				break;
			case 1:
				texCoords[offset] = 1.0f;
				texCoords[offset + 1] = 0.0f;
				break;
			case 2:
				texCoords[offset] = 1.0f;
				texCoords[offset + 1] = 1.0f;
				break;
			case 3:
				texCoords[offset] = 0.0f;
				texCoords[offset + 1] = 1.0f;
				break;
		}
		offset += 2;
	}
	glBufferData(GL_ARRAY_BUFFER, (scene.getVerticesCount() / 3) * 2 * sizeof(float), texCoords, GL_STATIC_DRAW);
	delete[] texCoords;
		
	// VAO definuje ktery VBO se preda shaderu a jak se data naskladaji do vstupnich atributu
	glGenVertexArrays(1, &n_vertex_array_object);
	glBindVertexArray(n_vertex_array_object);
	{		
		// rekneme OpenGL odkud si ma brat data; kazdy vrchol ma 3 souradnice,		
		glBindBuffer(GL_ARRAY_BUFFER, n_vertex_buffer_object);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(0));
	
		// rekneme OpenGL odkud si ma brat data pro 1. atribut shaderu; kazda barva se sklada ze 3 floatu
		glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
		glEnableVertexAttribArray(1);
		//glVertexAttribPointer(1, 4, GL_UNSIGNED_INT_2_10_10_10_REV, false, 0, p_OffsetInVBO(0));
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, p_OffsetInVBO(0));

		// rekneme OpenGL odkud si ma brat souradnice textur; souradnice se sklada ze dvou hodnot	
		glBindBuffer(GL_ARRAY_BUFFER, n_tex_buffer_object);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(0));

		// rekneme OpenGL odkud bude brat indexy geometrie pro glDrawElements (nize)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, n_index_buffer_object);
		
	} // tento blok se "zapamatuje" ve VAO
	
	// vratime VAO 0, abychom si nahodne VAO nezmenili (pripadne osetreni 
	// proti chybe v ovladacich nvidia kde se VAO poskodi pri volani nekterych wgl funkci)
	// vytvorime a nastavime vertex array objekt - objekt na GPU, ktery si pamatuje konfiguraci 
	// nastaveni VBO. pak pri kresleni volame jen glBindVertexArray() namisto vsech prikazu v bloku vyse
	glBindVertexArray(0); 
	

	
	// rozdelit patche na intervaly: from - prvni patch v intervalu; to - patch za poslednim patchem v intervalu
	int patchCount = scene.getIndicesCount() / 6;
	int divided = 0;
	do {
		int from = divided;
		int to = divided + Colors::getColorRange();
		if (to > patchCount)
			to = patchCount;

		interval intv = { from, to };
		patchIntervals.push_back(intv);
		divided = to;
	} while (divided < patchCount);

	// inicializovat pole VAO
	n_color_array_object = new GLuint[patchIntervals.size()];

	cout << "     intervals: " << patchIntervals.size()<< endl;

	// naplnime pole VAO s odpovidajicimi bindy VBO
	for (unsigned int i = 0; i < patchIntervals.size(); i++) {

		glGenVertexArrays(1, &n_color_array_object[i]);
		glBindVertexArray(n_color_array_object[i]);
		{		
			// rekneme OpenGL odkud si ma brat data; kazdy vrchol ma 3 souradnice,		
			glBindBuffer(GL_ARRAY_BUFFER, n_vertex_buffer_object);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(3 * 4 * patchIntervals[i].from * sizeof(float)));
			
			// rekneme OpenGL odkud si ma brat data pro 1. atribut shaderu; kazda barva ma 3 hodnoty,		
			glBindBuffer(GL_ARRAY_BUFFER, n_id_color_buffer_object);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_UNSIGNED_INT_2_10_10_10_REV, false, 0, p_OffsetInVBO(0));

			// rekneme OpenGL odkud bude brat indexy geometrie pro glDrawElements
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, n_index_buffer_object);		

		} // tento blok se "zapamatuje" ve VAO	

		// vratit VAO 0
		glBindVertexArray(0);
	}

	
	// geometrie a texurovani nahledoveho ctverce pro pohledy z patchu
	// 3 souradnice vrcholu, 2 souradnice textury
	const float p_square[] = {
		// stredni ctverec
		-1, -1, 0, 0, 0,
		+1, -1, 0, 1, 0,
		+1, +1, 0, 1, 1,
		
		-1, -1, 0, 0, 0,
		+1, +1, 0, 1, 1,
		-1, +1, 0, 0, 1,

		// horni pulka
		-1, +1, 0, 0, 0,
		+1, +1, 0, 1, 0,
		+1, +2, 0, 1, 1,

		-1, +1, 0, 0, 0,
		+1, +2, 0, 1, 1,
		-1, +2, 0, 0, 1,

		// dolni pulka
		-1, -1, 0, 0, 1,
		+1, -1, 0, 1, 1,
		+1, -2, 0, 1, 0,

		-1, -1, 0, 0, 1,
		+1, -2, 0, 1, 0,
		-1, -2, 0, 0, 0,

		// leva pulka
		-1, -1, 0, 1, 0,
		-1, +1, 0, 1, 1,
		-2, +1, 0, 0, 1,

		-1, -1, 0, 1, 0,		
		-2, +1, 0, 0, 1,
		-2, -1, 0, 0, 0,

		// prava pulka
		+1, +1, 0, 0, 1,
		+1, -1, 0, 0, 0,
		+2, -1, 0, 1, 0,

		+1, +1, 0, 0, 1,
		+2, -1, 0, 1, 0,
		+2, +1, 0, 1, 1,
	};
	glGenBuffers(1, &n_vbo_square);
	glBindBuffer(GL_ARRAY_BUFFER, n_vbo_square);
	glBufferData(GL_ARRAY_BUFFER, 30 * 5 * sizeof(float), p_square, GL_STATIC_DRAW);

	glGenVertexArrays(1, &n_vao_square);
	glBindVertexArray(n_vao_square);
	{		
		// rekneme OpenGL odkud si ma brat data; kazdy vrchol ma 3 souradnice,		
		glBindBuffer(GL_ARRAY_BUFFER, n_vbo_square);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), p_OffsetInVBO(0));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), p_OffsetInVBO(3 * sizeof(float)));
	} // tento blok se "zapamatuje" ve VAO
	
	glBindVertexArray(0); 


	// ziskat slinkovany a zkontrolovany program (vertex + fragment shader) pro uzivatelsky pohled
	n_user_program_object = Shaders::getUserViewProgram();		
	// najde cislo parametru shaderu podle jeho jmena
	n_user_mvp_matrix_uniform = glGetUniformLocation(n_user_program_object, "t_modelview_projection_matrix");
	

	// ziskat slinkovany a zkontrolovany program (vertex + fragment shader) pro pohled z patche
	n_patch_program_object = Shaders::getPatchViewProgram();	
	// najde cislo parametru shaderu podle jeho jmena
	n_patchprogram_mvp_matrix_uniform = glGetUniformLocation(n_patch_program_object, "t_modelview_projection_matrix");
	

	// ziskat slinkovany a zkontrolovany program pro vykresleni nahledoveho krize
	n_preview_program_object = Shaders::getPreviewProgram();
	// najde cislo parametru shaderu podle jeho jmena
	n_preview_mvp_matrix_uniform = glGetUniformLocation(n_preview_program_object, "t_modelview_projection_matrix");
	// najde cislo texturovaciho parametru
	n_box_sampler_uniform = glGetUniformLocation(n_preview_program_object, "n_tex");

	// nabindovat pomocnou texturu k FBO
	glGenTextures(1, &n_patchlook_texture);
	glBindTexture(GL_TEXTURE_2D, n_patchlook_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Config::PATCHVIEW_TEX_W(), Config::PATCHVIEW_TEX_H() * Config::HEMICUBES_CNT(), 0, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	


	// vytvorit FBO pro kresleni pohledu z patchu do textury
	fbo = new CGLFrameBufferObject(
		Config::PATCHVIEW_TEX_W(), Config::PATCHVIEW_TEX_H() * Config::HEMICUBES_CNT(),
		1, true,
		0, 0,
		true, false,
		GL_DEPTH_COMPONENT24, 0,
		false, false, 0, 0);

	if (fbo->b_Status() == false) {
		cerr << "Unable to create FBO" << endl;
		return false;
	}
	

	// predpocitat form factory
	p_formfactors = precomputeHemicubeFormFactors();
	
	// alokovat misto pro vystup formfactoru z kernelu
	p_tmp_formfactors = new float[patchCount];
	for (int i = 0; i < patchCount; i++)
		p_tmp_formfactors[i] = 0;


	unsigned int HEMICUBE_W = Config::HEMICUBE_W();
	unsigned int HEMICUBE_H = Config::HEMICUBE_H();
	unsigned int HEMICUBES_CNT = Config::HEMICUBES_CNT();

	// pripravit misto pro docasne hodnoty radiosit patchu ze kterych se prave strili (v pripade vice hemicubes)
	p_tmp_radiosities = new Vector3f[HEMICUBES_CNT];


	// 'okna' do kterych se budou kreslit jednotlive pohledy; odpovida 'nakresu' v FormFactors.cpp
	// x, y, w, h		(0,0 = levy dolni roh)
	p_viewport_list = new int**[HEMICUBES_CNT];
	for (unsigned int hi = 0; hi < HEMICUBES_CNT; hi++) {
		p_viewport_list[hi] = new int*[5];
		for (unsigned int i = 0; i < 5; i++) {
			p_viewport_list[hi][i] = new int[4];
			p_viewport_list[hi][i][2] = HEMICUBE_W;
			p_viewport_list[hi][i][3] = HEMICUBE_H;

			switch (i) {
				case 0: // nahoru
					p_viewport_list[hi][i][0] = 0;
					p_viewport_list[hi][i][1] = HEMICUBE_H + int(HEMICUBE_W * 1.5 * hi);
					break;
				case 1: // dolu
					p_viewport_list[hi][i][0] = HEMICUBE_W;
					p_viewport_list[hi][i][1] = HEMICUBE_H/2  + int(HEMICUBE_W * 1.5 * hi);
					break;
				case 2: // vlevo
					p_viewport_list[hi][i][0] = -1 * int(HEMICUBE_W/2);
					p_viewport_list[hi][i][1] = 0 + int(HEMICUBE_W * 1.5 * hi);
					break;
				case 3: // vpravo
					p_viewport_list[hi][i][0] = int(HEMICUBE_W*1.5);
					p_viewport_list[hi][i][1] = 0 + int(HEMICUBE_W * 1.5 * hi);
					break;
				case 4: // pred sebe
					p_viewport_list[hi][i][0] = HEMICUBE_W/2;
					p_viewport_list[hi][i][1] = 0 + int(HEMICUBE_W * 1.5 * hi);
					break;
			}
		}	
	}

	// oblasti v texture, do kterych je povoleno kreslit;
	// jelikoz se nektere casti kresli pres sebe, muze pri kresleni 'pruhledna' vznikat
	// nezadouci zviditelneni drive vykreslene casti pohledu, ktery ale ma byt skryty, coz resi glScissor
	// 
	// x, y, w, h
	p_scissors_list = new int**[HEMICUBES_CNT];
	for (unsigned int hi = 0; hi < HEMICUBES_CNT; hi++) {
		p_scissors_list[hi] = new int*[5];
		for (unsigned int i = 0; i < 5; i++) {
			p_scissors_list[hi][i] = new int[4];

			if (i == 0 || i == 1) {
				p_scissors_list[hi][i][1] = HEMICUBE_H + int(HEMICUBE_W * 1.5 * hi);
				p_scissors_list[hi][i][3] = HEMICUBE_H/2;
			} else {
				p_scissors_list[hi][i][1] = 0 + int(HEMICUBE_W * 1.5 * hi);
				p_scissors_list[hi][i][3] = HEMICUBE_H;
			}

			if (i == 2 || i == 3)
				p_scissors_list[hi][i][2] = HEMICUBE_W/2;
			else
				p_scissors_list[hi][i][2] = HEMICUBE_W;
		
			switch (i) {
				case 0:
				case 2:
					p_scissors_list[hi][i][0] = 0;
					break;
				case 1:
					p_scissors_list[hi][i][0] = HEMICUBE_W;
					break;
				case 3:
					p_scissors_list[hi][i][0] = int(HEMICUBE_W*1.5);
					break;
				case 4:
					p_scissors_list[hi][i][0] = HEMICUBE_W/2;
					break;
			}
		}
	}


	// pripravit pole pro patche s nejvyssi energii
	p_emitters = new Patch*[Config::HEMICUBES_CNT()];
	p_emitters_ids = new unsigned int[Config::HEMICUBES_CNT()];

	return true;
}

int errorMessage(cl_int e) { cout << e; return e; }

/**
 *  @brief vytvori vsechny OpenCL objekty, potrebne pro vypocty na GPU
 *  @return vraci true pri uspechu, false pri neuspechu
 */
bool InitCLObjects() {
	
	// Platform
	cl_int error = clGetPlatformIDs(1, &ocl_platform, NULL);
	if (error != CL_SUCCESS) {
	   cerr << "Error getting platform id: " << errorMessage(error) << endl;
	   return false;
	}
	// Device
	error = clGetDeviceIDs(ocl_platform, CL_DEVICE_TYPE_GPU, 1, &ocl_device, NULL);
	if (error != CL_SUCCESS) {
	   cerr << "Error getting device ids: " << errorMessage(error) << endl;
	   return false;
	}
	// Context - vytvoreny z OpenGL contextu
	cl_context_properties props[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)driver.GetContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)driver.GetDevice(),
		0
	};
	ocl_context = clCreateContext(props, 1, &ocl_device, NULL, NULL, &error);
	if (error != CL_SUCCESS) {
	   cerr << "Error creating context: " << errorMessage(error) << endl;
	   return false;
	}
	// Command-queue
	ocl_queue = clCreateCommandQueue(ocl_context, ocl_device, 0, &error);
	if (error != CL_SUCCESS) {
	   cerr << "Error creating command queue: " << errorMessage(error) << endl;
	   return false;
	}
	
	unsigned int PATCHVIEW_TEX_W = Config::PATCHVIEW_TEX_W();
	unsigned int PATCHVIEW_TEX_H = Config::PATCHVIEW_TEX_H();
	unsigned int PATCHVIEW_TEX_RES = Config::PATCHVIEW_TEX_RES();
	unsigned int HEMICUBES_CNT = Config::HEMICUBES_CNT();
	
	// pole ID patchu a jejich energii (na shodnych indexech)
	ocl_arg_hemicubes = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, PATCHVIEW_TEX_RES * HEMICUBES_CNT * sizeof(uint32_t), NULL, &error);
	ocl_arg_ids = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, PATCHVIEW_TEX_RES * HEMICUBES_CNT * sizeof(uint32_t), NULL, &error);
	ocl_arg_energies = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, PATCHVIEW_TEX_RES * HEMICUBES_CNT * sizeof(float), NULL, &error);

	// index do ID patchu a energii, sdileny mezi instacemi, atomicky posouvany
	ocl_arg_writeindex = clCreateBuffer(ocl_context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &error);

	// data textury pohledu z nejakeho patche
	ocl_arg_patchview = clCreateFromGLTexture2D(ocl_context, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, n_patchlook_texture, &error);

	// data 'textury' form factoru
	ocl_arg_ffactors = clCreateBuffer(ocl_context, CL_MEM_READ_ONLY, PATCHVIEW_TEX_RES * HEMICUBES_CNT * sizeof(float), NULL, &error);

	_ASSERT(error == CL_SUCCESS);
	
	// nacist zdrojovy kod kernelu z Kernel_ProcessHemicube.h
	const char* source = kernel_processHemicube;
	
	// pridat makro na rozbaleni UINT_2_10_10_10_REV do indexu barvy (Colors::index)
	// pres parametr pri kompilaci to nejak nefunguje
	short* shifts = Colors::getShifts();
	unsigned int* rMasks = Colors::getRevMasks();
	ostringstream opts(ostringstream::out);
	opts << "#define unpack(c) ( (((c) & " << rMasks[0] << ") >> " << shifts[0] << ") | (((c) & " << rMasks[1] << ") >> " << shifts[1] << ") | (((c) & " << rMasks[2] << ") >> " << shifts[2] << ") )" << endl;	
	opts << "#define correction " << Colors::getCorrection() << endl;
	
	string src = opts.str() + string(source);
	const char* sourceStr = src.c_str();
	size_t sourceSize = src.size();


	// vytvorit program
	cl_program ocl_program = clCreateProgramWithSource(
				  ocl_context,
                  1,   // number of files
				  &sourceStr,   // array of strings, each one is a file
				  &sourceSize,   // array specifying the file lengths
                  &error);   // error code to be returned
	if (error != CL_SUCCESS) {
	   cerr << "Error creating program: " << errorMessage(error) << endl;
	   return false;
	}

	
	// zkompilovat program
	error = clBuildProgram(ocl_program, 1, &ocl_device, "", NULL, NULL);
	if (error != CL_SUCCESS) {
		cerr << "error: can't compile OpenCL program - ";
		switch (error) {
			case CL_INVALID_PROGRAM:
				cerr << "INVALID_PROGRAM";
				break;
			case CL_INVALID_VALUE:
				cerr << "INVALID_VALUE";
				break;
			case CL_INVALID_DEVICE:
				cerr << "INVALID_DEVICE";
				break;
			case CL_INVALID_BINARY:
				cerr << "INVALID_BINARY";
				break;
			case CL_INVALID_BUILD_OPTIONS:
				cerr << "INVALID_BUILD_OPTIONS";
				break;
			case CL_INVALID_OPERATION:
				cerr << "INVALID_OPERATION";
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				cerr << "COMPILER_NOT_AVAILABLE";
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				cerr << "BUILD_PROGRAM_FAILURE";
				break;
			case CL_OUT_OF_HOST_MEMORY:
				cerr << "OUT_OF_HOST_MEMORY";
				break;
		}
		cerr << endl;

		// compile log
		char* build_log;
		size_t log_size;

		// poprve pouze zjistime velikost logu
		clGetProgramBuildInfo(ocl_program, ocl_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		build_log = new char[log_size+1];
	
		// podruhe nacteme samotny vypis
		clGetProgramBuildInfo(ocl_program, ocl_device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
		build_log[log_size] = '\0';
		cout << build_log << endl;
		delete[] build_log;

		return false;
	}	
	

	// ziskat vstupni bod programu (kernel)
	ocl_kernel = clCreateKernel (ocl_program, "ProcessHemicube", &error);
	if (error != CL_SUCCESS) {
	   cerr << "error: can't extract ProcessHemicube kernel" << endl;
	   return false;
	}

	cl_sampler ocl_sampler = clCreateSampler(ocl_context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, &error);
	if(!ocl_sampler || error) {
		cerr << "error: unable to create OpenCL sampler" << endl;
        return false;
	}

		
	
	unsigned int spanLength = PATCHVIEW_TEX_W / Config::OCL_WORKITEMS_X();
	unsigned int hemicubesCnt = HEMICUBES_CNT;

	// nastavit argumenty kernelu	
	error  = clSetKernelArg (ocl_kernel, 0, sizeof(cl_mem), &ocl_arg_hemicubes);
	error |= clSetKernelArg (ocl_kernel, 1, sizeof(cl_mem), &ocl_arg_ids);
	error |= clSetKernelArg (ocl_kernel, 2, sizeof(cl_mem), &ocl_arg_energies);
	error |= clSetKernelArg (ocl_kernel, 3, sizeof(cl_mem), &ocl_arg_writeindex);
	error |= clSetKernelArg (ocl_kernel, 4, sizeof(cl_mem), &ocl_arg_patchview);
	error |= clSetKernelArg (ocl_kernel, 5, sizeof(cl_sampler), &ocl_sampler);
	error |= clSetKernelArg (ocl_kernel, 6, sizeof(cl_mem), &ocl_arg_ffactors);
	error |= clSetKernelArg (ocl_kernel, 7, sizeof(unsigned int), &PATCHVIEW_TEX_W);
	error |= clSetKernelArg (ocl_kernel, 8, sizeof(unsigned int), &PATCHVIEW_TEX_H);
	error |= clSetKernelArg (ocl_kernel, 9, sizeof(unsigned int), &spanLength);
	error |= clSetKernelArg (ocl_kernel, 10, sizeof(unsigned int), &hemicubesCnt);
	if (error != CL_SUCCESS) {
		cerr << "error: can't set up ProcessHemicube kernel arguments" << endl;
		return false;
	}

	// naplnit buffer argumentu - pole s formfactory - konstantni po celou dobu
	error = clEnqueueWriteBuffer(ocl_queue, ocl_arg_ffactors, CL_TRUE, 0, PATCHVIEW_TEX_RES * HEMICUBES_CNT * sizeof(float), p_formfactors,
 					0, NULL, NULL);
	if (error != CL_SUCCESS) {
		cerr << "error: can't write to ocl_arg_ffactors buffer" << endl;
		return false;
	}

	// pripravit pocty vlaken
	ocl_local_work_size = new unsigned int[2];
	ocl_local_work_size[0] = 1/*Config::OCL_WORKITEMS_X()*/;		ocl_local_work_size[1] = 256;
	ocl_global_work_size = new unsigned int[2];
	ocl_global_work_size[0] = Config::OCL_WORKITEMS_X();	ocl_global_work_size[1] = Config::OCL_WORKITEMS_Y();

	printf("local work size: (%d, %d)\n", ocl_local_work_size[0], ocl_local_work_size[1]);
	printf("global work size: (%d, %d)\n", ocl_global_work_size[0], ocl_global_work_size[1]);

	// round up - zaokrouhlit globalni pocty vlaken; pokud se vnuti rucne, je to casto vykonnejsi nez kdyz je odhadne samo OCL
	for(int i = 1; i < 2; ++ i) {
		ocl_global_work_size[i] += ocl_local_work_size[i] - 1;
		ocl_global_work_size[i] -= ocl_global_work_size[i] % ocl_local_work_size[i];
	}

	printf("global work size: (%d, %d) // after round-up\n", ocl_global_work_size[0], ocl_global_work_size[1]);

	printf("launched %d thread blocks\n", (ocl_global_work_size[0] / ocl_local_work_size[0]) *
		(ocl_global_work_size[1] / ocl_local_work_size[1]));
	
	// alokovat prostor, do ktereho se budou kopirovat vystupni data z kernelu; alokace v kreslici smycce by zdrzovala
	// PATCHVIEW_TEX_RES je maximalni pocet pro pripad, kdy by kazdy jeden pixel textury mel jinou barvu/patchId
	p_ocl_pids = new uint32_t[ PATCHVIEW_TEX_RES * HEMICUBES_CNT ];
	p_ocl_energies = new float[ PATCHVIEW_TEX_RES * HEMICUBES_CNT ];
	p_ocl_hemicubes = new uint32_t[ PATCHVIEW_TEX_RES * HEMICUBES_CNT ];

	cout << "OpenCL init OK" << endl;
	return true;
}

/**
 *	@brief uvolni vsechny OpenGL objekty
 */
void CleanupGLObjects()
{
	// smaze dynamicky alokovane objekty
	delete[] n_color_array_object;
	delete[] p_tmp_formfactors;
	delete[] p_formfactors;
	delete[] p_tmp_radiosities;

	for (unsigned int hi = 0; hi < Config::HEMICUBES_CNT(); hi++) {
		for (unsigned int i = 0; i < 5; i++) {
			delete[] p_viewport_list[hi][i];
			delete[] p_scissors_list[hi][i];
		}
		delete[] p_viewport_list[hi];
		delete[] p_scissors_list[hi];
	}
	delete[] p_viewport_list;
	delete[] p_scissors_list;
	delete[] p_emitters;
	delete[] p_emitters_ids;
	
	// smaze vertex buffer objekty
	glDeleteBuffers(1, &n_vertex_buffer_object);
	glDeleteBuffers(1, &n_index_buffer_object);

	// smaze shadery
	Shaders::cleanup();

	// smaze textury
	glDeleteTextures(1, &n_patchlook_texture);

	// smaze render buffer
	delete fbo;	
}

/**
 *  @brief uvolni vsechny OpenCL objekty
 */
void CleanupCLObjects() {
	clReleaseKernel(ocl_kernel);
	clReleaseCommandQueue(ocl_queue);
	clReleaseContext(ocl_context);
	clReleaseMemObject(ocl_arg_hemicubes);
	clReleaseMemObject(ocl_arg_ids);
	clReleaseMemObject(ocl_arg_energies);
	clReleaseMemObject(ocl_arg_writeindex);
	clReleaseMemObject(ocl_arg_patchview);
	clReleaseMemObject(ocl_arg_ffactors);

	delete[] p_ocl_hemicubes;
	delete[] p_ocl_pids;
	delete[] p_ocl_energies;
	delete[] ocl_local_work_size;
	delete[] ocl_global_work_size;
}

/**
 *	@brief vykresli uzivatelsky pohled do sceny (skutecne barvy patchu)
 */
void DrawScene() {		
	//glBindVertexArray(n_color_array_object[0]);
	glBindVertexArray(n_vertex_array_object);
	glDrawElements(GL_TRIANGLES, scene.getIndicesCount(), GL_UNSIGNED_INT, p_OffsetInVBO(0));
	
	// vratime VAO 0, abychom si nahodne VAO nezmenili (pripadne osetreni 
	//proti chybe v ovladacich nvidia kde se VAO poskodi pri volani nekterych wgl funkci)		
	glBindVertexArray(0); 		
}

/**
 *	@brief vykresli pohled do sceny z patche lookFromPatch s barvami odpovidajicimi ID patchu
 *  @param[in] interval interval ktery se bude kreslit
 */
void DrawPatchLook(unsigned int interval) {	

	int* indices = scene.getIndices();
	float* vertices = scene.getVertices();

	// kreslit cernou barvu mimo aktivni interval ve dvou krocich
	if (patchIntervals.size() > 1) {
		glBindVertexArray(n_vertex_array_object);
		glVertexAttrib3f(1, 0.0f, 0.0f, 0.0f); // vse cerne

		// vykreslit cerne vse pred aktivnim intervalem
		if (interval > 0) {
			int fromIndex = 0;
			int count = 6 * patchIntervals[interval - 1].to; // kreslit indexy od 0 az po posledni pred aktivnim intervalem
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, p_OffsetInVBO( fromIndex * sizeof(int) ));
		}
			
		// vykresli cerne vse za aktivnim intervalem
		if (interval < patchIntervals.size()-1) {
			int fromIndex = 6 * patchIntervals[interval + 1].from;
			int count = 6 * (patchIntervals.back().to - patchIntervals[interval + 1].from);
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, p_OffsetInVBO( fromIndex * sizeof(int) ));
		}						
	}
		
	// vykresli jeden interval s barevnymi patchi
	glBindVertexArray(n_color_array_object[interval]);	
				
	// spocitat pocet - vzdy 6 indexu; offset neni treba udavat, VAO uz obsahuje VBO na spravnem offsetu
	int count = 6 * (patchIntervals[interval].to - patchIntervals[interval].from);

	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, p_OffsetInVBO(0));	

	// vratime VAO 0, abychom si nahodne VAO nezmenili (pripadne osetreni 
	//proti chybe v ovladacich nvidia kde se VAO poskodi pri volani nekterych wgl funkci)		
	glBindVertexArray(0); 		
}

/**
 * Vykresli nahledovy kriz s pohledem 'patchove' kamery
 */
void DrawPatchLookPreview() {		
	Matrix4f t_mvp;
	t_mvp.Identity();
	t_mvp.Scale(1, float(n_width) / n_height, 1);
	t_mvp.Scale(0.25);
	t_mvp.Translate(-1.0, -1.0, 0);
	
	glUniformMatrix4fv(n_preview_mvp_matrix_uniform, 1, GL_FALSE, &t_mvp[0][0]);

	glBindVertexArray(n_vao_square);			
	glVertexAttrib3f(1, 0.0, 0.0, 0.0);			

	// vykreslit jednotlive casti s odpovidajicimi texturami (i odpovida Camera::PatchLook)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, n_patchlook_texture);		
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// zrusit bind textury
	glBindTexture(GL_TEXTURE_2D, 0);

	// vratime VAO 0, abychom si nahodne VAO nezmenili (pripadne osetreni 
	//proti chybe v ovladacich nvidia kde se VAO poskodi pri volani nekterych wgl funkci)		
	glBindVertexArray(0); 		
}

/**
 *	@brief main
 *	@param[in] n_arg_num je pocet argumentu prikazove radky
 *	@param[in] p_arg_list je seznam argumentu prikazove radky; tento program neni nejak ovladan prikazovou radkou a ignoruje ji
 *	@return vreci 0 pri uspesnem ukonceni programu, pripadne -1 pri chybe
 */
int main(int n_arg_num, const char **p_arg_list)
{
	if ((n_arg_num-1) % 2 > 0) {
		cerr << "Spatny pocet parametru!" << endl;
		return -1;
	}

	// parsovani parametru
	for (int i = 1; i < n_arg_num; i += 2) {
		if (strcmp(p_arg_list[i], "area") == 0) {
			Config::setMaxPatchArea( atof(p_arg_list[i+1]) );
		}
		if (strcmp(p_arg_list[i], "hemicube") == 0) {
			Config::setHemicubeSide( atoi(p_arg_list[i+1]) );
		}
		if (strcmp(p_arg_list[i], "shoots") == 0) {
			Config::setShootsPerCycle( atoi(p_arg_list[i+1]) );
		}
		if (strcmp(p_arg_list[i], "hemicubes") == 0) {
			Config::setHemicubesCount( atoi(p_arg_list[i+1]) );
		}
	}

	// parametry zname, muzeme zmrazit config a nechat jej dopocitat ostatni hodnoty
	Config::freeze();

	// registruje tridu okna
	WNDCLASSEX t_wnd_class;
	t_wnd_class.cbSize = sizeof(WNDCLASSEX);
	t_wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	t_wnd_class.lpfnWndProc = WndProc;
	t_wnd_class.cbClsExtra = 0;
	t_wnd_class.cbWndExtra = 0;
	t_wnd_class.hInstance = GetModuleHandle(NULL);
	t_wnd_class.hIcon = LoadIcon(0, IDI_APPLICATION);
	t_wnd_class.hCursor = LoadCursor(0, IDC_ARROW);
	t_wnd_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	t_wnd_class.lpszMenuName = 0;
	t_wnd_class.lpszClassName = p_s_class_name;
	t_wnd_class.hIconSm = LoadIcon(0, IDI_APPLICATION);
	RegisterClassEx(&t_wnd_class);	

	// spocita rozmery tak, aby vnitrek okna (client area) mel pozadovane rozmery
	RECT t_rect = {0, 0, n_width, n_height};
	AdjustWindowRectEx(&t_rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
	int n_wnd_width = t_rect.right - t_rect.left;
	int n_wnd_height = t_rect.bottom - t_rect.top;	

	// vyrobi a ukaze okno
	h_wnd = CreateWindow(p_s_class_name, p_s_window_name, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, n_wnd_width, n_wnd_height, NULL, NULL, GetModuleHandle(NULL), NULL);
	ShowWindow(h_wnd, SW_SHOW);
	UpdateWindow(h_wnd);	

	// inicializuje OpenGL
	bool b_forward_compatible = true; // jestli ma byt OpenGL kontext dopredne kompatibilni (tzn. nepodporovat funkce "stareho" OpenGL); pokud vase GPU nepodporuje OpenGL 3.0 nebo vyssi, pak vam to s timto nepobezi
	//CGL30Driver driver;
	if(!driver.Init(h_wnd, b_forward_compatible, 3, 0, n_width, n_height, 32, 24, 0, false)) {
		fprintf(stderr, "error: OpenGL initialization failed\n"); // nepodarilo se zinicializovat OpenGL
		return -1;
	}
	
	// vytiskne verzi OpenGL
	printf("OpenGL initialized: %s\n", (const char*)glGetString(GL_VERSION));

	{
		int n_extension_num;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n_extension_num);
		for(int i = 0; i < n_extension_num; ++ i) {
			const char *p_s_ext_name = (const char*)glGetStringi(GL_EXTENSIONS, i); // glGetString(GL_EXTENSIONS) uz v OpenGL 3 nefrci, protoze aplikace mely historicky problemy s prilis dlouhymi stringy. ted se extensions zjistuji po jedne.
			//printf((i)? ", %s" : "%s", p_s_ext_name);
		}
		//printf("\n");
	}
	if(!GL_VERSION_3_0) {
		fprintf(stderr, "error: OpenGL 3.0 not supported\n"); // OpenGL 3.0 neni podporovane
		return -1;
	}
	// zkontroluje zda jsou podporovane pozadovane rozsireni

	// nacteme globalni objekt sceny a nastavime limit velikosti patchu
	scene.load();	
	scene.maxPatchArea = Config::MAX_PATCH_AREA();

	{
		//lightPatches.clear();
		Patch **patches = scene.getPatches();
		for(unsigned int i = 0, n = scene.getPatchesCount(); i < n; ++ i) {
			if(patches[i]->radiosity.f_Length() > 1e-3f) {
				patches[i]->tag = 1;
				//lightPatches.push_back(patches[i]);
				lightPatchIDs.push_back(i);
				if(lightPatchID_min > i)
					lightPatchID_min = i;
				if(lightPatchID_max < i)
					lightPatchID_max = i;
			} else
				patches[i]->tag = 0;
			initialEnergies.push_back(patches[i]->radiosity);
		}
		if(lightPatchID_max - lightPatchID_min + 1 != lightPatchIDs.size())
			fprintf(stderr, "light patch id's not contiguous!\n");

		const float *pv = scene.getVertices();
		const Vector3f *pvv = (const Vector3f*)pv;
		// get scene verts

		lightPatchCoords.insert(lightPatchCoords.begin(), pvv + (4 * lightPatchID_min),
			pvv + (4 * (lightPatchID_max + 1)));
		lightPatchCoordsCopy.resize(lightPatchCoords.size());
		if(lightPatchCoords.size() != 4 * lightPatchIDs.size())
			fprintf(stderr, "light patch coords garbled!\n");

		printf("have %d light patch coords to update each frame\n", lightPatchCoords.size());
	}
	// hack (collect light patches)
	
	// vyrobime objekty OpenGL, nutne ke kresleni
	if(!InitGLObjects()) {
		cerr << "error: failed to initialize OpenGL objects" << endl; // neco se nepovedlo (chyba pri kompilaci shaderu / chyba pri nacitani obrazku textury)
		return -1;
	}		

	// vyrobime objekty OpenCL
	if(!InitCLObjects()) {
		cerr << "error: failed to initialize OpenCL objects" << endl;
		return -1;
	}	

	// skryt kurzor mysi
	ShowCursor(false);
	
	// smycka zprav, s funkci OnIdle()
	MSG t_msg;
	while(b_running) {
		if(PeekMessage(&t_msg, h_wnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&t_msg);
			DispatchMessage(&t_msg);
		} else {
			OnIdle(driver);
#ifdef _DEBUG
			int err = glGetError();
			if(err != GL_NO_ERROR) {
				fprintf(stderr, "error: OpenGL error(s) occured\n");
				switch (err) {
					case GL_INVALID_ENUM:
						fprintf(stderr, "error: GL_INVALID_ENUM\n");
						break;
					case GL_INVALID_VALUE:
						fprintf(stderr, "error: GL_INVALID_VALUE\n");
						break;
					case GL_INVALID_OPERATION:
						fprintf(stderr, "error: GL_INVALID_OPERATION\n");
						break;
					case GL_STACK_OVERFLOW:
						fprintf(stderr, "error: GL_STACK_OVERFLOW\n");
						break;
					case GL_STACK_UNDERFLOW:
						fprintf(stderr, "error: GL_STACK_UNDERFLOW\n");
						break;
					case GL_OUT_OF_MEMORY:
						fprintf(stderr, "error: GL_OUT_OF_MEMORY\n");
						break;
					case GL_TABLE_TOO_LARGE:
						fprintf(stderr, "error: GL_TABLE_TOO_LARGE\n");
						break;
				}
			}
#endif //_DEBUG
		}
	}
	
	
	// uvolnime OpenGL objekty
	CleanupGLObjects();	

	// uvolnime OpenCL objekty
	CleanupCLObjects();
	
	// znovu zobrazit kurzor mysi
	ShowCursor(true);

	return 0;
}

/**
 *	@brief obsluha smycky zprav
 *	@param[in] h_wnd je okno pro ktere je zprava urcena
 *	@param[in] n_msg je cislo zpravy (WM_*)
 *	@param[in] n_w_param je prvni parametr zpravy
 *	@param[in] n_l_param je druhy parametr zpravy
 *	@return vraci 0, pokud zprava byla zpracovana aplikaci, pripadne vraci vysledek DefWindowProc()
 */
LRESULT CALLBACK WndProc(HWND h_wnd, UINT n_msg, WPARAM n_w_param, LPARAM n_l_param)
{
	switch(n_msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			b_running = false;
			return 0;

		case WM_SIZE:
			{
				RECT t_wnd_area;
				GetClientRect(h_wnd, &t_wnd_area);
				int n_ww = t_wnd_area.right - t_wnd_area.left;
				int n_wh = t_wnd_area.bottom - t_wnd_area.top;
				onResize(0, 0, n_ww, n_wh);
			}
			return 0;

		case WM_KEYDOWN:			
				if (n_w_param == VK_ESCAPE || n_w_param == KEY_Q)
					SendMessage(h_wnd, WM_CLOSE, 0, 0);
				
				// Ctrl + S
				if (n_w_param == KEY_S && (GetKeyState(VK_CONTROL) & 0xF0)) {
					SaveToFile();
				}

				// Ctrl + O
				if (n_w_param == KEY_O && (GetKeyState(VK_CONTROL) & 0xF0)) {
					LoadFromFile();
				}

				// F - wireframe
				if (n_w_param == KEY_F) {
					if (wireframe) {
						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
						wireframe = false;
					} else {
						glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
						wireframe = true;
					}
				}

				// TAB - prepinani mezi vykreslovanim illuminativni a radiativni energie
				if (n_w_param == KEY_TAB) {
					b_draw_radiative = !b_draw_radiative;
					if (b_draw_radiative) {
						// zmenit VBO v userview VAO na radiativni energie
						glBindVertexArray(n_vertex_array_object);														
						glBindBuffer(GL_ARRAY_BUFFER, n_patch_radiative_buffer_object);
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, p_OffsetInVBO(0));
						glBindVertexArray(0); 
						cout << "Switched to radiative energies" << endl;
					} else {
						// zmenit VBO v userview VAO na iluminativni energie
						glBindVertexArray(n_vertex_array_object);														
						glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, p_OffsetInVBO(0));
						glBindVertexArray(0); 
						cout << "Switched to illuminative energies" << endl;
					}					
				}

				// O - vystup informaci pri vypoctu radiosity
				if (n_w_param == KEY_O)
					debugOutput = !debugOutput;

				// P - zobrazit/skryt nahledove okynko pohledu z patche
				if (n_w_param == KEY_P) 
					showPatchLook = !showPatchLook;

				// L - spustit/pozastavit sireni energie
				if (n_w_param == KEY_L) {
					computeRadiosity = !computeRadiosity;
					if (computeRadiosity) {
						cout << "Light emitting continues" << endl;
						totalTimer.ResetTimer(); // spustit stopky celkove doby vypoctu
					} else {
						cout << "Light emitting paused" << endl;
					}
				}

				return 0;


		case WM_MOUSEMOVE:	
			// ignorovat eventy pokud je zdrojem posunu SetCursorPos
			if (!b_mouse_controlled) {
				GLint viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);	// x, y, w, h  okna

				// stred okna
				POINT center;
				center.x = viewport[2] / 2;
				center.y = viewport[3] / 2;				

				// odchyleni od stredu okna
				float dx = (float)LOWORD(n_l_param) - center.x;
				float dy = (float)HIWORD(n_l_param) - center.y;
								
				if(dx || dy) {
					dx *= mouse_sensitivity;
					dy *= mouse_sensitivity;
				
					cam.Aim(-dy, dx); // invertovat osu Y pro spravne ovladani mysi
					
					b_mouse_controlled = true; // pristi event chceme ignorovat
					ClientToScreen(h_wnd, &center); // chceme nastavit pozici relativne k oknu
					SetCursorPos(center.x, center.y);
				}
			} else {
				b_mouse_controlled = false;
			}			
			return 0;
	}

	return DefWindowProc(h_wnd, n_msg, n_w_param, n_l_param) ;
}

/**
 *	@brief notifikace zmeny velikosti okna
 *	@param[in] n_x je x-souradnice leveho horniho rohu (vzdy 0)
 *	@param[in] n_y je y-souradnice leveho horniho rohu (vzdy 0)
 *	@param[in] n_new_width je nova sirka okna (klientska cast)
 *	@param[in] n_new_height je nova vyska okna (klientska cast)
 */
void onResize(int n_x, int n_y, int n_new_width, int n_new_height)
{
	n_width = n_new_width;
	n_height = n_new_height;
	glViewport(n_x, n_y, n_new_width, n_new_height);
}


/**
 * Vlastni handler stisknutych klaves; volany v onIdle
 */
void handleActiveKeys() {
	
	// pohyb kamery pomoci W A S D
	bool a_down = HIBYTE(GetKeyState(KEY_A)) & 0x01;
	bool s_down = HIBYTE(GetKeyState(KEY_S)) & 0x01;
	bool d_down = HIBYTE(GetKeyState(KEY_D)) & 0x01;
	bool w_down = HIBYTE(GetKeyState(KEY_W)) & 0x01;

	// chceme aby byla rychlost pohybu nezavisla na fps
	float f_fps = float(1 / f_frame_time_average);
	float f_step = float(walk_speed / f_fps);

	// vysledkem jsou slozky vektoru ve smerech X ("strafe", ne otaceni) a Z
	float x = -( (-1.0f * a_down) + (1.0f * d_down) ) * f_step;	
	float z = ( (-1.0f * s_down) + (1.0f * w_down) ) * f_step;		
	
	cam.Move(x, 0.0f, z);

	// R - reset kamery
	if (HIBYTE(GetKeyState(0x52)) & 0x01)
		cam.Reset();	
}

#define MAX_MARKS 100
//#define MARK(n) do { if(n_marker_used < MAX_MARKS) { p_marker_name_list[n_marker_used] = n; p_marker[n_marker_used] = timer.f_Time(); ++ n_marker_used; } } while(false)
#define MARK(n) do {} while(false)

/**
 *	@brief tato funkce se vola ve smycce pokazde kdyz nejsou zadne nove zpravy; lze vyuzit ke kresleni animace
 *	@param[in] driver je reference na OpenGL driver
 */
void OnIdle(CGL30Driver &driver)
{
	// spustit stopky fps
	double t_start = timer.f_Time();

	size_t n_marker_used = 0;
	const char *p_marker_name_list[MAX_MARKS] = {0};
	double p_marker[MAX_MARKS] = {0};

	if(computeRadiosity)
		MARK("reference");

	// vycistime framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	// cas pro animaci
	float f_time = clock() / 1000.0f;
		
	// spustit vsechny akce spojene se stiskem klaves
	// napriklad posun kamery
	handleActiveKeys();
	
	Matrix4f t_mvp;

	Patch** scenePatches = scene.getPatches();
	unsigned int scenePatchesCount = scene.getPatchesCount();

	// ***********************************************************************************
	// Vykreslit do FBO pohled z patche
	// ***********************************************************************************

	// v pripade, ze scena neni nactena, nesnazit se renderovat do textury
	// ani pocitat radiozitu
	if (computeRadiosity && scenePatchesCount > 0) {
		{
			Patch **patches = scene.getPatches();
			for(unsigned int i = 0, n = scene.getPatchesCount(); i < n; ++ i) {
				patches[i]->radiosity = initialEnergies[i];
				patches[i]->illumination = Vector3f(0, 0, 0);
			}
		}
		// clear the energies

		float f_light_movement_speed = 0.7f;
		float f_light_movement_amp = 1.25f;
		Vector3f v_light_offset(Vector3f(float(sin(t_start * f_light_movement_speed)), 0,
			float(cos(t_start * f_light_movement_speed))) * f_light_movement_amp);

		for(size_t i = 0, n = lightPatchCoords.size(); i < n; ++ i)
			lightPatchCoordsCopy[i] = lightPatchCoords[i] + v_light_offset;
		// transform new light position

		glBindBuffer(GL_ARRAY_BUFFER, n_vertex_buffer_object);
		glBufferSubData(GL_ARRAY_BUFFER, lightPatchID_min * sizeof(float) * 3 * 4,
			lightPatchCoordsCopy.size() * sizeof(float) * 3, (const float*)&lightPatchCoordsCopy[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// update light patch

		MARK("starting up");

		// pouzije shader pro pohled z patche a bude kreslit do framebuffer objectu
		glUseProgram(n_patch_program_object);
		fbo->Bind();
		fbo->Bind_ColorTexture2D(0, GL_TEXTURE_2D, n_patchlook_texture);
		glViewport(0, 0, fbo->n_Width(), fbo->n_Height());
		//glFinish(); // remove me		

		MARK("FBO bound");

		for (unsigned int shoot = 0; /*shoot < Config::SHOOTS_PER_CYCLE() &&*/ computeRadiosity; shoot++) { 

			// najit patche s nejvetsi energii
			scene.getHighestRadiosityPatchesId(Config::HEMICUBES_CNT(), p_emitters, p_emitters_ids);

			MARK("getHighestRadiosityPatchesId");

			// pro kazdy interval patchu ve scene
			for (unsigned int interval = 0; interval < patchIntervals.size(); interval++) {
				
				// vycistit fbo
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 			
				glEnable(GL_SCISSOR_TEST);
//glFinish(); // remove me

				MARK("glClear");

				// pro kazdou hemicube
				for (unsigned int hi = 0; hi < Config::HEMICUBES_CNT(); hi++) {
					// pokud uz neni patch s energii, preskocit - vykresli se cerno
					if (p_emitters[hi] == NULL)
						continue;

					// poznacit si puvodni hodnotu radiosity, ta se po uplnem vyzareni patche odecte
					p_tmp_radiosities[hi] = p_emitters[hi]->radiosity;

					// celkem 5 pohledu
					for(int i=0; i < 5; i++) {
				
						Camera::PatchLook dir = p_patchlook_perm[i];

						// spocitame modelview - projection matici, kterou potrebujeme k transformaci vrcholu		
						{
							// matice perspektivni projekce
							Matrix4f t_projection;
							CGLTransform::Perspective(t_projection, 90, 1.0f, 0.01f, 1000);		 // ratio 1.0!
		
							// modelview
							Matrix4f t_modelview;
							t_modelview.Identity();	

							if(p_emitters[hi]->tag == 1) {
								// vynasobit pohledem kamery patche
								patchCam.lookFromPatch(p_emitters[hi], dir, v_light_offset);
							} else {
								// vynasobit pohledem kamery patche
								patchCam.lookFromPatch(p_emitters[hi], dir);
							}
							t_modelview *= patchCam.GetMatrix();

							// matice pohledu kamery
							t_mvp = t_projection * t_modelview;
						}

						// nahrajeme matici do OpenGL jako parametr shaderu
						glUniformMatrix4fv(n_patchprogram_mvp_matrix_uniform, 1, GL_FALSE, &t_mvp[0][0]);		

						// nastavit parametry viewportu a oblast, do ktere je povoleno kreslit
						glScissor(p_scissors_list[hi][i][0], p_scissors_list[hi][i][1], p_scissors_list[hi][i][2], p_scissors_list[hi][i][3]);
						glViewport(p_viewport_list[hi][i][0], p_viewport_list[hi][i][1], p_viewport_list[hi][i][2], p_viewport_list[hi][i][3]);
		
						// vykreslit do textury (pres FBO)
						DrawPatchLook(interval);

					} // pro kazdy pohled

				} // pro kazdou hemicube

				glDisable(GL_SCISSOR_TEST);

//glFinish(); // remove me
				MARK("hemicubes finished");
					
				//FBO2BMP();
				
				// priznak chyby pri praci s OCL
				cl_int error = 0;			

				// ziskat pristup k OGL texture s pohledem z patche		
				//glFinish(); // nutne pro sync
				error |= clEnqueueAcquireGLObjects(ocl_queue, 1, &ocl_arg_patchview, 0, NULL, NULL);
				
				//clFinish(ocl_queue); // remove me
				MARK("clEnqueueAcquireGLObjects");

				// vynulovat index na ktery se zapisuje - nutne v kazde iteraci!
				{
					unsigned int writeindex = 0;
					error |= clEnqueueWriteBuffer(ocl_queue, ocl_arg_writeindex, CL_FALSE, 0, sizeof(unsigned int), &writeindex,	0, NULL, NULL);
				}
				_ASSERT(error == CL_SUCCESS);

				// spustit program!
				error = clEnqueueNDRangeKernel(ocl_queue, ocl_kernel, 2, NULL, ocl_global_work_size, ocl_local_work_size, 0, NULL, NULL);
				_ASSERT(error == CL_SUCCESS);

				//clFinish(ocl_queue); // remove me
				MARK("clEnqueueNDRangeKernel");

				// zjistit kolik polygonu*instanci se ulozilo (pocet je vzdy ruzny v zavislosti na pohledu a rozlozeni work-items)
				unsigned int n_last_index = 0;
				error = clEnqueueReadBuffer (ocl_queue, ocl_arg_writeindex, CL_TRUE, 0, sizeof(unsigned int), &n_last_index, 0, NULL, NULL);
				_ASSERT(error == CL_SUCCESS);

				// precist data
				error  = clEnqueueReadBuffer (ocl_queue, ocl_arg_hemicubes, CL_TRUE, 0, n_last_index*sizeof(uint32_t), p_ocl_hemicubes, 0, NULL, NULL);
				error  = clEnqueueReadBuffer (ocl_queue, ocl_arg_ids, CL_TRUE, 0, n_last_index*sizeof(uint32_t), p_ocl_pids, 0, NULL, NULL);
				error |= clEnqueueReadBuffer (ocl_queue, ocl_arg_energies, CL_TRUE, 0, n_last_index*sizeof(float), p_ocl_energies, 0, NULL, NULL);		
				_ASSERT(error == CL_SUCCESS);
				
				MARK("data readback");

				// uvolnit OGL objekty z drzeni OCL
				//clFinish(ocl_queue); // nutne pro sync
				error |= clEnqueueReleaseGLObjects(ocl_queue, 1, &ocl_arg_patchview, 0, NULL, NULL);
				_ASSERT(error == CL_SUCCESS);				

				MARK("clEnqueueReleaseGLObjects");
				
				for (unsigned int hi = 0; hi < Config::HEMICUBES_CNT(); hi++) {
					// jenom pokud se skutecne z patche koukalo
					if (p_emitters[hi] == NULL)
						continue;

					// secist formfactory do p_tmp_formfactors
					for (unsigned int i = 0; i < n_last_index; i++) {			

						if (p_ocl_pids[i] >= scenePatchesCount) {
							cerr << "Uknown patch id: " << p_ocl_pids[i] << "! Is there a problem with video card?" << endl;
							continue;
						}

						if (p_ocl_hemicubes[i] != hi)
							continue;

						// jeste nesirit, nejdriv jen sesbirat
						p_tmp_formfactors[p_ocl_pids[i]] += p_ocl_energies[i];
					}					

					// prenest energie
					for (unsigned int i = 0; i < scenePatchesCount; i++) {
						Patch* p = scenePatches[i];
						p->radiosity += p_tmp_radiosities[hi] * p_tmp_formfactors[i] * p->getReflectivity() * p_emitters[hi]->getColor();
					}
				
					// vyprazdnit pole pro dalsi pruchod
					fill_n(p_tmp_formfactors, scenePatchesCount, (float)0);
				}

				MARK("energies update");
				
			} // for each interval


			// zdroje se vyzarily
			Vector3f lastEnergy; // posledni vyzarena energie
			for (unsigned int hi = 0; hi < Config::HEMICUBES_CNT(); hi++) {
				if (p_emitters[hi] == NULL)
					continue;

				lastEnergy = p_emitters[hi]->radiosity;
				p_emitters[hi]->illumination += p_tmp_radiosities[hi];
				p_emitters[hi]->radiosity -= p_tmp_radiosities[hi];
			}

			// ukoncit, jakmile energie nejnabitejsiho patche ve scene klesne pod danou hranici
			if (lastEnergy.f_Length() < 0.1) {
				cout << "Done in " << (timer.f_Time() - t_start) << " seconds, " << (shoot * Config::HEMICUBES_CNT()) << " cycles" << endl;				
				computeRadiosity = false; 
			} else if (debugOutput) {
				cout << "Pass " << passCounter << ", the emitter had " << setprecision(10) << lastEnergy.f_Length2() << " energy" << endl;
			}	

			MARK("emitters update");

			passCounter++;			

		} // for 'shoot' times

		if(!computeRadiosity)
			computeRadiosity = true;
		// hack - compute radiosity in the next frame as well

		// uvolnit fbo
		fbo->Bind_ColorTexture2D(0, GL_TEXTURE_2D, 0);
		fbo->Release();
		
		//glFinish(); // remove me
		MARK("FBO release");
				
		// nabindovat buffer s barvami patchu a updatovat jej
		{
			glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
			float* buffer = (float*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); // mapovani obou VBO prida cca 5 FPS

			for (unsigned int i = 0; i < scenePatchesCount; i++) {
				Patch* p = scenePatches[i];

				float newData[4 * 3]; // zde budou nove, vyhlazene barvy (4 vrcholy * 3 slozky)
				Colors::smoothShadePatch(newData, p);

				if (buffer != NULL)
					memcpy(buffer + i * 4 * 3, newData, 4 * 3 * sizeof(float));
				else
					glBufferSubData(GL_ARRAY_BUFFER, i * 4 * 3 * sizeof(float), 4 * 3 * sizeof(float), newData);
			}
		
			if (buffer != NULL) {
				if (glUnmapBuffer(GL_ARRAY_BUFFER) == GL_FALSE)
					cerr << "Error unmapping VBO" << endl;
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0); // odbindovat buffer
		}
		
		//glFinish(); // remove me
		MARK("VBO update");
		
// pro zobrazovani radiativnich energii
#define SHOW_RADIATIVE
#ifdef SHOW_RADIATIVE
		// nabindovat VBO s radiativnimi energiemi a updatovat jej
		{
			glBindBuffer(GL_ARRAY_BUFFER, n_patch_radiative_buffer_object);
			float* buffer = (float*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);		

			for (unsigned int i = 0; i < scenePatchesCount; i++) {				
				float newData[4 * 3]; 
				for (unsigned int n = 0; n < 12; n += 3) {
					newData[n] = scenePatches[i]->radiosity.x;
					newData[n+1] = scenePatches[i]->radiosity.y;
					newData[n+2] = scenePatches[i]->radiosity.z;
				}

				if (buffer != NULL)
					memcpy(buffer + i*4*3, newData, 4*3*sizeof(float));
				else
					glBufferSubData(GL_ARRAY_BUFFER, i * 4 * 3 * sizeof(float), 4 * 3 * sizeof(float), newData);
			}

			if (buffer != NULL) {
				if (glUnmapBuffer(GL_ARRAY_BUFFER) == GL_FALSE)
					cerr << "Error unmapping VBO" << endl;
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, 0); // odbindovat buffer		
		}
#endif		

	} // if scenePatchesCount > 0

	// ***********************************************************************************
	// Vykreslit na obrazovku uzivatelsky pohled + nahledovy kriz
	// ***********************************************************************************

	// obonvit vychozi rozmer viewportu
	glViewport(0, 0, n_width, n_height);

	// nove nastavit modelview matici, tentokrat pro uzivatelsky (volny) pohled
	{
		// matice perspektivni projekce
		Matrix4f t_projection;
		CGLTransform::Perspective(t_projection, 90, float(n_width) / n_height, .01f, 1000);		
		
		// modelview
		Matrix4f t_modelview;
		t_modelview.Identity();				

		// vynasobit pohledem uzivatelske kamery
		//cam.lookFromPatch(scene.getPatch(lookFromPatch), lookFromPatchDir);
		t_modelview *= cam.GetMatrix();

		// matice pohledu kamery
		t_mvp = t_projection * t_modelview;
	}

	
	// pouzije shader pro uzivatelsky pohled
	glUseProgram(n_user_program_object);
	//glUseProgram(n_patch_program_object);
	
	// nastavime parametry shaderu (musi se delat pokazde kdyz se shader pouzije)
	// uniformni parametry se mohou prubezne menit
	{
		// nahrajeme matici do OpenGL jako parametr shaderu
		glUniformMatrix4fv(n_user_mvp_matrix_uniform, 1, GL_FALSE, &t_mvp[0][0]);
				
	}

	// vykresli scenu
	DrawScene(); 

	if (showPatchLook) {
		// pouzije shader pro nahledovy kriz
		glUseProgram(n_preview_program_object);

		{
			// nastavime cisla texturovacich jednotek, odkud si shader bude brat textury
			glUniform1i(n_box_sampler_uniform, 0);
		}

		// vykresli nahledovy kriz; obsahuje vlastni modelview matici pro fixni pozici na obrazovce
		DrawPatchLookPreview(); 
	}



	// ***********************************************************************************

	// preklopi buffery, zobrazi co jsme nakreslili
	driver.Blit(); 

	if(n_marker_used) {
	for(size_t i = 0; i < n_marker_used; ++ i)
		printf("mark: \'%s\': %f (%f total)\n", p_marker_name_list[i], p_marker[i] - ((i)? p_marker[i - 1] : t_start), p_marker[i]);
	}

	// spocitat fps
	glFinish();
	f_frame_time_average = f_frame_time_average * .9 + (timer.f_Time() - t_start) * .1;
	double f_fps = 1 / f_frame_time_average;
	ostringstream winTitle;
	winTitle << p_s_window_name << ", FPS: " << f_fps;
	SetWindowText(h_wnd, winTitle.str().c_str());
}




/**
 * Otevre dialog na vyber souboru odkud se nactou obsahy bufferu
 */
void LoadFromFile() {
	
	OPENFILENAME ofn;	// struktura dialogu
	char szFile[260];	// buffer pro cestu k souboru		

	// naplnit strukturu parametry dialogu
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = h_wnd;	// rodicovske okno
	ofn.lpstrFile = szFile; // cesta
	ofn.lpstrFile[0] = '\0'; // vynulovat cestu, nechceme zadnou predvyplnenou
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Radiosity Renderer File\0*.rr\0\0"; // pouze soubory *.rr
	ofn.nFilterIndex = 1; // vychozi filtr
	ofn.lpstrFileTitle = NULL; // zadny soubor neni vybrany
	ofn.lpstrInitialDir = NULL; // zadny vychozi adresar
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// povolit kurzor
	ShowCursor(true);

	if (GetOpenFileName(&ofn) == TRUE) {
		cout << "Loading from " << ofn.lpstrFile << endl;

		FILE* fp = NULL;
		errno_t err = fopen_s(&fp, ofn.lpstrFile, "rb");
		if (err != 0 || fp == NULL) {
			cerr << "Unable to open the file" << endl;
		} else {
			bool error = false;
			size_t read;

			// nacist patche ------------------------------			
			unsigned long count;
			read = fread(&count, sizeof(unsigned long), 1, fp);
			if (read != 1)
				error = true;

			Patch* data = new Patch[count];
			read = fread(data, sizeof(Patch), count, fp);
			if (read != count)
				error = true;
			

			// rekonstruovat scenu ------------------------
			if (!error) {				
				CleanupGLObjects();
				CleanupCLObjects();

				// zresetovat staticke objekty
				patchIntervals.clear();

				// vytvorit novou scenu
				scene = ModelContainer::ModelContainer();
				scene.maxPatchArea = Config::MAX_PATCH_AREA();

				// vytvorit z nactenych patchu model, ktery vlozime do sceny
				Model* dummy = new LoadingModel(data, count);
				scene.addModel(dummy);

				// znovu inicializovat GL, naplnit buffery, ...
				if (!InitGLObjects() || !InitCLObjects())
					error = true;

				delete[] data;
			}
			/*
			// provest vyhlazovani, at je obraz pekny ikdyz se rad. jeste nepocita
			{
				Patch** scenePatches = scene.getPatches();
				glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
				for (unsigned int i = 0; i < count; i++) {
					Patch* p = scenePatches[i];
			
					uint32_t newData[4];
					Colors::smoothShadePatch(newData, p);

					glBufferSubData(GL_ARRAY_BUFFER, i * 4 * sizeof(uint32_t), 4 * sizeof(uint32_t), newData);
				}		
				glBindBuffer(GL_ARRAY_BUFFER, 0); // odbindovat buffer
			}
			*/
			// pokud probihal vypocet, zastavit jej
			computeRadiosity = false;

			if (!error)
				cout << "Done!" << endl;
			else
				cerr << "An error occured!" << endl;

			fclose(fp);
		}
	}
	
	ShowCursor(false);
}



/**
 * Otevre dialog na vyber souboru, kam se ulozi patche ve scene (z nich je mozne zrekonstruovat jiz osvetlenou scenu)
 */
void SaveToFile() {
	
	OPENFILENAME ofn;	// struktura dialogu
	char szFile[260];	// buffer pro cestu k souboru		

	// naplnit strukturu parametry dialogu
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = h_wnd;	// rodicovske okno
	ofn.lpstrFile = szFile; // cesta
	ofn.lpstrFile[0] = '\0'; // vynulovat cestu, nechceme zadnou predvyplnenou
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Radiosity Renderer File\0*.rr\0\0"; // pouze soubory *.rr
	ofn.lpstrDefExt = "rr\0"; // vychozi pripona
	ofn.nFilterIndex = 1; // vychozi filtr
	ofn.lpstrFileTitle = NULL; // zadny soubor neni vybrany
	ofn.lpstrInitialDir = NULL; // zadny vychozi adresar
	ofn.Flags = OFN_OVERWRITEPROMPT; // uzivatel musi potvrdit pripadne prepsani

	// povolit kurzor
	ShowCursor(true);

	if (GetSaveFileName(&ofn)==TRUE) {
		cout << "Saving to " << szFile << endl;
		
		FILE* fp = NULL;
		errno_t err = fopen_s(&fp, ofn.lpstrFile, "wb");
		if (err != 0 || fp == NULL) {
			cerr << "Unable to create the file" << endl;
		} else {

			bool error = false;
			size_t written;

			// patche --------------------------------------------
			{
				unsigned long count = scene.getPatchesCount();
				Patch** patches = scene.getPatches();
				
				// prevest pole ukazatelu na pole patchu
				Patch* data = new Patch[count];
				for (unsigned long i = 0; i < count; i++) {
					data[i] = (*patches[i]);
				}

				// v patchich jsou ukazatele na sousedy - prevedeme je na relativni (cisla) ukazatele v ramci sceny
				for (unsigned long i = 0; i < count; i++) {
					
					for (unsigned int n = 0; n < 8; n++) { // pro kazdeho souseda
						
						for (unsigned long j = 0; j < count; j++) {
							if (patches[i]->neighbours[n] == patches[j]) {
								data[i].relativeNeighbours[n] = j;
								data[i].neighbours[n] = NULL;
								break;
							}
						}

					}

				}

				// prvni ulozena hodnota je pocet patchu
				written = fwrite(&count, sizeof(unsigned long), 1, fp);
				if (written != 1)
					error = true;

				// nasleduji data
				written = fwrite(data, sizeof(Patch), count, fp);
				if (written != count)
					error = true;
			}
			
			
			if (!error)
				cout << "Done!" << endl;
			else
				cerr << "An error occured!" << endl;

			fclose(fp);			
		}
		
	}
	
	ShowCursor(false);
}
