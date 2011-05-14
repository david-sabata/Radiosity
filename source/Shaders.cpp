#include "Shaders.h"


GLuint Shaders::n_user_vertex_shader = 0;
GLuint Shaders::n_user_fragment_shader = 0;
GLuint Shaders::n_user_program_object = 0;

GLuint Shaders::n_patch_vertex_shader = 0;
GLuint Shaders::n_patch_fragment_shader = 0;
GLuint Shaders::n_patch_program_object = 0;

GLuint Shaders::n_preview_vertex_shader = 0;
GLuint Shaders::n_preview_fragment_shader = 0;
GLuint Shaders::n_preview_program_object = 0;

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

		if (!b_compiled)
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

		if (!b_linked)
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

	glDeleteShader(n_patch_vertex_shader);
	glDeleteShader(n_patch_fragment_shader);
	glDeleteProgram(n_patch_program_object);

	glDeleteShader(n_preview_vertex_shader);
	glDeleteShader(n_preview_fragment_shader);
	glDeleteProgram(n_preview_program_object);
}



/**
 * Vraci program (zkompilovane shadery) pro renderovani nahledoveho krize
 */
GLuint Shaders::getPreviewProgram() {
	const char *p_s_vertex_shader =
		"#version 330\n"		
		"in vec3 v_pos;\n" // atributy - vstup z dat vrcholu
		"in vec2 v_tex;\n" // souradnice textury
		"\n"
		"uniform mat4 t_modelview_projection_matrix;\n" // parametr shaderu - transformacni matice
		"\n"
		"out vec2 v_texcoord, v_normal_tex;\n" 		
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = t_modelview_projection_matrix * vec4(v_pos, 1.0);\n" // musime zapsat pozici
		"	 v_texcoord = v_tex;\n"
		"    v_normal_tex = v_tex * 2.0 - 1.0;\n"		
		"}\n";


	const char *p_s_fragment_shader =
		"#version 330\n"
		"in vec2 v_texcoord, v_normal_tex;\n"
		"\n"
		"out vec4 frag_color;\n" // vystup do framebufferu
		"\n"
		"uniform sampler2D n_tex;\n"
		"\n"
		"void main()\n"
		"{\n"			
		/*
		"    vec4 tex_color = vec4(0.0);"
		"    if(v_texcoord.y >= 0.25 && v_texcoord.y <= 0.75) {"
		"        if(v_texcoord.x < 0.25)"
		"            tex_color = texture(n_tex, (v_texcoord + vec2(0, -.25)) * vec2(0.25/0.25, 0.66667/0.5));"
		"        else if(v_texcoord.x < 0.75)"
		"            tex_color = texture(n_tex, (v_texcoord + vec2(0, -.25)) * vec2(0.5/0.5, 0.66667/0.5));"
		"        else"
		"            tex_color = texture(n_tex, (v_texcoord + vec2(0, -.25)) * vec2(0.25/0.25, 0.66667/0.5));"
		"    } else if(v_texcoord.y < 0.25 && v_texcoord.x >= 0.25 && v_texcoord.x <= 0.75)"
		"        tex_color = texture(n_tex, (v_texcoord + vec2(-.25, 0)) * vec2(0.5/0.5, 0.33333/0.25) + vec2(0.5, 0.66667));"
		"    else if(v_texcoord.y > 0.75 && v_texcoord.x >= 0.25 && v_texcoord.x <= 0.75)"
		"        tex_color = texture(n_tex, (v_texcoord + vec2(-0.25, -.75)) * vec2(0.5/0.5, 0.33333/0.25) + vec2(0.0, 0.66667));"
		"    else discard;"	
		*/
		"    vec4 tex_color = texture(n_tex, v_texcoord);\n"
		/*		
		"    vec4 tex_color = texture(n_tex, v_texcoord) * .5;//vec4(0.0);\n" // precte texturu krabice
		"    float f_tex_length = length(v_normal_tex);\n"
		"    vec3 v_ray = vec3(v_normal_tex, sqrt(1 - f_tex_length));\n"
		"    if(f_tex_length > 1.0)\n"
		"        discard;\n"
		"    vec3 v_ray_abs = abs(v_ray);\n"
		"    if(v_ray.z > v_ray_abs.x && v_ray.z > v_ray_abs.y)\n" // dopredu
		"        tex_color = texture(n_tex, ((v_ray.xy / v_ray.z) * .5 + .5) * vec2(0.5, 0.66667) + vec2(0.25, 0.0));\n"
		"    else if(v_ray_abs.x > v_ray_abs.y) {\n"
		"        if(v_ray.x < 0)\n" // vlevo
		"            tex_color = texture(n_tex, ((v_ray.yz / v_ray.x) * vec2(0.5, 0.5) + vec2(0.5, 0.5)) * vec2(-0.25, -0.66667) + vec2(0.25, 0.66667));\n"
		"        else\n" // vpravo
		"            tex_color = texture(n_tex, ((v_ray.yz / v_ray.x) * vec2(-0.5, 0.5) + vec2(0.5, 0.5)) * vec2(0.25, 0.66667) + vec2(0.75, 0.0));\n"
		"    } else {\n"
		"        if(v_ray.y < 0)\n" // nahoru
		"            tex_color = texture(n_tex, ((v_ray.xz / v_ray.y) * vec2(0.5, 0.5) + vec2(0.5, 0.5)) * vec2(-0.5, -0.33333) + vec2(1.0, 1.0));\n"
		"        else\n" // dolu
		"            tex_color = texture(n_tex, ((v_ray.xz / v_ray.y) * vec2(0.5, -0.5) + vec2(0.5, 0.5)) * vec2(0.5, 0.33333) + vec2(0, 0.66667));\n"
		"    }\n"
		*/
		"    frag_color = tex_color;\n"
		"}\n";


	// zkompiluje vertex / fragment shader, pripoji je k programu
	n_preview_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(n_preview_vertex_shader, 1, &p_s_vertex_shader, NULL);
	glCompileShader(n_preview_vertex_shader);
	if(!CheckShader(n_preview_vertex_shader, "vertex shader"))
		return false;

	n_preview_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(n_preview_fragment_shader, 1, &p_s_fragment_shader, NULL);
	glCompileShader(n_preview_fragment_shader);
	if(!CheckShader(n_preview_fragment_shader, "fragment shader"))
		return false;
	
	n_preview_program_object = glCreateProgram();
	glAttachShader(n_preview_program_object, n_preview_vertex_shader);
	glAttachShader(n_preview_program_object, n_preview_fragment_shader);
	
	// nabinduje atributy (propojeni mezi obsahem VBO a vstupem do vertex shaderu)
	glBindAttribLocation(n_preview_program_object, 0, "v_pos");
	glBindAttribLocation(n_preview_program_object, 2, "v_tex");
	
	// nabinduje vystupni promenou (propojeni mezi framebufferem a vystupem fragment shaderu)
	glBindFragDataLocation(n_preview_program_object, 0, "frag_color");
	
	// slinkuje program
	glLinkProgram(n_preview_program_object);
	if(!CheckProgram(n_preview_program_object, "program"))
		return false;
	else
		return n_preview_program_object;
}



/**
 * Vraci program (zkompilovane shadery) pro renderovani pohledu z jednotlivych patchu
 */
GLuint Shaders::getPatchViewProgram() {
	const char *p_s_vertex_shader =
		"#version 330\n"		
		"in vec3 v_pos;\n" // atributy - vstup z dat vrcholu
		"in vec3 v_col;\n" // barva vrcholu
		"\n"
		"uniform mat4 t_modelview_projection_matrix;\n" // parametr shaderu - transformacni matice
		"\n"
		"out vec3 v_color;\n"		
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = t_modelview_projection_matrix * vec4(v_pos, 1.0);\n" // musime zapsat pozici
		"    v_color = v_col / 1024.0;\n"
		"}\n";


	const char *p_s_fragment_shader =
		"#version 330\n"
		"in vec3 v_color;\n" // vstupy z vertex shaderu
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
	
	// nabinduje vystupni promenou (propojeni mezi framebufferem a vystupem fragment shaderu)
	glBindFragDataLocation(n_patch_program_object, 0, "frag_color");
	
	// slinkuje program
	glLinkProgram(n_patch_program_object);
	if(!CheckProgram(n_patch_program_object, "program"))
		return false;
	else
		return n_patch_program_object;
}



/**
 * Vraci program (zkompilovane shadery) pro renderovani uzivatelskeho pohledu
 */
GLuint Shaders::getUserViewProgram() {
	const char *p_s_vertex_shader =
		"#version 330\n"		
		"in vec3 v_pos;\n" // atributy - vstup z dat vrcholu
		"in vec3 v_col;\n" // barva vrcholu
		"\n"
		"uniform mat4 t_modelview_projection_matrix;\n" // parametr shaderu - transformacni matice
		"\n"
		"out vec3 v_color;\n"		
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = t_modelview_projection_matrix * vec4(v_pos, 1.0);\n" // musime zapsat pozici
		"    v_color = v_col;\n"
		"}\n";


	const char *p_s_fragment_shader =
		"#version 330\n"
		"in vec3 v_color;\n" // vstupy z vertex shaderu
		"\n"
		"out vec4 frag_color;\n" // vystup do framebufferu
		"\n"
		"void main()\n"
		"{\n"
		"    frag_color = vec4(v_color, 1.0f);\n"
		"}\n";


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
	
	// nabinduje vystupni promenou (propojeni mezi framebufferem a vystupem fragment shaderu)
	glBindFragDataLocation(n_user_program_object, 0, "frag_color");
	
	// slinkuje program
	glLinkProgram(n_user_program_object);
	if(!CheckProgram(n_user_program_object, "program"))
		return false;
	else
		return n_user_program_object;
}