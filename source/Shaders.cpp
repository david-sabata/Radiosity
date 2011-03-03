#include "Shaders.h"


GLuint Shaders::n_user_vertex_shader = 0;
GLuint Shaders::n_user_fragment_shader = 0;
GLuint Shaders::n_user_program_object = 0;

GLuint Shaders::n_patch_vertex_shader = 0;
GLuint Shaders::n_patch_fragment_shader = 0;
GLuint Shaders::n_patch_program_object = 0;

/**
 *	@brief zkontroluje zda byl shader spravne zkompilovan, tiskne chyby do stderr
 *	@param[in] n_shader_object je id shaderu
 *	@param[in] p_s_shader_name je jmeno shaderu (jen pro uzivatele - aby vedel o ktery shader se jedna pokud se vytiskne nejaka chyba)
 *	@return vraci true pro uspesne zkompilovane shadery, jinak false
 */
bool Shaders::CheckShader(GLuint n_shader_object, const char *p_s_shader_name)
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
bool Shaders::CheckProgram(GLuint n_program_object, const char *p_s_program_name)
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
 * Uvolni z pameti shadery a slinkovane programy
 */
void Shaders::cleanup() {
	glDeleteShader(n_user_vertex_shader);
	glDeleteShader(n_user_fragment_shader);
	glDeleteProgram(n_user_program_object);
}



/**
 * Vraci program (zkompilovane shadery) pro renderovani uzivatelskeho pohledu
 */
GLuint Shaders::getUserViewProgram() {
	const char *p_s_vertex_shader =
		"#version 330\n"		
		"in vec3 v_pos;\n" // atributy - vstup z dat vrcholu
		"in vec3 v_col;\n" // barva vrcholu
		"in vec2 v_tex;\n" // souradnice textury
		"\n"
		"uniform float f_time;\n" // parametr shaderu - cas pro animovani
		"\n"
		"uniform mat4 t_modelview_projection_matrix;\n" // parametr shaderu - transformacni matice
		"\n"
		"out vec3 v_color;\n"		
		"out vec2 v_texcoord;\n" 		
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = t_modelview_projection_matrix * vec4(v_pos, 1.0);\n" // musime zapsat pozici
		"    v_color = v_col;\n"
		"	 v_texcoord = v_tex;\n"		
		"}\n";


	const char *p_s_fragment_shader =
		"#version 330\n"
		"in vec3 v_color;\n" // vstupy z vertex shaderu
		"in vec2 v_texcoord;\n"
		"\n"
		"out vec4 frag_color;\n" // vystup do framebufferu
		"\n"
		"uniform sampler2D n_box_tex;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    frag_color = vec4(v_color, 1.0f);\n"
		"    vec4 box_color = texture(n_box_tex, v_texcoord);\n" // precte texturu krabice		
		//"	   frag_color = box_color;\n"
		"    frag_color = mix(frag_color, box_color, 0.5);\n" // smicha texturu a krabici podle alfa kanalu krabice (je tam vyznacene co ma byt pruhledne a co ne)
		"}\n";
	// zdrojove kody pro vertex / fragment shader (OpenGL 3 uz nepouziva specifikator 'varying', jinak je kod stejny)


	// zkompiluje vertex / fragment shader, pripoji je k programu
	n_user_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(n_user_vertex_shader, 1, &p_s_vertex_shader, NULL);
	glCompileShader(n_user_vertex_shader);
	if(!CheckShader(n_user_vertex_shader, "vertex shader"))
		return false;

	n_user_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(n_user_fragment_shader, 1, &p_s_fragment_shader, NULL);
	glCompileShader(n_user_fragment_shader);
	if(!CheckShader(n_user_fragment_shader, "fragment shader"))
		return false;
	
	n_user_program_object = glCreateProgram();
	glAttachShader(n_user_program_object, n_user_vertex_shader);
	glAttachShader(n_user_program_object, n_user_fragment_shader);
	
	// nabinduje atributy (propojeni mezi obsahem VBO a vstupem do vertex shaderu)
	glBindAttribLocation(n_user_program_object, 0, "v_pos");
	glBindAttribLocation(n_user_program_object, 1, "v_col");
	glBindAttribLocation(n_user_program_object, 2, "v_tex");
	
	// nabinduje vystupni promenou (propojeni mezi framebufferem a vystupem fragment shaderu)
	glBindFragDataLocation(n_user_program_object, 0, "frag_color");
	
	// slinkuje program
	glLinkProgram(n_user_program_object);
	if(!CheckProgram(n_user_program_object, "program"))
		return false;
	else
		return n_user_program_object;
}



/**
 * Vraci program (zkompilovane shadery) pro renderovani pohledu z jednotlivych patchu
 */
GLuint Shaders::getPatchViewProgram() {
	const char *p_s_vertex_shader =
		"#version 330\n"		
		"in vec3 v_pos;\n" // atributy - vstup z dat vrcholu
		"in vec3 v_col;\n" // barva vrcholu
		"in vec2 v_tex;\n" // souradnice textury		
		"\n"
		"uniform mat4 t_modelview_projection_matrix;\n" // parametr shaderu - transformacni matice
		"\n"
		"out vec3 v_color;\n"		
		"out vec2 v_texcoord;\n" 		
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = t_modelview_projection_matrix * vec4(v_pos, 1.0);\n" // musime zapsat pozici
		"    v_color = v_col;\n"
		"	 v_texcoord = v_tex;\n"		
		"}\n";


	const char *p_s_fragment_shader =
		"#version 330\n"
		"in vec3 v_color;\n" // vstupy z vertex shaderu
		"in vec2 v_texcoord;\n"
		"\n"
		"out vec4 frag_color;\n" // vystup do framebufferu
		"\n"
		"void main()\n"
		"{\n"
		"    frag_color = vec4(v_color, 1.0f);\n"
		"}\n";


	// zkompiluje vertex / fragment shader, pripoji je k programu
	n_patch_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(n_patch_vertex_shader, 1, &p_s_vertex_shader, NULL);
	glCompileShader(n_patch_vertex_shader);
	if(!CheckShader(n_patch_vertex_shader, "vertex shader"))
		return false;

	n_patch_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(n_patch_fragment_shader, 1, &p_s_fragment_shader, NULL);
	glCompileShader(n_patch_fragment_shader);
	if(!CheckShader(n_patch_fragment_shader, "fragment shader"))
		return false;
	
	n_patch_program_object = glCreateProgram();
	glAttachShader(n_patch_program_object, n_patch_vertex_shader);
	glAttachShader(n_patch_program_object, n_patch_fragment_shader);
	
	// nabinduje atributy (propojeni mezi obsahem VBO a vstupem do vertex shaderu)
	glBindAttribLocation(n_patch_program_object, 0, "v_pos");
	glBindAttribLocation(n_patch_program_object, 1, "v_col");
	glBindAttribLocation(n_patch_program_object, 2, "v_tex");
	
	// nabinduje vystupni promenou (propojeni mezi framebufferem a vystupem fragment shaderu)
	glBindFragDataLocation(n_patch_program_object, 0, "frag_color");
	
	// slinkuje program
	glLinkProgram(n_patch_program_object);
	if(!CheckProgram(n_patch_program_object, "program"))
		return false;
	else
		return n_patch_program_object;
}