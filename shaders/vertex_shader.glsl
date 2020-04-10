#version 460 core

// input data
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec3 vertex_normal;

// output data
out vec3 fragment_position;
out vec3 fragment_base_color;
out vec3 fragment_normal;

out vec3 light_direction;
out vec3 light_color;
out vec3 ambient_color;

// value that stays constant for the whole frame
uniform mat4 matrix;
uniform mat4 view;
uniform mat4 model;

uniform vec3 ambient_light_color = vec3(0.25, 0.25, 0.25);
uniform vec3 directional_light_color = vec3(1, 1, 1);
uniform vec3 directional_light_direction = vec3(1, 0, 0);

void main()
{
	gl_Position = matrix * vec4(vertex_position, 1.0);

	fragment_position = vec3(model * vec4(vertex_position, 1.0));
	fragment_base_color = vertex_color;
	fragment_normal = vertex_normal;

	light_color = directional_light_color;
	light_direction = normalize(directional_light_direction);
	ambient_color = ambient_light_color;
}