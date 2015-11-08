#version 330 core

in vec2 UV;
layout(location = 0) out vec4 color;
uniform sampler2D currTex;
uniform sampler2D maskTex;

const float weights[7] = float[7](0.12, 0.14, 0.15, 0.18, 0.15, 0.14, 0.12); 

void main()
{
    vec2 tex_size = textureSize(currTex, 0);
    float blur_power = texture2D(maskTex, UV).r;
		
	vec4 color_base = vec4(texture2D(currTex, UV).rgb, 1.f);
	vec4 color_blur = vec4(0.0, 0.0, 0.0, 0.0);
	
	for(int i = 0; i < 7; i++)
	{
	  vec2 uv_shifted = UV + vec2((-3.0 + i) / tex_size.x, 0);
	  uv_shifted = clamp(uv_shifted, vec2(0.0, 0.0), vec2(1.0, 1.0));
      color_blur += texture2D(currTex, uv_shifted) * weights[i];
	}
	
	color = color_blur * blur_power + (1.0 - blur_power) * color_base;
}