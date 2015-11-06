#version 330 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

out vec4 color;

uniform sampler2D CurrTex;
uniform float LightPower;
uniform vec3 LightPosition_worldspace;

void main(){
	vec3 LightColor = vec3(1, 1, 1);
	vec4 tex_color = texture2D(CurrTex, UV);
	vec3 MaterialDiffuseColor = tex_color.rgb;
	vec3 MaterialSpecularColor = vec3(0.4, 0.4, 0.4);
	float distance = length(LightPosition_worldspace - Position_worldspace);
	vec3 n = normalize(Normal_cameraspace);
	vec3 l = normalize(LightDirection_cameraspace);
	float cosTheta = clamp(dot(n, l), 0, 1);
	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l, n);
	float cosAlpha = clamp(dot(E,R), 0, 1);
	vec3 lightD = LightColor * LightPower * cosTheta         / (distance*distance);
	vec3 lightS = LightColor * LightPower * pow(cosAlpha, 5) / (distance*distance) / 3;
	color = vec4(MaterialDiffuseColor * lightD + MaterialSpecularColor * lightS, tex_color.a);
}