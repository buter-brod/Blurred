#version 330 core

in vec2 UV;
layout(location = 0) out vec4 color;
uniform sampler2D currTex;

void main()
{
	color = vec4(texture2D(currTex, UV).rgb, 1.f);
}