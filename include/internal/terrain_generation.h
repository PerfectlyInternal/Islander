#ifndef TERRAIN_GENERATION_DEF
#define TERRAIN_GENRATION_DEF

#include <time.h>
#include <vector>
#include <stdio.h>
#include <thread>
#include <glm/glm.hpp>

double random();
int round_down(int n, int m);
double bilinear_interpolation(double v1, double v2, double v3, double v4, double x1, double x2, double y1, double y2, double x, double y);
std::vector<std::vector<double>> bicubic_interpolation(std::vector<std::vector<double>> h, int gap);
void noise(int size, std::vector<std::vector<double>>& map, double amplitude, int frequency);
void generate_terrain(int size, int iterations, double amplitude, std::vector<std::vector<glm::vec3>>& vertices, std::vector<glm::vec3>& colors, std::vector<glm::vec3>& normals, int& completion);

#endif // !TERRAIN_GENERATION_DEF

//#define max(a, b) a > b ? a : b
//#define min(a, b) a < b ? a : b

#define INIT_VALUE 0