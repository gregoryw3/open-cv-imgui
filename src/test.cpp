#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

enum class ShaderType
{
    NONE = -1,
    VERTEX = 0,
    FRAGMENT = 1
};

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 960;

GLFWwindow* InitGlfwWindow()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return nullptr;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Triangle", NULL, NULL);
    if (!window)
    {
        std::cerr << "Error: Window creation failed" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Set swap interval. frame rate */
    glfwSwapInterval(1);

    /* GLEW test */
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW init error" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    return window;
}

std::string ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream ss;

    if (stream.fail())
    {
        std::cout << "[ERROR][ParseShader] The filepath  \"" << filepath << "\" doesn't exist." << std::endl;
    }

    while (getline(stream, line))    // parse the shader file line by line
    {
        ss << line << '\n';
    }

    return ss.str();
}

void CheckShaderStatus(unsigned int shaderID, unsigned int status)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "[ERROR] Shader failed.\n" << infoLog << std::endl;
    }
}

unsigned int CreateShader(ShaderType shaderType, const std::string& filepath)
{
    std::string shaderSrc = ParseShader(filepath);
    const char* shaderSrcCStr = shaderSrc.c_str();      // convert string to the format that OpenGL use

    unsigned int shaderID = 0;
    switch (shaderType)
    {
    case ShaderType::VERTEX:
        shaderID = glCreateShader(GL_VERTEX_SHADER);    // generate an id for this shader
        break;
    case ShaderType::FRAGMENT:
        shaderID = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        std::cout << "[Error] Invalid shader type " << (int)shaderType << std::endl;
        break;
    }

    glShaderSource(shaderID, 1, &shaderSrcCStr, NULL);  // load shader data from file          
    glCompileShader(shaderID);                          // compile shader
    CheckShaderStatus(shaderID, GL_COMPILE_STATUS);     // error check

    return shaderID;
}

int GetUniformLocation(unsigned int shaderProgramID, const std::string& name)
{
    int location = glGetUniformLocation(shaderProgramID, name.c_str());   // send data to the vertex shader

    if (location == -1)
    {
        std::cout << "Warning: uniform \""<< name << "\" doesn't exist or unused!" << std::endl;
    }

    return location;
}

int main(void)
{
    if (!glfwInit()) {
		std::cout << "GLFW initialisation failed!\n";
		glfwTerminate();
		return 1;
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Triangle", NULL, NULL);
	if (!window)
	{
		std::cout << "GLFW creation failed!\n";
		glfwTerminate();
		return 1;
	}

    int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
		std::cout << "glew initialisation failed!\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

    glViewport(0, 0, bufferWidth, bufferHeight);

    /* ----------------------------------------------------
                        VAO Setup
    -----------------------------------------------------*/
    // specify which input data goes to which vertex attribute
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);                   // actively working on this vao id

    /* ----------------------------------------------------
                           VBO
    -----------------------------------------------------*/
    // Vertices
    const unsigned int NUM_POS_ATTRIB = 3;
    const unsigned int NUM_NORMAL_ATTRIB = 3;
    const unsigned int stride = NUM_POS_ATTRIB * sizeof(float) + NUM_NORMAL_ATTRIB * sizeof(float);

    float vertices[] = {
        // pos coord (x, y, z)      normal (x, y, z)
        // front
        -0.5f, -0.5f,  0.5f,        0.0f, 0.0f, 1.0f,  // 0
         0.5f, -0.5f,  0.5f,        0.0f, 0.0f, 1.0f,  // 1
         0.5f,  0.5f,  0.5f,        0.0f, 0.0f, 1.0f,  // 2
        -0.5f,  0.5f,  0.5f,        0.0f, 0.0f, 1.0f,  // 3

        // back
        -0.5f, -0.5f, -0.5f,        0.0f, 0.0f, -1.0f, // 4
         0.5f, -0.5f, -0.5f,        0.0f, 0.0f, -1.0f, // 5
         0.5f,  0.5f, -0.5f,        0.0f, 0.0f, -1.0f, // 6
        -0.5f,  0.5f, -0.5f,        0.0f, 0.0f, -1.0f, // 7

        // left
        -0.5f, -0.5f, -0.5f,       -1.0f, 0.0f, 0.0f,  // 8
        -0.5f,  0.5f, -0.5f,       -1.0f, 0.0f, 0.0f,  // 9
        -0.5f, -0.5f,  0.5f,       -1.0f, 0.0f, 0.0f,  // 10
        -0.5f,  0.5f,  0.5f,       -1.0f, 0.0f, 0.0f,  // 11

        // right
         0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,  // 12
         0.5f, -0.5f,  0.5f,        1.0f, 0.0f, 0.0f,  // 13
         0.5f,  0.5f,  0.5f,        1.0f, 0.0f, 0.0f,  // 14
         0.5f,  0.5f, -0.5f,        1.0f, 0.0f, 0.0f,  // 15

        // up
        -0.5f,  0.5f, -0.5f,        0.0f, 1.0f, 0.0f,  // 16
         0.5f,  0.5f, -0.5f,        0.0f, 1.0f, 0.0f,  // 17
         0.5f,  0.5f,  0.5f,        0.0f, 1.0f, 0.0f,  // 18
        -0.5f,  0.5f,  0.5f,        0.0f, 1.0f, 0.0f,  // 19

        // down
        -0.5f, -0.5f, -0.5f,        0.0f, -1.0f, 0.0f, // 20
         0.5f, -0.5f, -0.5f,        0.0f, -1.0f, 0.0f, // 21
         0.5f, -0.5f,  0.5f,        0.0f, -1.0f, 0.0f, // 22
        -0.5f, -0.5f,  0.5f,        0.0f, -1.0f, 0.0f  // 23
    };

    unsigned int vboID;
    glGenBuffers(1, &vboID);                    // generate an id for this buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboID);       // actively working on this buffer id
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // load vertices to this buffer

    /* ----------------------------------------------------
                           IBO
    -----------------------------------------------------*/
    // Index buffer
    // cube
    unsigned int indices[] = {
        0, 1, 2,    // front
        0, 2, 3,

        4, 5, 6,    // back
        4, 6, 7,

        8,  9, 10,  // left
        9, 10, 11,

        12, 13, 14, // right
        12, 14, 15,

        16, 17, 18, // up
        16, 18, 19,

        20, 21, 22, // down
        20, 22, 23        
    };
    unsigned int numVertices = sizeof(indices) / sizeof(float);
    
    unsigned int iboID;
    glGenBuffers(1, &iboID);                        // generate an id for this buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);   // actively working on this buffer id
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);    // load indices to this buffer

    /* ----------------------------------------------------
                     VAO Setup completed
    -----------------------------------------------------*/
    // glVertexAttribPointer
    // 0: correspond to "layout (location = 0)" in vertex shader
    // 3: this attribute has 3 elements (vec3)
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    // 1: correspond to "layout (location = 1)" in vertex shader
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind VAO, VBO, and IBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* ----------------------------------------------------
                   Light VAO, VBO, IBO Setup
    -----------------------------------------------------*/
    unsigned int lightVaoID;
    glGenVertexArrays(1, &lightVaoID);
    glBindVertexArray(lightVaoID);
    
    // Use the same cube vbo
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    
    // Use the same cube ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
    
    // Set vertex attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO, VBO, and IBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* ----------------------------------------------------
                      Shaders Setup
    -----------------------------------------------------*/
    unsigned int shaderProgramID;
    {
        unsigned int vertexShaderID = CreateShader(ShaderType::VERTEX, "src/shaders/BasicVS.vert");
        unsigned int fragShaderID = CreateShader(ShaderType::FRAGMENT, "src/shaders/BasicPS.frag");

        // Link shaders with shader program
        shaderProgramID = glCreateProgram();
        glAttachShader(shaderProgramID, vertexShaderID);
        glAttachShader(shaderProgramID, fragShaderID);
        glLinkProgram(shaderProgramID);
        CheckShaderStatus(shaderProgramID, GL_LINK_STATUS);

        // delete shaders since we have linked them
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragShaderID);
    }

    /* ----------------------------------------------------
                   Light Shaders Setup
    -----------------------------------------------------*/
    unsigned int lightShaderProgramID;
    {
        unsigned int vertexShaderID = CreateShader(ShaderType::VERTEX, "src/shaders/BasicVS.vert");
        unsigned int fragShaderID = CreateShader(ShaderType::FRAGMENT, "src/shaders/LightPS.frag");

        // Link shaders with shader program
        lightShaderProgramID = glCreateProgram();
        glAttachShader(lightShaderProgramID, vertexShaderID);
        glAttachShader(lightShaderProgramID, fragShaderID);
        glLinkProgram(lightShaderProgramID);
        CheckShaderStatus(lightShaderProgramID, GL_LINK_STATUS);

        // delete shaders since we have linked them
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragShaderID);
    }

    /* ----------------------------------------------------
           MVP (Model, View, Projection) Matrix Setup
    -----------------------------------------------------*/
    // Model matrix. Transform the model from local space to world space.
    glm::vec3 cubePos = glm::vec3(0.0f, -0.5f, 2.0f);
    glm::mat4 cubeModel = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)), cubePos);
    glm::vec3 lightPos = glm::vec3(5.0f, 3.0f, 0.0f);
    glm::mat4 lightModel = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)), lightPos);

    // View matrix. Also known as camera matrix
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);

    // Projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);  // perspective projection

    /* ----------------------------------------------------
                       Draw loop
    -----------------------------------------------------*/
    // Common set up for drawings
    const glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Render here
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        {
            /* ----------------------------------------------------
                             Draw the object cube
            -----------------------------------------------------*/
            cubeModel = glm::rotate(cubeModel, glm::radians(0.3f), glm::vec3(0.0f, 1.0f, 1.0f));    // Rotate the model

            // Set shader values
            glUseProgram(shaderProgramID);      // activate shaders
            {
                // Object color
                glm::vec3 objColor = glm::vec3(0.9f, 0.5f, 0.0f);
                int location = GetUniformLocation(shaderProgramID, "u_objColor");
                glUniform3f(location, objColor.x, objColor.y, objColor.z);
            }
            {
                // Light color
                int location = GetUniformLocation(shaderProgramID, "u_lightColor");
                glUniform3f(location, lightColor.x, lightColor.y, lightColor.z);
            }
            {
                // Light position
                int location = GetUniformLocation(shaderProgramID, "u_lightPos");
                glUniform3f(location, lightPos.x, lightPos.y, lightPos.z);
            }
            {
                // Camera position
                int location = GetUniformLocation(shaderProgramID, "u_camPos");
                glUniform3f(location, camPos.x, camPos.y, camPos.z);
            }
            {
                // MVP matrix
                int location = GetUniformLocation(shaderProgramID, "u_model");
                glUniformMatrix4fv(location, 1, GL_FALSE, &cubeModel[0][0]);

                location = GetUniformLocation(shaderProgramID, "u_view");
                glUniformMatrix4fv(location, 1, GL_FALSE, &view[0][0]);
                
                location = GetUniformLocation(shaderProgramID, "u_proj");
                glUniformMatrix4fv(location, 1, GL_FALSE, &proj[0][0]);
            }

            // Draw this VAO
            glBindVertexArray(vaoID);

            // Draw
            glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, nullptr);

            // Unbind
            glUseProgram(0);
            glBindVertexArray(0);
        }

        {
            /* ----------------------------------------------------
                             Draw the light cube
            -----------------------------------------------------*/
            // Set shader values
            glUseProgram(lightShaderProgramID);      // activate shaders
            {
                // Light color
                int location = GetUniformLocation(lightShaderProgramID, "u_lightColor");
                glUniform3f(location, lightColor.x, lightColor.y, lightColor.z);
            }
            {
                // MVP matrix
                int location = GetUniformLocation(shaderProgramID, "u_model");
                glUniformMatrix4fv(location, 1, GL_FALSE, &lightModel[0][0]);

                location = GetUniformLocation(shaderProgramID, "u_view");
                glUniformMatrix4fv(location, 1, GL_FALSE, &view[0][0]);

                location = GetUniformLocation(shaderProgramID, "u_proj");
                glUniformMatrix4fv(location, 1, GL_FALSE, &proj[0][0]);
            }

            // Draw this VAO
            glBindVertexArray(lightVaoID);

            // Draw
            glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, nullptr);

            // Unbind
            glUseProgram(0);
            glBindVertexArray(0);
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Unbind everything at the end
    glUseProgram(0);
    glBindVertexArray(0);

    glfwTerminate();
    return 0;
}