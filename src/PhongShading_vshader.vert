// #version 120

// varying vec3 N;
// varying vec3 v;

// void main(void)
// {

//    v = vec3(gl_ModelViewMatrix * gl_Vertex);       
//    N = normalize(gl_NormalMatrix * gl_Normal);

//    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;


// }

#version 410 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 fragNormal;
out vec3 fragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPosition = model * vec4(inPosition, 1.0);
    fragPosition = worldPosition.xyz;
    fragNormal = mat3(transpose(inverse(model))) * inNormal; // Correctly transform normals
    gl_Position = projection * view * worldPosition;
}
