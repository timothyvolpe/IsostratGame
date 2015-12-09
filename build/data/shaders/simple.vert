#version 430

layout(location = 0) in vec3 in_position;
//layout(location = 1) in vec3 in_color;

//uniform mat4 projectionMatrix;
//uniform mat4 viewMatrix;
//uniform mat4 modelMatrix;

out vec4 frag_color;

void main()
{
    gl_Position = vec4( in_position, 1.0f );
	frag_color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
}