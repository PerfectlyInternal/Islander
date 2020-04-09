#pragma once

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
class model
{
private:
	std::vector<glm::vec3> vertices = std::vector<glm::vec3>();
	std::vector<glm::vec3> colors = std::vector<glm::vec3>();
	std::vector<glm::vec3> normals = std::vector<glm::vec3>();

public:
	model();
	model(const model& m);
	~model();

	bool load_model(const char * obj_path, const char * mtl_path);
	void translate(double x, double y, double z);
	void get_model(std::vector<glm::vec3>& out_verts, std::vector<glm::vec3>& out_colors, std::vector<glm::vec3>& out_normals);
};

