#version 330 core

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texCoords;
out vec2 geom_texCoords;

layout(std140) uniform GlobalMatrices {
	mat4 mvp_persp;
	mat4 mvp_ortho;
} modelViewProjection;

uniform vec2 resolution;

void main()
{
    gl_Position = modelViewProjection.mvp_ortho * vec4( in_position*resolution, 0.5f, 1.0f );
	geom_texCoords = in_texCoords;
}