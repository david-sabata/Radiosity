
#include <GL/glew.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class Shaders {
	public:
		static GLuint getUserViewProgram();
		static GLuint getWireframeProgram();
		static GLuint getPatchViewProgram();
		static GLuint getPreviewProgram();
		static void cleanup();

	private:
		static bool CheckShader(GLuint n_shader_object, const char *p_s_shader_name);
		static bool CheckProgram(GLuint n_program_object, const char *p_s_program_name);

		static GLuint n_user_vertex_shader, n_user_fragment_shader, n_wireframe_program_object;
		static GLuint n_wireframe_vertex_shader, n_wireframe_fragment_shader, n_user_program_object;
		static GLuint n_patch_vertex_shader, n_patch_fragment_shader, n_patch_program_object;
		static GLuint n_preview_vertex_shader, n_preview_fragment_shader, n_preview_program_object;
};

