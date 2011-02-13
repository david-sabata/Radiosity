#include <GL/glew.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include "OpenGL30Drv.h"
#include "Transform.h"
#include "Tga.h"
#include "Camera.h"
#include "ModelContainer.h"
#include <vld.h> 

// pro Cornell Box
// 0.1   =    69 000 vertexu
// 0.01  =   368 000 vertexu
// 0.001 = 5 000 000 vertexu
#define MAX_PATCH_AREA 0.5

static const char *p_s_window_name = "Radiosity renderer";
static const char *p_s_class_name = "my_wndclass";
// jmeno okna

static int n_width = 800;
static int n_height = 600;
// velikost okna

static bool b_running = true;
// flag pro vyskoceni ze smycky zprav

void OnIdle(CGL30Driver &driver);
void onResize(int n_x, int n_y, int n_new_width, int n_new_height);
LRESULT CALLBACK WndProc(HWND h_wnd, UINT n_msg, WPARAM n_w_param, LPARAM n_l_param);
// prototypy funkci

static GLuint n_box_texture, n_fire_texture;
static GLuint n_vertex_buffer_object, n_index_buffer_object, n_vertex_array_object;
static GLuint n_vertex_shader, n_fragment_shader, n_program_object, n_mvp_matrix_uniform,
	n_box_sampler_uniform, n_fire_sampler_uniform, n_time_uniform;
static GLuint n_color_buffer_object;
static GLuint n_color_array_object;
//static vector<GLuint>* n_color_array_object = NULL; // pole VAO pro kazdy interval vykreslovanych patchu
// OpenGL objekty


// citlivosti / rychlosti pohybu
static float mouse_sensitivity = 0.001f;
static float keys_sensitivity = 0.004f;

// je mys ovladana programem? (v pripade nastavovani pozice mysi nechceme odchytavat eventy)
static bool b_mouse_controlled = false;

// aliasy pro virtual-keys
#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_S 0x53
#define KEY_W 0x57

// objekt kamery
Camera cam;

// objekt sceny
ModelContainer scene;

// docasne - pro ruzne krokovani
int step = 0;

// zobrazit pouze wireframe?
bool wireframe = false;


// funkcni deklarace
int getColorRange();
float* getColors();


typedef struct { 
	int from, to; 
} interval;



/**
 *	@brief zkontroluje zda byl shader spravne zkompilovan, tiskne chyby do stderr
 *	@param[in] n_shader_object je id shaderu
 *	@param[in] p_s_shader_name je jmeno shaderu (jen pro uzivatele - aby vedel o ktery shader se jedna pokud se vytiskne nejaka chyba)
 *	@return vraci true pro uspesne zkompilovane shadery, jinak false
 */
bool CheckShader(GLuint n_shader_object, const char *p_s_shader_name)
{
	bool b_compiled;
	{
		int n_tmp;
		glGetShaderiv(n_shader_object, GL_COMPILE_STATUS, &n_tmp);
		b_compiled = n_tmp == GL_TRUE;
	}
	int n_log_length;
	glGetShaderiv(n_shader_object, GL_INFO_LOG_LENGTH, &n_log_length);
	// query status ...

	if(n_log_length > 1) {
		char *p_s_info_log;
		try {
			p_s_info_log = new char[n_log_length + 1];
		} catch (bad_alloc& ba) {
			ba = NULL;
			return false;
		}
		//if(!(p_s_info_log = new(std::nothrow) char[n_log_length + 1]))
		//			return false;
		// alloc temp storage for log string

		glGetShaderInfoLog(n_shader_object, n_log_length, &n_log_length, p_s_info_log);
		// get log string

		printf("GLSL compiler (%s): %s\n", p_s_shader_name, p_s_info_log);
		// print info log

		delete[] p_s_info_log;
		// delete temp storage
	}
	// get info-log

	return b_compiled;
}

/**
 *	@brief zkontroluje zda byl program spravne slinkovan, tiskne chyby do stderr
 *	@param[in] n_program_object je id programu
 *	@param[in] p_s_program_name je jmeno programu (jen pro uzivatele - aby vedel o ktery program se jedna pokud se vytiskne nejaka chyba)
 *	@return vraci true pro uspesne slinkovne programy, jinak false
 */
bool CheckProgram(GLuint n_program_object, const char *p_s_program_name)
{
	bool b_linked;
	{
		int n_tmp;
		glGetProgramiv(n_program_object, GL_LINK_STATUS, &n_tmp);
		b_linked = n_tmp == GL_TRUE;
	}
	int n_length;
	glGetProgramiv(n_program_object, GL_INFO_LOG_LENGTH, &n_length);
	// query status ...

	if(n_length > 1) {
		char *p_s_info_log;
		try {
			p_s_info_log = new char[n_length + 1];
		} catch (bad_alloc& ba) {
			ba = NULL;
			return false;
		}
		//if(!(p_s_info_log = new(std::nothrow) char[n_length + 1]))
		//			return false;
		// alloc temp log

		glGetProgramInfoLog(n_program_object, n_length, &n_length, p_s_info_log);
		// get info log

		printf("GLSL linker (%s): %s\n", p_s_program_name, p_s_info_log);
		// print info log

		delete[] p_s_info_log;
		// release temp log
	}
	// get info-log

	return b_linked;
}

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
	

	// VBO pro ulozeni barev - bude obsahovat cely rozsah barev
	glGenBuffers(1, &n_color_buffer_object);	
	glBindBuffer(GL_ARRAY_BUFFER, n_color_buffer_object);	
	
	float* colorData = getColors(); // colorRange * 3 (barvy) floatu
	int colorRange = getColorRange();	
	int indicesCount = scene.getIndicesCount();

	// barvy v bufferu pouze jedenkrat - nestacilo by?
	//glBufferData(GL_ARRAY_BUFFER, getColorRange() * 12 * sizeof(float), colorData, GL_STATIC_DRAW);
		
	// buffer barev pro vsechny indexy
	float* indexColors = new float[ indicesCount * 3 ];

	int offset = 0;	// offset v indexColors na kterem je prvni hodnota prave ukladane barvy
	int colorOffset = 0; // offset v colorData na kterem je prvni hodnota aktualni barvy
	
	/*
	// naplni buffer barvami pro vsechny indexy (3 floaty pro 1 index)
	while (offset < indicesCount * 3) {
		float r = colorData[ colorOffset * 3 ];
		float g = colorData[ colorOffset * 3 + 1 ];
		float b = colorData[ colorOffset * 3 + 2 ];
			
		// vzdy 4 indexy v rade za sebou budou mit stejnou barvu
		for (int v = 0; v < 4; v++) {					
			indexColors[ offset ] = r;
			indexColors[ offset + 1] = g;
			indexColors[ offset + 2] = b;
			offset += 3;
		}
		offset+=3;

		colorOffset = (colorOffset + 1) % colorRange;	
	}
	*/

	while (offset < indicesCount * 3) {
		float r = colorData[ colorOffset * 3 ];
		float g = colorData[ colorOffset * 3 + 1 ];
		float b = colorData[ colorOffset * 3 + 2 ];
			
		// vzdy 6 indexu v rade za sebou (tj. dva trojuhelniky, tj. jeden patch) budou mit stejnou barvu
		for (int v = 0; v < 6; v++) {					
			indexColors[ offset ] = r;
			indexColors[ offset + 1] = g;
			indexColors[ offset + 2] = b;
			offset += 3;
		}

		colorOffset = (colorOffset + 1) % colorRange;	
	}
	

	if (0) {
		cout << "-------------------" << endl;
		for (int i = 0; i < indicesCount * 3; i++) {
			cout << indexColors[i] << "\t";
			if (i % 3 == 2)
				cout << endl;
		}
	}
	
	glBufferData(GL_ARRAY_BUFFER, indicesCount * 3 * sizeof(float), indexColors, GL_STATIC_DRAW);	
	
	// data jsou uz zkopirovana ve VBO	
	delete[] indexColors;
	delete[] colorData; 	



	// VAO definuje ktery VBO se preda shaderu a jak se data naskladaji do vstupnich atributu
	glGenVertexArrays(1, &n_vertex_array_object);
	glBindVertexArray(n_vertex_array_object);
	{		
		// rekneme OpenGL odkud si ma brat data; kazdy vrchol ma 3 souradnice,		
		glBindBuffer(GL_ARRAY_BUFFER, n_vertex_buffer_object);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(0));
	
		/*
		// rekneme OpenGL odkud si ma brat data pro 1. atribut shaderu; kazda barva ma 3 hodnoty,		
		glBindBuffer(GL_ARRAY_BUFFER, n_color_buffer_object);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(0));
		*/

		// rekneme OpenGL odkud bude brat indexy geometrie pro glDrawElements (nize)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, n_index_buffer_object);
		
	} // tento blok se "zapamatuje" ve VAO
	
	// vratime VAO 0, abychom si nahodne VAO nezmenili (pripadne osetreni 
	// proti chybe v ovladacich nvidia kde se VAO poskodi pri volani nekterych wgl funkci)
	// vytvorime a nastavime vertex array objekt - objekt na GPU, ktery si pamatuje konfiguraci 
	// nastaveni VBO. pak pri kresleni volame jen glBindVertexArray() namisto vsech prikazu v bloku vyse
	glBindVertexArray(0); 
	

	
	// druhy VAO bude pro ulozeni barev
	glGenVertexArrays(1, &n_color_array_object);
	glBindVertexArray(n_color_array_object);
	{
		
		// rekneme OpenGL odkud si ma brat data; kazdy vrchol ma 3 souradnice,		
		glBindBuffer(GL_ARRAY_BUFFER, n_vertex_buffer_object);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(0));			

		// rekneme OpenGL odkud si ma brat data pro 1. atribut shaderu; kazda barva ma 3 hodnoty,		
		glBindBuffer(GL_ARRAY_BUFFER, n_color_buffer_object);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, p_OffsetInVBO(0));		
		
		// rekneme OpenGL odkud bude brat indexy geometrie pro glDrawElements (nize)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, n_index_buffer_object);		

	} // tento blok se "zapamatuje" ve VAO	

	// vratit VAO 0
	glBindVertexArray(0);

	


	const char *p_s_vertex_shader =
		"#version 330\n"		
		"in vec3 v_pos;\n" // atributy - vstup z dat vrcholu
		"in vec3 v_col;\n" // barva vrcholu
		"\n"
		"uniform float f_time;\n" // parametr shaderu - cas pro animovani
		"\n"
		"uniform mat4 t_modelview_projection_matrix;\n" // parametr shaderu - transformacni matice
		"\n"
		"out vec3 v_color;\n"
		/*
		"out vec2 v_texcoord;\n" 
		"out vec2 v_texcoord1;\n" 
		"out vec2 v_texcoord2;\n" 
		"out vec2 v_texcoord3;\n" // vystupy - souradnice textury
		*/
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = t_modelview_projection_matrix * vec4(v_pos, 1.0);\n" // musime zapsat pozici
		//"	 v_color = vec3(abs(sin(v_pos.x)), abs(sin(v_pos.y)), abs(sin(v_pos.z)) );\n"
		"    v_color = v_col;\n"
		/*
		"    v_texcoord = v_tex;\n" // zkopirujeme souradnice textury pro fragment shader
		"    v_texcoord1 = v_tex + vec2(sin(f_time), f_time);\n"
		"    v_texcoord2 = v_tex + vec2(0.0, f_time * 1.5 + 1.0);\n"
		"    v_texcoord3 = v_tex + vec2(sin(f_time + 1.0), f_time + 2.0);\n" // spocitame dalsi souradnice textury pro animaci ohne
		*/
		"}\n";


	const char *p_s_fragment_shader =
		"#version 330\n"
		"in vec3 v_color;\n" // vstupy z vertex shaderu
		"\n"
		"out vec4 frag_color;\n" // vystup do framebufferu
		"\n"
		"uniform sampler2D n_box_tex, n_fire_tex;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    frag_color = vec4(v_color, 1.0f);\n"
		/*
		"    vec4 box_color = texture2D(n_box_tex, v_texcoord);\n" // precte texturu krabice
		"    vec4 fire_color = texture2D(n_fire_tex, v_texcoord1) +\n"
		"        texture2D(n_fire_tex, v_texcoord2) +\n"
		"        texture2D(n_fire_tex, v_texcoord3);\n" // precte tri vrstvy textury ohne (animace vznika vypoctem souradnic textury ve vertex shaderu)
		"    frag_color = mix(fire_color, box_color, box_color.a);\n" // smicha texturu a krabici podle alfa kanalu krabice (je tam vyznacene co ma byt pruhledne a co ne)
		*/
		"}\n";
	// zdrojove kody pro vertex / fragment shader (OpenGL 3 uz nepouziva specifikator 'verying', jinak je kod stejny)


	// zkompiluje vertex / fragment shader, pripoji je k programu
	n_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(n_vertex_shader, 1, &p_s_vertex_shader, NULL);
	glCompileShader(n_vertex_shader);
	if(!CheckShader(n_vertex_shader, "vertex shader"))
		return false;
	n_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(n_fragment_shader, 1, &p_s_fragment_shader, NULL);
	glCompileShader(n_fragment_shader);
	if(!CheckShader(n_fragment_shader, "fragment shader"))
		return false;
	n_program_object = glCreateProgram();
	glAttachShader(n_program_object, n_vertex_shader);
	glAttachShader(n_program_object, n_fragment_shader);
	
	// nabinduje atributy (propojeni mezi obsahem VBO a vstupem do vertex shaderu)
	glBindAttribLocation(n_program_object, 0, "v_pos");
	glBindAttribLocation(n_program_object, 1, "v_col");
	
	// nabinduje vystupni promenou (propojeni mezi framebufferem a vystupem fragment shaderu)
	glBindFragDataLocation(n_program_object, 0, "frag_color");
	
	// slinkuje program
	glLinkProgram(n_program_object);
	if(!CheckProgram(n_program_object, "program"))
		return false;
	
	// najde cislo parametru shaderu podle jeho jmena
	n_mvp_matrix_uniform = glGetUniformLocation(n_program_object, "t_modelview_projection_matrix");
	//n_box_sampler_uniform = glGetUniformLocation(n_program_object, "n_box_tex");
	//n_fire_sampler_uniform = glGetUniformLocation(n_program_object, "n_fire_tex");
	n_time_uniform = glGetUniformLocation(n_program_object, "f_time");
	
	/*
	// nahraje obrazky
	TBmp *p_box, *p_fire;
	if(!(p_box = CTgaCodec::p_Load_TGA("box.tga"))) {
		fprintf(stderr, "error: failed to load \'box.tga\'\n");
		return false;
	}
	if(!(p_fire = CTgaCodec::p_Load_TGA("fire.tga"))) {
		p_box->Delete();
		fprintf(stderr, "error: failed to load \'fire.tga\'\n");
		return false;
	}
	
	// vyrobi z nich textury
	n_box_texture = n_GLTextureFromBitmap(p_box, GL_RGBA8);
	n_fire_texture = n_GLTextureFromBitmap(p_fire, GL_RGB8);
	
	// uvolni pamet
	p_box->Delete();
	p_fire->Delete();
	*/

	return true;
}

/**
 *	@brief uvolni vsechny OpenGL objekty
 */
void CleanupGLObjects()
{
	glDeleteBuffers(1, &n_vertex_buffer_object);
	glDeleteBuffers(1, &n_index_buffer_object);
	// smaze vertex buffer objekty

	glDeleteShader(n_vertex_shader);
	glDeleteShader(n_fragment_shader);
	glDeleteProgram(n_program_object);
	// smaze shadery

	//glDeleteTextures(1, &n_box_texture);
	//glDeleteTextures(1, &n_fire_texture);
	// smaze textury
}

/**
 *	@brief vykresli geometrii sceny (bez specifikace cehokoliv jineho nez geometrie)
 */
void DrawScene() {
	
	// rozdelit patche na intervaly: from - prvni patch v intervalu; to - patch za poslednim patchem v intervalu
	vector<interval> intervals;
	int patchCount = scene.getIndicesCount() / 6;
	int colorRange = getColorRange();
	int divided = 0;

	do {
		int from = divided;
		int to = divided + colorRange;
		if (to > patchCount)
			to = patchCount;

		interval intv = { from, to };
		intervals.push_back(intv);
		divided = to;
	} while (divided < patchCount);
	
	
	int* indices = scene.getIndices();
	float* vertices = scene.getVertices();

	if (step > 0) { // globalni - pro moznost prepinani
	
		// for each interval (   <from; to)   )
		unsigned int i = step % intervals.size();
		{
		//for (unsigned int i = 0; i < intervals.size(); i++) {	

			glBindVertexArray(n_vertex_array_object);					
			glVertexAttrib3f(1, 0.1f, 0.1f, 0.1f); // vse cerne
			
			// vykreslit cerne vse pred aktivnim intervalem
			if (i > 0) {
				int fromIndex = 0;
				int count = 6 * intervals[i-1].to; // kreslit indexy od 0 az po posledni pred aktivnim intervalem
				glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, p_OffsetInVBO( fromIndex * sizeof(int) ));
			}

			
			// vykresli cerne vse za aktivnim intervalem
			if (i < intervals.size()-1) {
				int fromIndex = 6 * intervals[i+1].from;
				int count = 6 * (intervals.back().to - intervals[i+1].from);
				glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, p_OffsetInVBO( fromIndex * sizeof(int) ));
			}
			
		
			// vykresli jeden interval s barevnymi patchi
			glBindVertexArray(n_color_array_object);		
				
			// spocitat offset a pocet - vzdy 6 vrcholu
			int fromIndex = 6 * intervals[i].from;
			int count = 6 * (intervals[i].to - intervals[i].from);			

			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, p_OffsetInVBO( fromIndex * sizeof(int) ));	
		}

	} else {
		// vykresli vse barevne (s moznym opakovanim barev)
		glBindVertexArray(n_color_array_object);								
		glDrawElements(GL_TRIANGLES, scene.getIndicesCount(), GL_UNSIGNED_INT, p_OffsetInVBO(0));	
	}


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
	// Vypis memory leaku po kazdem ukonceni aplikace
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );

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
	HWND h_wnd;
	h_wnd = CreateWindow(p_s_class_name, p_s_window_name, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, n_wnd_width, n_wnd_height, NULL, NULL, GetModuleHandle(NULL), NULL);
	ShowWindow(h_wnd, SW_SHOW);
	UpdateWindow(h_wnd);	

	// inicializuje OpenGL
	bool b_forward_compatible = true; // jestli ma byt OpenGL kontext dopredne kompatibilni (tzn. nepodporovat funkce "stareho" OpenGL); pokud vase GPU nepodporuje OpenGL 3.0 nebo vyssi, pak vam to s timto nepobezi
	CGL30Driver driver;
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
		fprintf(stderr, "error: failed to initialize OpenGL objects\n"); // neco se nepovedlo (chyba pri kompilaci shaderu / chyba pri nacitani obrazku textury)
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
				
				// e
				if (n_w_param == 0x45) {
					printf("barevny rozsah: %i\n", getColorRange());
					float* colors = getColors();
					for (int i = 0; i < getColorRange() * 9; i++) {
						printf("%f ", colors[i]);
						if (i % 3 == 2)
							printf("\n");
					}
					delete[] colors;
				}

				// f
				if (n_w_param == 0x46) {
					if (wireframe) {
						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
						wireframe = false;
					} else {
						glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
						wireframe = true;
					}
				}

				// x
				if (n_w_param == 0x58) {					
					step++;
					printf("step: %i\n", step);
				}

				// y
				if (n_w_param == 0x59) {
					step--;
					if (step < 0)
						step = 0;
					printf("step: %i\n", step);
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

	// vysledkem jsou slozky vektoru ve smerech X ("strafe", ne otaceni) a Z
	float x = -( (-1.0f * a_down) + (1.0f * d_down) ) * keys_sensitivity;	
	float z = ( (-1.0f * s_down) + (1.0f * w_down) ) * keys_sensitivity;		
	cam.Move(x, 0.0f, z);

	// R - reset kamery
	if (HIBYTE(GetKeyState(0x52)) & 0x01)
		cam.Reset();

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
	// vycistime framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	// cas pro animaci
	float f_time = clock() / 1000.0f;
	
	
	// spustit vsechny akce spojene se stiskem klaves
	// napriklad posun kamery
	handleActiveKeys();
	

	// spocitame modelview - projection matici, kterou potrebujeme k transformaci vrcholu
	Matrix4f t_mvp;
	{
		// matice perspektivni projekce
		Matrix4f t_projection;
		CGLTransform::Perspective(t_projection, 90, float(n_width) / n_height, .01f, 1000);		
		
		// modelview
		Matrix4f t_modelview;
		t_modelview.Identity();				

		// vynasobit pohledem kamery
		t_modelview *= cam.GetMatrix();

		// matice pohledu kamery
		t_mvp = t_projection * t_modelview;
	}
	
	// pouzije shader, ktery jsme zkompilovali
	glUseProgram(n_program_object);
	
	// nastavime parametry shaderu (musi se delat pokazde kdyz se shader pouzije)
	// uniformni parametry se mohou prubezne menit
	{
		// nahrajeme matici do OpenGL jako parametr shaderu
		glUniformMatrix4fv(n_mvp_matrix_uniform, 1, GL_FALSE, &t_mvp[0][0]);
		
		// nastavime cisla texturovacich jednotek, odkud si shader bude brat textury
		//glUniform1i(n_box_sampler_uniform, 0);
		//glUniform1i(n_fire_sampler_uniform, 1);
		
		// nastavime cas pro animaci
		glUniform1f(n_time_uniform, f_time);		
	}
	
	/*
	// nastavime textury (do prvni texturovaci jednotky da texturu krabice, do druhou da texturu ohne)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, n_box_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, n_fire_texture); // opet - zadne enable / disable GL_TEXTURE_2D
	*/

	// vykreslime objekt pomoci VBO
	DrawScene();
	
	// preklopi buffery, zobrazi co jsme nakreslili
	driver.Blit(); 
}


/**
 * Vraci pole floatu kde kazde tri hodnoty reprezentuji jednu barvu
 * Pocet barev odpovida getColorRange(), hodnot je colorRange * 3 (barvy)
 */
float* getColors() {	
	int range = getColorRange();
	float* colors = new float[range * 3]; // rgb = 3 hodnoty

	//printf("color range: %i\n", range);

	int i = 0;
	for (float r = 0.0f; r <= 1.0f; r += 0.5f) {
		for (float g = 0.0f; g <= 1.0f; g += 0.5f) {
			for (float b = 0.0f; b <= 1.0f; b += 0.5f) {
				if (r != 0.0f || g != 0.0f || b != 0.0f) {																			
					colors[i] = r;
					colors[i+1] = g;
					colors[i+2] = b;
					i += 3;										
				}
			}
		}
	}	

	/*
	for (int i = 0; i < range * 3; i++) {
		cout << colors[i] << " ";
		if (i % 3 == 2)
			cout << endl;
	}
	*/

	return colors;
}

/**
 * Vraci pocet unikatnich barev snizeny o jednicku (cernou nepocitame), ktery lze zobrazit
 */
int getColorRange() {	
	
	/* vraci chybu INVALID_ENUM - zrejme deprecated
		GLint redBits, greenBits, blueBits;

		glGetIntegerv (GL_RED_BITS, &redBits);
		glGetIntegerv (GL_GREEN_BITS, &greenBits);
		glGetIntegerv (GL_BLUE_BITS, &blueBits);

		printf("r: %d\n", redBits);
		printf("g: %d\n", greenBits);
		printf("b: %d\n", blueBits);			
	*/	
	
	// predpokladat 8b

	// barvy 0.0, 0.5, 1.0	
	return 26;
}

