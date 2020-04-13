#version 460 core

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 vertex_uv;

out vec2 uv;

uniform vec2 screen_dimensions;

void main(){

    vec2 vertexPosition_homoneneousspace = vertex_position - screen_dimensions; 
    vertexPosition_homoneneousspace /= screen_dimensions;
    gl_Position = vec4(vertexPosition_homoneneousspace,0,1);
    //gl_Position = vec4(vertex_position, 0, 1);

    // UV of the vertex. No special space for this one.
    uv = vertex_uv;
}