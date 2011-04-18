#pragma warning(disable:4996)

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
#include "LoadingModel.h"
#include "Kernel_ProcessHemicube.h"

using namespace std;


// parametr pro subdivision
#define MAX_PATCH_AREA 0.1


// rozmery hemicube, resp. pohledove textury jsou v FormFactors.h
const unsigned int OCL_WORKITEMS_X = 8;
const unsigned int OCL_WORKITEMS_Y = PATCHVIEW_TEX_H;
const unsigned int OCL_SPANLENGTH = PATCHVIEW_TEX_W / OCL_WORKITEMS_X;


// parametry okna
static const char *p_s_window_name = "Radiosity renderer";
static const char *p_s_class_name = "my_wndclass";
static int n_width = 800;
static int n_height = 600;

// flag pro vyskoceni ze smycky zprav
static bool b_running = true;

// okno, GL driver
HWND h_wnd;
CGL30Driver driver;



// textura, do ktere se renderuje pohled z patche
static GLuint	n_patchlook_texture;

// vbos, vaos
static GLuint	n_vertex_buffer_object,				// VBO s geometrii sceny 
				n_index_buffer_object,				// VBO s indexy vrcholu sceny
				n_patch_color_buffer_object,		// VBO s iluminativnimi energiemi patchu
				n_patch_radiative_buffer_object,	// VBO s radiativnimi energiemi patchu
				n_vertex_array_object,				// VAO pro scenu
				n_tex_buffer_object,				// !!!
				n_vbo_square,						// VBO pro nahledove okynko
				n_vao_square,						// VAO pro nahledove okynko
				n_id_color_buffer_object;			// VBO s jednim intervalem unikatnich barev pro kresleni pohledu z patche

// pole VAO pro kazdy interval vykreslovanych patchu
static GLuint*	n_color_array_object = NULL; 

// objekt shaderu pro kresleni uzivatelskeho pohledu a jeho parametry
static GLuint	n_user_program_object, 
				n_user_mvp_matrix_uniform, 
				n_box_sampler_uniform;

// objekt shaderu pro pohled z patche a jeho parametr
static GLuint	n_patch_program_object, 
				n_patchprogram_mvp_matrix_uniform;

// objekt shaderu pro nahledove okynko a jeho parametr
static GLuint	n_preview_program_object, 
				n_preview_mvp_matrix_uniform;

// pole form factoru o velikosti odpovidajici rozliseni textury hemicube
static float*	p_formfactors = NULL;


// CL objekty
static cl_platform_id ocl_platform;
static cl_kernel ocl_kernel;
static cl_context ocl_context;
static cl_command_queue ocl_queue;
static cl_device_id ocl_device;
static uint32_t* ocl_data_patchids; // !!!
static float* ocl_data_formfactors; // !!!

// CL argumenty programu
cl_mem	ocl_arg_ids, 
		ocl_arg_energies, 
		ocl_arg_writeindex, 
		ocl_arg_patchview, 
		ocl_arg_ffactors;


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

// objekt offscreen FrameBufferu do ktereho se kresli pohled z patche
CGLFrameBufferObject* fbo = NULL;


// pocitadlo pruchodu - kolikrat probehla distribuce energie 
unsigned int passCounter = 0;

// priznak pro ne/vypisovani informaci pri distribuci energie (E) // !!! I
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
 *	@brief vraci "pointer" do VBO
 *	@param[in] off je offset v bytech, na ktery ma vraceny pointer ukazovat
 */
#define p_OffsetInVBO(off) ((void*)(off))


void OnIdle(CGL30Driver &driver);
void onResize(int n_x, int n_y, int n_new_width, int n_new_height);
LRESULT CALLBACK WndProc(HWND h_wnd, UINT n_msg, WPARAM n_w_param, LPARAM n_l_param);
void smoothShadePatch(uint32_t* colors, Patch* p);
void SaveToFile();
void LoadFromFile();
// prototypy funkci




// docasne - pro ruzne testovani
int step = 0; // !!!
unsigned long lookFromPatch = 0; // !!!
Camera::PatchLook lookFromPatchDir = Camera::PATCH_LOOK_FRONT; // !!!
