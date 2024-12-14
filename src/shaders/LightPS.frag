#version 330 core
out vec4 FragColor;

uniform vec3 u_lightColor;

void main()
{
	vec3 lightColor = u_lightColor;

	FragColor = vec4(lightColor, 1.0f);
}