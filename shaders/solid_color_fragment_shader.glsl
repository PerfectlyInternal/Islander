#version 460 core

// input data
in vec3 fragment_position;
in vec3 fragment_base_color;
in vec3 fragment_normal;

in vec3 light_direction;
in vec3 light_color;
in vec3 ambient_color;

// output data
out vec3 color;

uniform vec3 camera_pos;

uniform vec3 fog_color = vec3(1, 1, 1);
uniform float fog_start = 100.0;
uniform float fog_end = 250.0; 

void main(){
	vec3 normal = normalize(fragment_normal);
	float diff = max(dot(normal, light_direction), 0.0);
	vec3 diffuse = diff * light_color;

	float fog = 0;

	float dist = distance(camera_pos, fragment_position);
	if (dist < fog_start) fog = 0;
	else if (dist > fog_end) fog = 1;
	else fog = (dist - fog_start) / (fog_end - fog_start);

	//fog = 0;

	color = mix((ambient_color + (diffuse * light_color)) * fragment_base_color, fog_color, fog);
}