#version 330 core

in vec3 frag_color;
out vec4 outputColor;

void main()
{
	outputColor = vec4( frag_color, 1.0f );
}