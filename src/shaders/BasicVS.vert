#version 330 core
layout (location = 0) in vec3 aPos;		// vertex attributes
layout (location = 1) in vec3 aNormal;

out vec3 Normal;	// forward normal vector from vertex shaderes to fragment shaders
out vec3 WorldPos;	// world space position of this vertex

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
	// Check this article for how normal vector is transformed: https://learnopengl.com/Lighting/Basic-Lightings
	Normal = mat3(transpose(inverse(u_model))) * aNormal; 

	WorldPos = (u_model * vec4(aPos, 1.0)).xyz;

	gl_Position = u_proj * u_view * u_model * vec4(aPos, 1.0);
}