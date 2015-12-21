#version 330 core

in vec2 frag_texCoords;
out vec4 outputColor;

uniform sampler2D textureSampler;

void main()
{
	outputColor = vec4( 1.0f, 0.0f, 0.0f, texture( textureSampler, frag_texCoords ).r );
}