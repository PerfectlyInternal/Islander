#include "Window.h"

Window::Window()
{
	printf("unsupported feature");
}

Window::Window(GLFWwindow* w, const char * solid_color_vert_shader_path, const char * solid_color_frag_shader_path, const char * texture_vert_shader_path, const char * texture_frag_shader_path, const char * font_path)
{
	// main window handle
	window = w;

	// load shaders from file
	solid_color_shader_id = load_shaders(solid_color_vert_shader_path, solid_color_frag_shader_path);
	texture_shader_id = load_shaders(texture_vert_shader_path, texture_frag_shader_path);

	// load font from file
	font = load_dds(font_path);

	// generate buffers
	glGenBuffers(2, &vertex_buffer[0]);
	glGenBuffers(2, &color_buffer[0]);
	glGenBuffers(2, &normal_buffer[0]);
	glGenBuffers(2, &uv_buffer[0]);

	current_buffer = 0;

	// enable Vsync
	glfwSwapInterval(1);

	// enable a bunch of OpenGL flags
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glEnable(GL_MULTISAMPLE);

	// get the font dimensions
	glBindTexture(GL_TEXTURE_2D, font);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &font_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &font_height);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::load_tris(std::vector<glm::vec3> verts, std::vector<glm::vec3> colors, std::vector<glm::vec3> normals)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[current_buffer]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &verts[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, color_buffer[current_buffer]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer[current_buffer]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_DYNAMIC_DRAW);
}

void Window::draw_tris()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[!current_buffer]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer[!current_buffer]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer[!current_buffer]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glfwSwapBuffers(window);

	current_buffer = !current_buffer;
}

void Window::load_text(std::string text, int x, int y, int width, int height)
{
	text_vertices = std::vector<glm::vec2>();
	uvs = std::vector<glm::vec2>();

	int pos = 0;
	for (char& c : text)
	{
		// calculate quad corner positions
		text_vertices.push_back(glm::vec2(x + pos * width, y + height));
		text_vertices.push_back(glm::vec2(x + pos * width, y));
		text_vertices.push_back(glm::vec2(x + +width + pos * width, y + height));

		text_vertices.push_back(glm::vec2(x + width + pos * width, y));
		text_vertices.push_back(glm::vec2(x + width + pos * width, y + height));
		text_vertices.push_back(glm::vec2(x + pos * width, y));

		// calculate position of the current letter in the font texture
		float uv_x = (c % 16) / 16.0f;
		float uv_y = (c / 16) / 16.0f;

		glm::vec2 uv_up_left = glm::vec2(uv_x, uv_y);
		glm::vec2 uv_up_right = glm::vec2(uv_x + 1.0f / 16.0f, uv_y);
		glm::vec2 uv_down_right = glm::vec2(uv_x + 1.0f / 16.0f, (uv_y + 1.0f / 16.0f));
		glm::vec2 uv_down_left = glm::vec2(uv_x, (uv_y + 1.0f / 16.0f));

		uvs.push_back(uv_up_left);
		uvs.push_back(uv_down_left);
		uvs.push_back(uv_up_right);

		uvs.push_back(uv_down_right);
		uvs.push_back(uv_up_right);
		uvs.push_back(uv_down_left);

		pos++;
	}

	// put the data in the buffers
	glBindBuffer(GL_ARRAY_BUFFER, text_vertex_buffer[current_buffer]);
	glBufferData(GL_ARRAY_BUFFER, text_vertices.size() * sizeof(glm::vec2), &text_vertices[0], GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer[current_buffer]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STREAM_DRAW);
}

void Window::draw_text()
{
	// render the letter
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, text_vertex_buffer[!current_buffer]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer[!current_buffer]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDisable(GL_CULL_FACE);

	glUseProgram(texture_shader_id);

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, font);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(texture_shader_id, "texture_sampler"), 0);

	int d[2];
	glfwGetWindowSize(window, &d[0], &d[1]);
	glUniform2iv(glGetUniformLocation(texture_shader_id, "screen_dimensions"), 1, &d[0]);

	glDrawArrays(GL_TRIANGLES, 0, text_vertices.size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glEnable(GL_CULL_FACE);

	glfwSwapBuffers(window);

	current_text_buffer = !current_text_buffer;
}