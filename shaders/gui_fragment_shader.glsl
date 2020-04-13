#version 460 core

// texture position
in vec2 uv;

// output color
out vec4 color;

uniform sampler2D texture_sampler;

void main()
{
	color = texture(texture_sampler, uv);
}