#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 WorldPos;

uniform vec3 u_objColor;
uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform vec3 u_camPos;

void main()
{
	// Set color
	vec3 objColor = u_objColor;
	vec3 lightColor = u_lightColor;

	// Ambient light
	float ambientStrength = 0.2f;
	vec3 ambientLight = ambientStrength * lightColor;

	// Diffuse light
	vec3 lightDir = normalize(u_lightPos - WorldPos);
	vec3 norm = normalize(Normal);
	float diffuseStrength = max(dot(lightDir, norm), 0.0f);
	vec3 diffuseLight = diffuseStrength * lightColor;

	// Specular light
	vec3 viewDir = normalize(u_camPos - WorldPos);
	vec3 reflectDir = reflect(-lightDir, Normal);
	float specularStrength = 1.0f;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specularLight = specularStrength * spec * lightColor;

	// Combine all light factors together
	vec3 finalLight = ambientLight + diffuseLight + specularLight;

	FragColor = vec4(finalLight * objColor, 1.0f);
}