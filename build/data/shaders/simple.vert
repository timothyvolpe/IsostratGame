#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
out vec4 frag_color;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(std140) uniform GlobalMatrices {
	mat4 mvp_persp;
	mat4 mvp_ortho;
} modelViewProjection;

void main()
{
    gl_Position = modelViewProjection.mvp_persp * vec4( in_position, 1.0f );
	frag_color = vec4( in_color, 1.0f );
}