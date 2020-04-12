#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint load_bmp(const char* image_path);
GLuint load_dds(const char* image_path);

#endif // !TEXTURE_LOADER_H