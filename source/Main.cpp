#include <GL/glew.h>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <sstream>
#include "OpenGL30Drv.h"
#include "opencl.h"
#include "Transform.h"
#include "Tga.h"
#include "Camera.h"
#include "ModelContainer.h"
#include "Colors.h"
#include "FrameBuffer.h"
#include "Shaders.h"
#include "Timer.h"
#include "FormFactors.h"


using namespace std;

// parametr pro subdivision
#define MAX_PATCH_AREA 1

const unsigned int OCL_WORKITEMS_X = 4;
const unsigned int OCL_WORKITEMS_Y = PATCHVIEW_TEX_H;
const unsigned int OCL_SPANLENGTH = PATCHVIEW_TEX_W / OCL_WORKITEMS_X;

// rozmery hemicube, resp. pohledove textury jsou v FormFactors.h
// const unsigned int HEMICUBE_W = 256
// const unsigned int HEMICUBE_H = 256
// const unsigned int PATCHVIEW_TEX_W = HEMICUBE_W*2
// const unsigned int PATCHVIEW_TEX_H = HEMICUBE_H*1.5
// const unsigned int PATCHVIEW_TEX_RES = PATCHVIEW_TEX_W * PATCHVIEW_TEX_H 

static const char *p_s_window_name = "Radiosity renderer";
static const char *p_s_class_name = "my_wndclass";
// jmeno okna

static int n_width = 800;
static int n_height = 600;
// velikost okna

static bool b_running = true;
// flag pro vyskoceni ze smycky zprav

HWND h_wnd;
// okno

CGL30Driver driver;
// OpenGL driver

void OnIdle(CGL30Driver &driver);
void onResize(int n_x, int n_y, int n_new_width, int n_new_height);
LRESULT CALLBACK WndProc(HWND h_wnd, UINT n_msg, WPARAM n_w_param, LPARAM n_l_param);
// prototypy funkci

static GLuint n_patchlook_texture;

// vbos, vaos
static GLuint n_vertex_buffer_object, n_index_buffer_object, n_vertex_array_object, n_tex_buffer_object,
	n_vbo_square, n_vao_square;

static GLuint n_user_program_object, n_user_mvp_matrix_uniform, n_box_sampler_uniform;

static GLuint n_patch_program_object, n_patchprogram_mvp_matrix_uniform;

static GLuint n_preview_program_object, n_preview_mvp_matrix_uniform;

// VBO pro barvy odpovidajici IDckam patchu
static GLuint n_id_color_buffer_object; 

// VBO pro vykreslovani iluminativni (barva) a radiativni energie patchu
static GLuint n_patch_color_buffer_object, n_patch_radiative_buffer_object;

// pole VAO pro kazdy interval vykreslovanych patchu
static GLuint* n_color_array_object = NULL; 

// pole form factoru o velikosti odpovidajici rozliseni textury hemicube
static float* p_formfactors = NULL;

// CL objekty
//static cl_int ocl_error = 0;
static cl_platform_id ocl_platform;
static cl_kernel ocl_kernel;
static cl_context ocl_context;
static cl_command_queue ocl_queue;
static cl_device_id ocl_device;
static uint32_t* ocl_data_patchids;
static float* ocl_data_formfactors;

// CL argumenty programu
cl_mem ocl_arg_ids, ocl_arg_energies, ocl_arg_writeindex, ocl_arg_patchview, ocl_arg_ffactors;


// citlivosti / rychlosti pohybu
static float mouse_sensitivity = 0.001f;
static float walk_speed = 2.5f;

// je mys ovladana programem? (v pripade nastavovani pozice mysi nechceme odchytavat eventy)
static bool b_mouse_controlled = false;

// aliasy pro virtual-keys
#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_L 0x4C
#define KEY_P 0x50
#define KEY_S 0x53
#define KEY_W 0x57


// objekty kamery
Camera cam; // uzivatelsky pohled
Camera patchCam; // pevne nastavovany pohled z plosky

// objekt sceny
ModelContainer scene;

// docasne - pro ruzne testovani
int step = 0;
unsigned long lookFromPatch = 0;
Camera::PatchLook lookFromPatchDir = Camera::PATCH_LOOK_FRONT;
CGLFrameBufferObject* fbo = NULL;
unsigned int passCounter = 0;
bool debugOutput = false;

// kreslit radiativni energie namisto iluminativnich? (TAB)
bool b_draw_radiative = false;

// spustit/pozastavit vypocet (L)
bool computeRadiosity = false;

// zobrazit pouze wireframe? (F)
bool wireframe = false;

// zobrazit nahledovy kriz? (P)
bool showPatchLook = false;

// polozka intervalu patchu
typedef struct { 
	int from, to; 
} interval;

// intervaly patchu ve VBO po kterych se vykresluje - jen poradova cisla patchu
// inicializuji se v InitGlObjects
static vector<interval> patchIntervals; 

// pro mereni fps
static CTimer timer;
static double f_frame_time_average = 0;



/**
 *	@brief vyrobi OpenGL texturu z bitmapy
 *	@param[in] p_bitmap je obrazek textury
 *	@param[in] n_internal_format je pozadovany format ulozeni textury na graficke karte (format obrazku je v typu TBmp vzdy GL_RGBA)
 *	@return vraci id textury (neuvazuje neuspech)
 */
GLuint n_GLTextureFromBitmap(const TBmp *p_bitmap, GLenum n_internal_format = GL_RGBA8)
{
	GLuint n_tex;
	glGenTextures(1, &n_tex);
	glBindTexture(GL_TEXTURE_2D, n_tex);
	//glEnable(GL_TEXTURE_2D); !! OpenGL 3 uz nema enable / disable textur, shader si sam cte textury ktere potrebuje !!
	// vyrobi a nastavi novou 2D texturu, zatim nema zadne data

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// nastavime filtrovani

	glTexImage2D(GL_TEXTURE_2D, 0, n_internal_format, p_bitmap->n_width, p_bitmap->n_height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, p_bitmap->p_buffer);
	// specifikujeme data textury

	glGenerateMipmap(GL_TEXTURE_2D);
	// nechame OpenGL spocitat jeji mipmapy

	return n_tex;
}

/**
 *	@brief vraci "pointer" do VBO
 *	@param[in] off je offset v bytech, na ktery ma vraceny pointer ukazovat
 */
#define p_OffsetInVBO(off) ((void*)(off))

/**
 *	@brief vytvori vsechny OpenGL objekty, potrebne pro kresleni
 *	@return vraci true pri uspechu, false pri neuspechu
 */
bool InitGLObjects() {		
	//glEnable(GL_CULL_FACE); // not such a good idea

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


	// VBO pro ulozeni zobrazovanych barev patchu (iluminativni energie; barvy jsou zabalene do intu)
	glGenBuffers(1, &n_patch_color_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
	// na zacatku maji iluminativni energii pouze svetla
	{
		Patch** patches = scene.getPatches();
		unsigned int indCnt = scene.getPatchesCount() * 4; // pocet neopakujicich se indexu
		uint32_t* colorData = new uint32_t[indCnt];
		for (unsigned int i=0; i < indCnt; i++) {
			colorData[i] = Colors::packColor( patches[i/4]->illumination ); // vychozi illuminance patchu (maji jen svetla)
		}
		glBufferData(GL_ARRAY_BUFFER, indCnt * sizeof(uint32_t), colorData, GL_DYNAMIC_DRAW);
		delete[] colorData;
	}

	// VBO pro ulozeni radiativnich energii patchu 
	glGenBuffers(1, &n_patch_radiative_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, n_patch_radiative_buffer_object);
	{
		Patch** patches = scene.getPatches();
		unsigned int indCnt = scene.getPatchesCount() * 4; // pocet neopakujicich se indexu
		uint32_t* colorData = new uint32_t[indCnt];
		for (unsigned int i=0; i < indCnt; i++) {
			colorData[i] = Colors::packColor( patches[i/4]->radiosity );
		}
		glBufferData(GL_ARRAY_BUFFER, indCnt * sizeof(uint32_t), colorData, GL_DYNAMIC_DRAW);
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
	
		// rekneme OpenGL odkud si ma brat data pro 1. atribut shaderu; kazda barva je zabalena v uintu	
		glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_INT_2_10_10_10_REV, false, 0, p_OffsetInVBO(0));

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, PATCHVIEW_TEX_W, PATCHVIEW_TEX_H, 0, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	


	// vytvorit FBO pro kresleni pohledu z patchu do textury
	fbo = new CGLFrameBufferObject(
		PATCHVIEW_TEX_W, PATCHVIEW_TEX_H,
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
	

	
	// pole ID patchu a jejich energii (na shodnych indexech)
	ocl_arg_ids = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, PATCHVIEW_TEX_RES*sizeof(uint32_t), NULL, &error);
	ocl_arg_energies = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, PATCHVIEW_TEX_RES*sizeof(float), NULL, &error);

	// index do ID patchu a energii, sdileny mezi instacemi, atomicky posouvany
	ocl_arg_writeindex = clCreateBuffer(ocl_context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &error);

	// data textury pohledu z nejakeho patche
	//ocl_arg_patchview = clCreateBuffer(ocl_context, CL_MEM_READ_ONLY, PATCHVIEW_TEX_RES*sizeof(unsigned int), NULL, &error);
	ocl_arg_patchview = clCreateFromGLTexture2D(ocl_context, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, n_patchlook_texture, &error);

	// data 'textury' form factoru
	ocl_arg_ffactors = clCreateBuffer(ocl_context, CL_MEM_READ_ONLY, PATCHVIEW_TEX_RES*sizeof(float), NULL, &error);

	_ASSERT(error == CL_SUCCESS);

	// nacist zdrojovy kod kernelu
	FILE* fp = fopen("ProcessHemicube.cl", "rb");
	if (fp == NULL) {
		cerr << "error: can't open OpenCL source file" << endl;
		return false;
	}
	fseek(fp, 0, SEEK_END);
	size_t filesize = ftell(fp);
	rewind(fp);
	
	char* source = new char[filesize];
	if (fread(source, sizeof(char), filesize, fp) != filesize) {
		cerr << "error: can't read OpenCL source file" << endl;
		return false;
	}
	fclose(fp);	
	
	// pridat makro na rozbaleni UINT_2_10_10_10_REV do indexu barvy (Colors::index)
	// pres parametr pri kompilaci to nejak nefunguje
	short* shifts = Colors::getShifts();
	unsigned int* rMasks = Colors::getRevMasks();
	ostringstream opts(ostringstream::out);
	opts << "#define unpack(c) ( (((c) & " << rMasks[0] << ") >> " << shifts[0] << ") | (((c) & " << rMasks[1] << ") >> " << shifts[1] << ") | (((c) & " << rMasks[2] << ") >> " << shifts[2] << ") )" << endl;	
	opts << "#define correction " << Colors::getCorrection() << endl;
	
	string src = opts.str() + string(source, filesize);
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
	delete[] source;
	

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

		
	
	unsigned int spanLength = PATCHVIEW_TEX_W;

	// nastavit argumenty kernelu	
	error  = clSetKernelArg (ocl_kernel, 0, sizeof(cl_mem), &ocl_arg_ids);
	error |= clSetKernelArg (ocl_kernel, 1, sizeof(cl_mem), &ocl_arg_energies);
	error |= clSetKernelArg (ocl_kernel, 2, sizeof(cl_mem), &ocl_arg_writeindex);
	error |= clSetKernelArg (ocl_kernel, 3, sizeof(cl_mem), &ocl_arg_patchview);
	error |= clSetKernelArg (ocl_kernel, 4, sizeof(cl_sampler), &ocl_sampler);
	error |= clSetKernelArg (ocl_kernel, 5, sizeof(cl_mem), &ocl_arg_ffactors);
	error |= clSetKernelArg (ocl_kernel, 6, sizeof(unsigned int), &PATCHVIEW_TEX_W);
	error |= clSetKernelArg (ocl_kernel, 7, sizeof(unsigned int), &PATCHVIEW_TEX_H);
	error |= clSetKernelArg (ocl_kernel, 8, sizeof(unsigned int), &spanLength);
	if (error != CL_SUCCESS) {
		cerr << "error: can't set up ProcessHemicube kernel arguments" << endl;
		return false;
	}

	// naplnit buffer argumentu - pole s formfactory - konstantni po celou dobu
	error = clEnqueueWriteBuffer(ocl_queue, ocl_arg_ffactors, CL_TRUE, 0, PATCHVIEW_TEX_RES*sizeof(float), p_formfactors,
 					0, NULL, NULL);
	if (error != CL_SUCCESS) {
		cerr << "error: can't write to ocl_arg_ffactors buffer" << endl;
		return false;
	}

	cout << "OpenCL init OK" << endl;
	return true;
}

/**
 *	@brief uvolni vsechny OpenGL objekty
 */
void CleanupGLObjects()
{
	delete[] n_color_array_object;
	// smaze dynamicky alokovane pole vao

	glDeleteBuffers(1, &n_vertex_buffer_object);
	glDeleteBuffers(1, &n_index_buffer_object);
	// smaze vertex buffer objekty

	Shaders::cleanup();
	// smaze shadery		

	glDeleteTextures(1, &n_patchlook_texture);
	// smaze textury

	delete fbo;
	// smaze render buffer
}

/**
 *  @brief uvolni vsechny OpenCL objekty
 */
void CleanupCLObjects() {
	clReleaseKernel(ocl_kernel);
	clReleaseCommandQueue(ocl_queue);
	clReleaseContext(ocl_context);
	clReleaseMemObject(ocl_arg_ids);
	clReleaseMemObject(ocl_arg_energies);
	clReleaseMemObject(ocl_arg_writeindex);
	clReleaseMemObject(ocl_arg_patchview);
	clReleaseMemObject(ocl_arg_ffactors);
}

/**
 *	@brief vykresli uzivatelsky pohled do sceny (skutecne barvy patchu)
 */
void DrawScene() {		
	glBindVertexArray(n_vertex_array_object);
	//glVertexAttribP1ui(1, GL_UNSIGNED_INT_2_10_10_10_REV, false, 1024);
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

	if (patchIntervals.size() > 1) {
		glBindVertexArray(n_vertex_array_object);
		glVertexAttrib3f(1, 0.0f, 0.0f, 0.0f); // vse cerne
	}

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
			printf((i)? ", %s" : "%s", p_s_ext_name);
		}
		printf("\n");
	}
	if(!GL_VERSION_3_0) {
		fprintf(stderr, "error: OpenGL 3.0 not supported\n"); // OpenGL 3.0 neni podporovane
		return -1;
	}
	// zkontroluje zda jsou podporovane pozadovane rozsireni

	// nacteme globalni objekt sceny a nastavime limit velikosti patchu
	scene.load();	
	scene.maxPatchArea = MAX_PATCH_AREA;

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
				if (n_w_param == VK_ESCAPE || n_w_param == 0x51)
					SendMessage(h_wnd, WM_CLOSE, 0, 0);
				
				// E
				if (n_w_param == 0x45) {
					/*
					cout << "patch cam: " << endl;
					patchCam.DebugDump();
					cout << "user cam: " << endl;
					cam.DebugDump();
					*/
					// vypsat patche ve scene s informacema o energiich
					cout << "\t\tR\t\t\tI\t\t" << endl;
					Patch** p = scene.getPatches();			
					cout << setiosflags(ios::fixed) << setprecision(3);
					for (unsigned int i=0; i < scene.getPatchesCount(); i++) {
						Vector3f r = p[i]->radiosity;
						Vector3f il = p[i]->illumination;
						cout << i << "\t" <<
									setw(5) << r.x << " " <<
									setw(5) << r.y << " " <<
									setw(5) << r.z << "\t" << il.x << " " << il.y << " " << il.z << endl;
					}
				}

				// F
				if (n_w_param == 0x46) {
					if (wireframe) {
						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
						wireframe = false;
					} else {
						glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
						wireframe = true;
					}
				}

				// X
				if (n_w_param == 0x58) {					
					step++;
					printf("step: %i\n", step);
				}

				// Y
				if (n_w_param == 0x59) {
					step--;
					if (step < 0)
						step = 0;
					printf("step: %i\n", step);
				}

				// TAB - prepinani mezi vykreslovanim illuminativni a radiativni energie
				if (n_w_param == 0x09) {
					b_draw_radiative = !b_draw_radiative;
					if (b_draw_radiative) {
						// zmenit VBO v userview VAO na radiativni energie
						glBindVertexArray(n_vertex_array_object);														
						glBindBuffer(GL_ARRAY_BUFFER, n_patch_radiative_buffer_object);
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(1, 4, GL_UNSIGNED_INT_2_10_10_10_REV, false, 0, p_OffsetInVBO(0));
						glBindVertexArray(0); 
						cout << "radiative energies" << endl;
					} else {
						// zmenit VBO v userview VAO na iluminativni energie
						glBindVertexArray(n_vertex_array_object);														
						glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(1, 4, GL_UNSIGNED_INT_2_10_10_10_REV, false, 0, p_OffsetInVBO(0));
						glBindVertexArray(0); 
						cout << "illuminative energies" << endl;
					}					
				}

				// O
				if (n_w_param == 0x4f)
					debugOutput = !debugOutput;

				// P
				if (n_w_param == KEY_P) 
					showPatchLook = !showPatchLook;

				// L
				if (n_w_param == KEY_L) {
					computeRadiosity = !computeRadiosity;
					if (computeRadiosity)
						cout << "Light emitting continues" << endl;
					else
						cout << "Light emitting paused" << endl;
				}

				// pgUp
				if (n_w_param == 0x21) {
					if (lookFromPatch+1 < scene.getPatchesCount()) {
						lookFromPatch++;
					}
					printf("looking from patch: %lu\n", lookFromPatch);
				}
				// pgDown
				if (n_w_param == 0x22) {
					if (lookFromPatch > 0) {
						lookFromPatch--;						
					}
					printf("looking from patch: %lu\n", lookFromPatch);
				}
				// up
				if (n_w_param == 0x26) {
					lookFromPatchDir = Camera::PATCH_LOOK_UP;
					cout << "looking up" << endl;
				}
				// down
				if (n_w_param == 0x28) {
					lookFromPatchDir = Camera::PATCH_LOOK_DOWN;
					cout << "looking down" << endl;
				}
				// left
				if (n_w_param == 0x25) {
					lookFromPatchDir = Camera::PATCH_LOOK_LEFT;
					cout << "looking left" << endl;
				}
				// right
				if (n_w_param == 0x27) {
					lookFromPatchDir = Camera::PATCH_LOOK_RIGHT;
					cout << "looking right" << endl;
				}
				// space
				if (n_w_param == 0x20) {
					lookFromPatchDir = Camera::PATCH_LOOK_FRONT;
					cout << "looking front" << endl;
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
	/*
	// P - zobrazit/skryt nahledovy kriz
	if (HIBYTE(GetKeyState(KEY_P)) & 0x01)
		showPatchLook = !showPatchLook;
	*/
	/*
	// X - provest krok
	if (HIBYTE(GetKeyState(0x58)) & 0x01)
		step++;	
	*/
}


/**
 *	@brief tato funkce se vola ve smycce pokazde kdyz nejsou zadne nove zpravy; lze vyuzit ke kresleni animace
 *	@param[in] driver je reference na OpenGL driver
 */
void OnIdle(CGL30Driver &driver)
{
	// spustit stopky fps
	double t_start = timer.f_Time();


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

	// pouzije shader pro uzivatelsky pohled
	glUseProgram(n_patch_program_object);

	fbo->Bind();
	fbo->Bind_ColorTexture2D(0, GL_TEXTURE_2D, n_patchlook_texture);

	glViewport(0, 0, fbo->n_Width(), fbo->n_Height());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // vycisti buffery

	Camera::PatchLook p_patchlook_perm[] = {Camera::PATCH_LOOK_UP, Camera::PATCH_LOOK_DOWN,
		Camera::PATCH_LOOK_LEFT, Camera::PATCH_LOOK_RIGHT, Camera::PATCH_LOOK_FRONT};

	// 'okna' do kterych se budou kreslit jednotlive pohledy; odpovida 'nakresu' v FormFactors.cpp
	// x, y, w, h		(0,0 = levy dolni roh)
	int p_viewport_list[][4] = {
		{	0,						HEMICUBE_H,		HEMICUBE_W,	HEMICUBE_H	},
		{	HEMICUBE_W,				HEMICUBE_H/2,	HEMICUBE_W,	HEMICUBE_H	},
		{	-1*int(HEMICUBE_W/2),	0,				HEMICUBE_W,	HEMICUBE_H	},
		{	int(HEMICUBE_W*1.5),	0,				HEMICUBE_W,	HEMICUBE_H	},
		{	HEMICUBE_W/2,			0,				HEMICUBE_W, HEMICUBE_H	}
	};

	// oblasti v texture, do kterych je povoleno kreslit;
	// jelikoz se nektere casti kresli pres sebe, muze pri kresleni 'pruhledna' vznikat
	// nezadouci zviditelneni drive vykreslene casti pohledu, ktery ale ma byt skryty, coz resi glScissor
	// 
	// x, y, w, h
	int p_scissors_list[][4] = {
		{	0,						HEMICUBE_H,		HEMICUBE_W,		HEMICUBE_H/2	},
		{	HEMICUBE_W,				HEMICUBE_H,		HEMICUBE_W,		HEMICUBE_H/2	},
		{	0,						0,				HEMICUBE_W/2,	HEMICUBE_H		},
		{	int(HEMICUBE_W*1.5),	0,				HEMICUBE_W/2,	HEMICUBE_H		},
		{	HEMICUBE_W/2,			0,				HEMICUBE_W,		HEMICUBE_H		}
	};

	// najit patch s nejvetsi energii
	unsigned int patchId = scene.getHighestRadiosityPatchId();

	glEnable(GL_SCISSOR_TEST);

	// celkem 5 pohledu
	for(int i=0; i < 5; i++) {
				
		Camera::PatchLook dir = p_patchlook_perm[i];

		// spocitame modelview - projection matici, kterou potrebujeme k transformaci vrcholu		
		{
			// matice perspektivni projekce
			Matrix4f t_projection;
			CGLTransform::Perspective(t_projection, 90, 1.0f, 0.01f, 1000);		 // RATIO JE 1.0!!!
		
			// modelview
			Matrix4f t_modelview;
			t_modelview.Identity();				

			// vynasobit pohledem kamery patche
			patchCam.lookFromPatch(scenePatches[patchId], dir);
			t_modelview *= patchCam.GetMatrix();

			// matice pohledu kamery
			t_mvp = t_projection * t_modelview;
		}

		// nahrajeme matici do OpenGL jako parametr shaderu
		glUniformMatrix4fv(n_patchprogram_mvp_matrix_uniform, 1, GL_FALSE, &t_mvp[0][0]);		

		// nastavit parametry viewportu a oblast, do ktere je povoleno kreslit
		glScissor(p_scissors_list[i][0], p_scissors_list[i][1], p_scissors_list[i][2], p_scissors_list[i][3]);
		glViewport(p_viewport_list[i][0], p_viewport_list[i][1], p_viewport_list[i][2], p_viewport_list[i][3]);
		
		// vykreslit do textury (pres FBO)		
		unsigned int interval = step % patchIntervals.size();
		DrawPatchLook(interval);
	}

	glDisable(GL_SCISSOR_TEST);

	fbo->Bind_ColorTexture2D(0, GL_TEXTURE_2D, 0);
	fbo->Release();
	




	// zpracovat vyrenderovanou texturu?
	if (computeRadiosity) {	
		
		cl_int error = 0;		
		
		/*
		// ziskat data z textury a zkopirovat je do OCL bufferu
		unsigned int* data = new unsigned int[PATCHVIEW_TEX_RES]; // alokovat predem		
		glBindTexture(GL_TEXTURE_2D, n_patchlook_texture); 
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, data);		
		glBindTexture(GL_TEXTURE_2D, 0); // odbindovat texturu, data uz jsou zkopirovana do pameti
		//memset(data, 0, PATCHVIEW_TEX_RES*sizeof(unsigned int));
		error = clEnqueueWriteBuffer(ocl_queue, ocl_arg_patchview, CL_TRUE, 0, PATCHVIEW_TEX_RES*sizeof(uint32_t), data, 0, NULL, NULL);
		delete[] data;	// uz jsou v bufferu
		*/

		// ziskat pristup k OGL texture s pohledem z patche		
		glFinish(); // nutne pro sync
		error |= clEnqueueAcquireGLObjects(ocl_queue, 1, &ocl_arg_patchview, 0, NULL, NULL);

		// vynulovat index na ktery se zapisuje - nutne v kazde iteraci!
		{
			unsigned int writeindex = 0;
			error |= clEnqueueWriteBuffer(ocl_queue, ocl_arg_writeindex, CL_TRUE, 0, sizeof(unsigned int), &writeindex,	0, NULL, NULL);
		}
		_ASSERT(error == CL_SUCCESS);		
		
		// pocet instanci programu		
		const unsigned int global_work_size[] = { OCL_WORKITEMS_X, OCL_WORKITEMS_Y };
		
		// spustit program!
		error = clEnqueueNDRangeKernel(ocl_queue, ocl_kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);		
		_ASSERT(error == CL_SUCCESS);

		
		// zjistit kolik polygonu*instanci se ulozilo (pocet je vzdy ruzny v zavislosti na pohledu a rozlozeni work-items)
		unsigned int n_last_index = 0;
		error = clEnqueueReadBuffer (ocl_queue, ocl_arg_writeindex, CL_TRUE, 0, sizeof(unsigned int), &n_last_index, 0, NULL, NULL);
		_ASSERT(error == CL_SUCCESS);

		// alokovat podle potreby (poctu indexu na ktere se zapsalo)
		uint32_t* p_pids = new uint32_t[n_last_index];
		float* p_energies = new float[n_last_index];

		// precist data
		error  = clEnqueueReadBuffer (ocl_queue, ocl_arg_ids, CL_TRUE, 0, n_last_index*sizeof(uint32_t), p_pids, 0, NULL, NULL);
		error |= clEnqueueReadBuffer (ocl_queue, ocl_arg_energies, CL_TRUE, 0, n_last_index*sizeof(float), p_energies, 0, NULL, NULL);		
		_ASSERT(error == CL_SUCCESS);

		// uvolnit OGL objekty z drzeni OCL
		clFinish(ocl_queue); // nutne pro sync
		error |= clEnqueueReleaseGLObjects(ocl_queue, 1, &ocl_arg_patchview, 0, NULL, NULL);
		_ASSERT(error == CL_SUCCESS);


		// nabindovat buffer s barvami patchu
		glBindBuffer(GL_ARRAY_BUFFER, n_patch_color_buffer_object);
		
		Patch* emitter = scenePatches[patchId]; // patch ze ktereho se koukalo
		
		// prenest energie
		for (unsigned int i = 0; i < n_last_index; i++) {			

			if (p_pids[i] >= scenePatchesCount) {
				cerr << "! warning !\t uknown patch id: " << p_pids[i] << endl;
				continue;
			}

			Patch* p = scenePatches[p_pids[i]];			
			p->radiosity += emitter->radiosity * p_energies[i] * p->getReflectivity();
		}

		// zdroj se vyzaril
		emitter->illumination += emitter->radiosity;
		emitter->radiosity = Vector3f(0.0f, 0.0f, 0.0f);
				
		if (emitter->illumination.x > 1.0)
			emitter->illumination.x = 1.0;
		if (emitter->illumination.y > 1.0)
			emitter->illumination.y = 1.0;
		if (emitter->illumination.z > 1.0)
			emitter->illumination.z = 1.0;
		
		uint32_t newColor = Colors::packColor(emitter->illumination * emitter->getColor());
		uint32_t newData[4] = {
			newColor,
			newColor,
			newColor,
			newColor
		};

		glBufferSubData(GL_ARRAY_BUFFER, patchId * 4 * sizeof(uint32_t), 4 * sizeof(uint32_t), newData); // aktualizovat data		
		glBindBuffer(GL_ARRAY_BUFFER, 0); // odbindovat buffer
		

		// nabindovat VBO s radiativnimi energiemi a updatovat jej
		glBindBuffer(GL_ARRAY_BUFFER, n_patch_radiative_buffer_object);
		uint32_t* buffer = (uint32_t*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (buffer != NULL) { // TODO: better!
			for (unsigned int i=0; i < scenePatchesCount; i++) {
				// patche kter nejsou v pohledu nebo uz nemaji radiativni energii nas nezajimaji
				if (scenePatches[i]->radiosity.f_Length2()==0)
					continue;

				uint32_t newColor = Colors::packColor(scenePatches[i]->radiosity);
				uint32_t newData[4] = {
					newColor,
					newColor,
					newColor,
					newColor
				};

				memcpy(buffer + i*4, newData, 4*sizeof(uint32_t));
			}

			if (glUnmapBuffer(GL_ARRAY_BUFFER) == GL_FALSE)
				cerr << "Chyba pri uvolneni mapovani VBO" << endl;
		} else
			cerr << "Chyba pri mapovani VBO" << endl;

		glBindBuffer(GL_ARRAY_BUFFER, 0); // odbindovat buffer
		

		// uklidit
		delete[] p_pids;
		delete[] p_energies;


		
		// zbyvajici energie ve scene
		float total = 0;
		for (unsigned int i = 0; i < scenePatchesCount; i++)
			total += scenePatches[i]->radiosity.f_Length2();

		if (total < 0.1) {
			if (debugOutput)
				cout << "pass " << passCounter << ", done!" << endl;
			computeRadiosity = false; 
		} else if (debugOutput)	
			cout << "pass " << passCounter << ", " << setprecision(10) << total << " energy left" << endl;
		
		passCounter++;
		//computeRadiosity = false;
	}



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

	// spocitat fps
	glFinish();
	f_frame_time_average = f_frame_time_average * .9 + (timer.f_Time() - t_start) * .1;
	double f_fps = 1 / f_frame_time_average;
	ostringstream winTitle;
	winTitle << p_s_window_name << ", FPS: " << f_fps;
	SetWindowText(h_wnd, winTitle.str().c_str());
}
