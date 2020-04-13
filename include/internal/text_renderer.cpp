#include <internal/text_renderer.h>

void render_text(GLFWwindow* window, GLuint vertex_buffer, GLuint uv_buffer, GLuint font_texture, GLuint shader_id, int char_width, int char_height, int x, int y, int width, int height, std::string text)
{
	glBindTexture(GL_TEXTURE_2D, font_texture);
	// dimensions of texture
	int tex_width, tex_height;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_height);

	std::vector<glm::vec2> verts = std::vector<glm::vec2>();
	std::vector<glm::vec2> uvs = std::vector<glm::vec2>();

	for (char& c : text)
	{
		// calculate quad corner positions
		verts.push_back(glm::vec2(x, y));
		verts.push_back(glm::vec2(x + width, y));
		verts.push_back(glm::vec2(x, y + height));

		verts.push_back(glm::vec2(x + width, y + height));
		verts.push_back(glm::vec2(x + width, y));
		verts.push_back(glm::vec2(x, y + height));

		// calculate position of the current letter in the font texture
		int char_x = (c % (tex_width / char_width)) * char_width;
		int char_y = (c / (tex_width / char_width)) * char_width;

		// calculate uv positions for each corner of the quad
		uvs.push_back(glm::vec2(char_x, tex_height - char_y));
		uvs.push_back(glm::vec2(char_x + char_width, tex_height - char_y));
		uvs.push_back(glm::vec2(char_x, tex_height - char_y - char_height));

		uvs.push_back(glm::vec2(char_x + char_width, tex_height - char_y - char_height));
		uvs.push_back(glm::vec2(char_x + char_width, tex_height - char_y));
		uvs.push_back(glm::vec2(char_x, tex_height - char_y - char_height));
	}

	glDisable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	//glDisable(GL_DEPTH_TEST);

	glUseProgram(shader_id);

	// bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glUniform1i(glGetUniformLocation(shader_id, "texture_sampler"), 0);

	float d[2];
	glfwGetWindowSize(window, (int*)&d[0], (int*)&d[1]);
	glUniform2fv(glGetUniformLocation(shader_id, "screen_dimensions"), 1, &d[0]);

	// put the data in the buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec2), &verts[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	// render the letter
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, verts.size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glEnable(GL_CULL_FACE);
	//glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}