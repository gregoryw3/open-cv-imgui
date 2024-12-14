#include <iostream>
#include <string>

#include <GL/glew.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <fstream>
#include <iostream>
#include <sstream>


const GLint WIDTH = 800;
const GLint HEIGHT = 600;

enum class ShaderType
{
    NONE = -1,
    VERTEX = 0,
    FRAGMENT = 1
};

// const unsigned int SCREEN_WIDTH = 1280;
// const unsigned int SCREEN_HEIGHT = 960;

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

GLuint VAO;
GLuint VBO;
GLuint FBO;
GLuint RBO;
GLuint texture_id;
GLuint shader;

const char* vertex_shader_code = R"*(
#version 330

layout (location = 0) in vec3 pos;

void main()
{
	gl_Position = vec4(0.9*pos.x, 0.9*pos.y, 0.5*pos.z, 1.0);
}
)*";

const char* fragment_shader_code = R"*(
#version 330

out vec4 color;

void main()
{
	color = vec4(0.0, 1.0, 0.0, 1.0);
}
)*";


void create_triangle()
{
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f, // 1. vertex x, y, z
		1.0f, -1.0f, 0.0f, // 2. vertex ...
		0.0f, 1.0f, 0.0f // etc... 
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void add_shader(GLuint program, const char* shader_code, GLenum type) 
{
	GLuint current_shader = glCreateShader(type);

	const GLchar* code[1];
	code[0] = shader_code;

	GLint code_length[1];
	code_length[0] = strlen(shader_code);

	glShaderSource(current_shader, 1, code, code_length);
	glCompileShader(current_shader);

	GLint result = 0;
	GLchar log[1024] = {0};

	glGetShaderiv(current_shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(current_shader, sizeof(log), NULL, log);
		std::cout << "Error compiling " << type << " shader: " << log << "\n";
		return;
	}

	glAttachShader(program, current_shader);
}

void create_shaders()
{
	shader = glCreateProgram();
	if(!shader) {
		std::cout << "Error creating shader program!\n"; 
		exit(1);
	}

	add_shader(shader, vertex_shader_code, GL_VERTEX_SHADER);
	add_shader(shader, fragment_shader_code, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar log[1024] = {0};

	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(log), NULL, log);
		std::cout << "Error linking program:\n" << log << '\n';
		return;
	}

	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(log), NULL, log);
		std::cout << "Error validating program:\n" << log << '\n';
		return;
	}
}

void create_framebuffer()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void bind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void unbind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void rescale_framebuffer(float width, float height)
{
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
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

int main()
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

	// Create the window
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "My Window", NULL, NULL);
	if (!mainWindow)
	{
		std::cout << "GLFW creation failed!\n";
		glfwTerminate();
		return 1;
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(mainWindow);
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "glew initialisation failed!\n";
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glViewport(0, 0, bufferWidth, bufferHeight);

	// create_triangle();
	// create_shaders();
	create_framebuffer();


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");
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

	while (!glfwWindowShouldClose(mainWindow))
	{
		glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();    
        
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui::NewFrame();
        ImGui::Begin("My Scene");

		const float window_width = ImGui::GetContentRegionAvail().x;
		const float window_height = ImGui::GetContentRegionAvail().y;

		rescale_framebuffer(window_width, window_height);
		glViewport(0, 0, window_width, window_height);

		ImVec2 pos = ImGui::GetCursorScreenPos();
		
		ImGui::GetWindowDrawList()->AddImage(
			texture_id, 
			ImVec2(pos.x, pos.y), 
			ImVec2(pos.x + window_width, pos.y + window_height), 
			ImVec2(0, 1), 
			ImVec2(1, 0)
		);

		ImGui::End();
		ImGui::Render();


		bind_framebuffer();
        // Clear the color and depth buffers
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
		
		// glUseProgram(shader);
		// glBindVertexArray(VAO);
		// glDrawArrays(GL_TRIANGLES, 0, 3);
		// glBindVertexArray(0);
		// glUseProgram(0);
		
		unbind_framebuffer();	

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

		glfwSwapBuffers(mainWindow);
	}

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &texture_id);
	glDeleteRenderbuffers(1, &RBO);

    glfwDestroyWindow(mainWindow);
    glfwTerminate();

	return 0;
}