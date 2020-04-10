#include <internal/model.h>

model::model()
{
}

model::model(const model& m)
{
	vertices = m.vertices;
	colors = m.colors;
	normals = m.normals;
}

model::~model()
{
}

bool model::load_model(const char * obj_path, const char * mtl_path)
{
	std::vector<unsigned int> vertex_indices, color_indices, normal_indices;
	std::vector<glm::vec3> temp_vertices, temp_colors, temp_normals;
	int color_index = -1;

	FILE * file;

	fopen_s(&file, obj_path, "r");
	if (file == NULL)
	{
		printf("could not open object file!\n");
		return false;
	}
	while (true)
	{
		char line_header[128];

		// read first word of line
		int res = fscanf_s(file, "%s", line_header, 128);
		if (res == EOF)
		{
			break;
		}
		if (strcmp(line_header, "v") == 0)
		{
			// vertex position
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(line_header, "vn") == 0)
		{
			// vertex normal
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(line_header, "f") == 0)
		{
			// face
			std::string vertex1, vertex2, vertex3;
			unsigned int vertex_index[3], normal_index[3];
			int matches = fscanf_s(file, "%d//%d %d//%d %d//%d\n", &vertex_index[0], &normal_index[0], &vertex_index[1], &normal_index[1], &vertex_index[2], &normal_index[2]);
			if (matches != 6) {
				printf("file can't be read.\n");
				return false;
			}
			vertex_indices.push_back(vertex_index[0]);
			vertex_indices.push_back(vertex_index[1]);
			vertex_indices.push_back(vertex_index[2]);
			normal_indices.push_back(normal_index[0]);
			normal_indices.push_back(normal_index[1]);
			normal_indices.push_back(normal_index[2]);

			if (color_index == -1) color_index++;

			color_indices.push_back(color_index);
			color_indices.push_back(color_index);
			color_indices.push_back(color_index);
		}
		else if (strcmp(line_header, "usemtl") == 0)
		{
			// using a new material/color
			color_index++;
		}
	}

	// process data
	for (unsigned int i = 0; i < vertex_indices.size(); i++)
	{
		unsigned int vertex_index = vertex_indices[i];
		glm::vec3 vertex = temp_vertices[vertex_index - 1];
		vertices.push_back(vertex);
	}
	for (unsigned int i = 0; i < normal_indices.size(); i++)
	{
		unsigned int normal_index = normal_indices[i];
		glm::vec3 normal = temp_normals[normal_index - 1];
		normals.push_back(normal);
	}

	// read the mtl file
	fopen_s(&file, mtl_path, "r");
	color_index = -1;
	if (file == NULL)
	{
		printf("could not open mtl file!\n");
		return false;
	}
	while (true)
	{
		char line_header[128];

		// read first word of line
		int res = fscanf_s(file, "%s", line_header, 128);
		if (res == EOF)
		{
			break;
		}
		if (strcmp(line_header, "newmtl") == 0)
		{
			color_index++;
		}
		else if (strcmp(line_header, "Kd") == 0)
		{
			glm::vec3 color;
			fscanf_s(file, "%f %f %f", &color.x, &color.y, &color.z);
			temp_colors.push_back(color);
		}
	}

	// process data
	for (unsigned int i = 0; i < color_indices.size(); i++)
	{
		color_index = color_indices[i];
		glm::vec3 color = temp_colors[color_index];
		colors.push_back(color);
	}

	return true;
}

void model::translate(double x, double y, double z)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].x += x;
		vertices[i].y += y;
		vertices[i].z += z;
	}
}

void model::get_model(std::vector<glm::vec3>& out_verts, std::vector<glm::vec3>& out_colors, std::vector<glm::vec3>& out_normals)
{
	out_verts.reserve(out_verts.size() + vertices.size());
	out_colors.reserve(out_colors.size() + colors.size());
	out_normals.reserve(out_normals.size() + normals.size());
	out_verts.insert(out_verts.end(), vertices.begin(), vertices.end());
	out_colors.insert(out_colors.end(), colors.begin(), colors.end());
	out_normals.insert(out_normals.end(), normals.begin(), normals.end());
}