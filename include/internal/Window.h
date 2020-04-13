#pragma once

#include <string.h>
#include <vector>
#include <stdio.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include <internal/shader_loader.h>
#include <internal//texture_loader.h>

/*
This is a wrapper class to handle window-related things, mostly rendering
All OpenGL-related stuff is handled inside this class, including buffers, shaders and textures
*/
class Window
{
private:
	GLFWwindow* window; // GLFW window where images are to be drawn
	GLuint solid_color_shader_id, texture_shader_id;
	GLuint vertex_buffer[2], color_buffer[2], normal_buffer[2], text_vertex_buffer[2], uv_buffer[2];
	GLuint font;

	bool current_buffer, current_text_buffer;

	std::vector<glm::vec3> vertices, colors, normals;
	std::vector<glm::vec2> text_vertices, uvs;

	int font_width, font_height;
	int char_width, char_height;

public:
	Window();
	Window(GLFWwindow* w, const char * solid_color_vert_shader_path, const char * solid_color_frag_shader_path, const char * texture_vert_shader_path, const char * texture_frag_shader_path, const char * font_path);
	~Window();

	// draw triangles in 3d space, used to render level geometry
	void load_tris(std::vector<glm::vec3> verts, std::vector<glm::vec3> colors, std::vector<glm::vec3> normals);
	void draw_tris();

	// draw text on screen, used for guis
	void load_text(std::string text, int x, int y, int width, int height);
	void draw_text();
};

