#version 330 core

in vec2 geom_texCoords[4];
out vec2 frag_texCoords;

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	frag_texCoords = geom_texCoords[0];
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	frag_texCoords = geom_texCoords[1];
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	frag_texCoords = geom_texCoords[2];
	EmitVertex();
	gl_Position = gl_in[3].gl_Position;
	frag_texCoords = geom_texCoords[3];
	EmitVertex();
	
	EndPrimitive();
}