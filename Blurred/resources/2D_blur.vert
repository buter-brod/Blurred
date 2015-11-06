#version 330 core

layout(location = 0) in vec4 vert;
layout(location = 1) in vec2 vertexUV;
out vec2 UV;
uniform mat4 MVP;

void main()
{
	UV = vertexUV;
	gl_Position = MVP * vert;
}