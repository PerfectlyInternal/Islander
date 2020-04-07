#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

void loading_screen(GLFWwindow* window, int& percentage, GLuint program_id, glm::vec3 background, glm::vec3 progress_bar, glm::vec3 progress_bar_border);