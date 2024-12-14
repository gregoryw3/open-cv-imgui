#version 410 core

in vec3 fragNormal;
in vec3 fragPosition;

out vec4 fragColor;

uniform vec3 lightPosition; // World-space light position
uniform vec4 ambientColor;  // Ambient term
uniform vec4 diffuseColor;  // Diffuse term
uniform vec4 specularColor; // Specular term
uniform float shininess;    // Material shininess
uniform vec4 sceneColor;    // Scene ambient color
uniform vec3 viewPosition;  // World-space camera/eye position

void main() {
    // Normalize the normal
    vec3 N = normalize(fragNormal);

    // Light direction
    vec3 L = normalize(lightPosition - fragPosition);

    // View direction
    vec3 E = normalize(viewPosition - fragPosition);

    // Reflect light direction around normal
    vec3 R = reflect(-L, N);

    // Calculate ambient term
    vec4 Iamb = ambientColor;

    // Calculate diffuse term
    vec4 Idiff = diffuseColor * max(dot(N, L), 0.0);

    // Calculate specular term
    vec4 Ispec = specularColor * pow(max(dot(R, E), 0.0), shininess);

    // Combine terms with scene color
    fragColor = sceneColor + Iamb + Idiff + Ispec;
}


// #version 120

// varying vec3 N;
// varying vec3 v;    

// void main (void)  
// {  
//    vec3 L = normalize(gl_LightSource[0].position.xyz - v);   
//    vec3 E = normalize(-v);       // we are in Eye Coordinates, so EyePos is (0,0,0)  
//    vec3 R = normalize(-reflect(L,N));  
 
//    //calculate Ambient Term:  
//    vec4 Iamb = gl_FrontLightProduct[0].ambient;    

//    //calculate Diffuse Term:  
//    vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N,L), 0.0);    
   
//    // calculate Specular Term:
//    vec4 Ispec = gl_FrontLightProduct[0].specular 
//                 * pow(max(dot(R,E),0.0),0.3*gl_FrontMaterial.shininess);

//    // write Total Color:  
//    gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;   
// }