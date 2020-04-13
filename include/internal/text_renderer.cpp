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

	int pos = 0;
	for (char& c : text)
	{
		// calculate quad corner positions
		verts.push_back(glm::vec2(x + pos * width, y + height));
		verts.push_back(glm::vec2(x + pos * width, y));
		verts.push_back(glm::vec2(x + + width + pos * width, y + height));

		verts.push_back(glm::vec2(x + width + pos * width, y));
		verts.push_back(glm::vec2(x + width + pos * width, y + height));
		verts.push_back(glm::vec2(x + pos * width, y));

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

	glDisable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	//glDisable(GL_DEPTH_TEST);

	glUseProgram(shader_id);

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader_id, "texture_sampler"), 0);

	int d[2];
	glfwGetWindowSize(window, &d[0], &d[1]);
	glUniform2iv(glGetUniformLocation(shader_id, "screen_dimensions"), 1, &d[0]);

	// put the data in the buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec2), &verts[0], GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STREAM_DRAW);

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