#version 460 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_uv;

out vec2 uv;

uniform mat4 matrix;

void main()
{
	gl_Position = matrix * vec4(vertex_position, 1);

	uv = vertex_uv;
}