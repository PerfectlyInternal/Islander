#pragma once

#include <vector>
#include <string.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

void render_text(GLFWwindow* window, GLuint vert_buffer, GLuint uv_buffer, GLuint font_texture, GLuint shader_id, int char_width, int char_height, int x, int y, int width, int height, std::string text);