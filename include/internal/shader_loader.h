#ifndef SHADER_LOADER_DEF
#define SHADER_LOADER_DEF

#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

GLuint load_shaders(const char* vertex_file_path, const char* fragment_file_path);

#endif // !SHADER_LOADER_DEF
