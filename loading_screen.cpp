#include <internal/loading_screen.h>

void loading_screen(GLFWwindow* window, int* percentage, GLuint program_id, glm::vec3 background, glm::vec3 progress_bar, glm::vec3 progress_bar_border)
{
	std::vector<glm::vec3> vertices(12, glm::vec3());
	std::vector<glm::vec3> colors(12, glm::vec3());
	std::vector<glm::vec3> normals(12, glm::vec3(1, 0, 0));

	glm::mat4 projection = glm::ortho(0, 100, 0, 100);
	glm::mat4 view = glm::lookAt(glm::vec3(1, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 final_matrix = projection * view * model;

	vertices[0] = glm::vec3(0, 0, 0);
	vertices[1] = glm::vec3(0, 0, 100);
	vertices[2] = glm::vec3(0, 100, 0);

	vertices[3] = glm::vec3(0, 100, 100);
	vertices[4] = glm::vec3(0, 100, 0);
	vertices[5] = glm::vec3(0, 0, 100);
	
	vertices[6] = glm::vec3(0, 0, 0);
	vertices[7] = glm::vec3(0, 0, *percentage);
	vertices[8] = glm::vec3(0, 100, 0);
	
	vertices[9] = glm::vec3(0, 0, *percentage);
	vertices[10] = glm::vec3(0, 100, *percentage);
	vertices[11] = glm::vec3(0, 100, 0);

	colors[0] = background;
	colors[1] = background;
	colors[2] = background;

	colors[3] = background;
	colors[4] = background;
	colors[5] = background;

	colors[6] = progress_bar;
	colors[7] = progress_bar;
	colors[8] = progress_bar;

	colors[9] = progress_bar;
	colors[10] = progress_bar;
	colors[11] = progress_bar;

	// buffers for position and color
	GLuint vertex_buffer;
	GLuint color_buffer;
	GLuint normal_buffer;

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	// handle for the matrices in the shaders
	GLuint matrix_id = glGetUniformLocation(program_id, "matrix");
	GLuint view_id = glGetUniformLocation(program_id, "view");
	GLuint model_id = glGetUniformLocation(program_id, "model");

	// ambient lighting
	glm::vec3 ambient_light_color = glm::vec3(0.2f, 0.2f, 0.2f);
	GLuint ambient_light_id = glGetUniformLocation(program_id, "ambient_light_color");

	while (*percentage != 100)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vertices[6] = glm::vec3(0, 0, 0);
		vertices[7] = glm::vec3(0, 0, *percentage);
		vertices[8] = glm::vec3(0, 100, 0);

		vertices[9] = glm::vec3(0, 0, *percentage);
		vertices[10] = glm::vec3(0, 100, *percentage);
		vertices[11] = glm::vec3(0, 100, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		// send stuff to shaders
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &final_matrix[0][0]);
		glUniformMatrix4fv(view_id, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(model_id, 1, GL_FALSE, &model[0][0]);	

		// draw stuff
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(
			0, // attribute No 0
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // is it normalized?
			0, // gap between groups of data
			(void*)0 // offset from start
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glVertexAttribPointer(
			1, // attribute No 0
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // is it normalized?
			0, // gap between groups of data
			(void*)0 // offset from start
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glVertexAttribPointer(
			2, // attribute No 0
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // is it normalized?
			0, // gap between groups of data
			(void*)0 // offset from start
		);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() * 3);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glfwSwapBuffers(window);
	}
}