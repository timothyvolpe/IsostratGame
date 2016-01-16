#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
out vec3 frag_color;

layout(std140) uniform GlobalMatrices {
	mat4 mvp_persp;
	mat4 mvp_ortho;
} modelViewProjection;

void main()
{
    gl_Position = modelViewProjection.mvp_persp * vec4( in_position, 1.0f );
	frag_color = in_color;
}