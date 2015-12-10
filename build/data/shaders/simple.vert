#version 410

layout(location = 0) in vec3 in_position;
//layout(location = 1) in vec3 in_color;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(std140) uniform GlobalMatrices {
	mat4 mvp_persp;
	mat4 mvp_ortho;
} modelViewProjection;

out vec4 frag_color;

void main()
{
    gl_Position = modelViewProjection.mvp_persp * vec4( in_position, 1.0f );
	frag_color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
}