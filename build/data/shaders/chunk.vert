#version 330 core

layout(location = 0) in ivec3 in_position;
layout(location = 1) in vec3 in_color;
out vec4 frag_color;

uniform float voxelScale;

layout(std140) uniform GlobalMatrices {
	mat4 mvp_persp;
	mat4 mvp_ortho;
} modelViewProjection;

void main()
{
    gl_Position = modelViewProjection.mvp_persp * vec4( in_position*voxelScale, 1 );
	frag_color = vec4( in_color, 1.0f );
}