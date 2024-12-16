// https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
// https://github.com/ThoSe1990/opengl_imgui
// https://github.com/tiffanychen1118/Phong-Lighting

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>

#include <GL/glew.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "mesh.hpp"

using namespace std;
using namespace glm;

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

enum class ShaderType
{
    NONE = -1,
    VERTEX = 0,
    FRAGMENT = 1
};

GLuint VAO;
GLuint VBO;
GLuint FBO;
GLuint RBO;
GLuint texture_id;
GLuint shader;

/*
    ImGUI setup functions
*/

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

/*
    Bezier curve default control points
*/

std::vector<glm::vec3> controlPoints = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.987f),
    glm::vec3(0.0f, 0.0f, 1.922f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 3.948f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 2.878f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.299f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 2.129f, 0.0f),
    glm::vec3(0.0f, 3.948f, 4.0f),
    glm::vec3(0.0f, 2.398f, 4.0f),
    glm::vec3(3.105f, 0.935f, 4.0f),
    glm::vec3(0.0f, 0.0f, 4.0f),

};

std::vector<glm::vec3> cameraControlPoints = {
    glm::vec3(0.0f, 2.398f, 4.0f),
    glm::vec3(0.0f, 0.0f, 4.0f),
    glm::vec3(3.105f, 0.935f, 4.0f),
    glm::vec3(0.0f, 0.0f, 4.0f)
};

std::vector<glm::quat> rotationControlPoints = {
    glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)), // Identity quaternion (no rotation)
    glm::quat(glm::vec3(glm::radians(100.0f), 0.3f, 0.0f)), // Rotate 45 degrees around the x-axis
    glm::quat(glm::vec3(0.0f, glm::radians(-136.0f), 0.0f)), // Rotate 45 degrees around the y-axis
    glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(172.0f)))  // Rotate 45 degrees around the z-axis
};

// This is a cubic Bezier curve function
glm::vec3 bezier(float scaler, const glm::vec3* controlPoints) {
    float scaler_coeff = 1.0f - scaler;
    float scaler_squared = scaler * scaler;
    float scaler_coeff_squared = scaler_coeff * scaler_coeff;
    float scaler_coeff_cubed = scaler_coeff_squared * scaler_coeff;
    float scaler_cubed = scaler_squared * scaler;

    glm::vec3 point = scaler_coeff_cubed * controlPoints[0]; // (1-t)^3 * P0
    point += 3 * scaler_coeff_squared * scaler * controlPoints[1];   // 3 * (1-t)^2 * t * P1
    point += 3 * scaler_coeff * scaler_squared * controlPoints[2];   // 3 * (1-t) * t^2 * P2
    point += scaler_cubed * controlPoints[3];          // t^3 * P3

    return point;
}

std::vector<glm::vec3> generateBezierCurve(const glm::vec3* controlPoints, int numPoints) {
    std::vector<glm::vec3> curvePoints;
    for (int i = 0; i <= numPoints; ++i) {
        float t = (float)i / (float)numPoints;
        curvePoints.push_back(bezier(t, controlPoints));
    }
    return curvePoints;
}

glm::vec3 calculateBezierPoint(float t, const std::vector<glm::vec3>& controlPoints) {
    size_t n = controlPoints.size() - 1;
    glm::vec3 point(0.0f);
    for (size_t i = 0; i <= n; ++i) {
        float binomialCoeff = static_cast<float>(tgamma(n + 1) / (tgamma(i + 1) * tgamma(n - i + 1)));
        float powT = glm::pow(t, i);
        float powOneMinusT = glm::pow(1 - t, n - i);
        point += binomialCoeff * powT * powOneMinusT * controlPoints[i];
    }
    return point;
}

glm::quat slerp(float t, const std::vector<glm::quat>& controlPoints) {
    size_t n = controlPoints.size() - 1;
    glm::quat result = controlPoints[0];
    for (size_t i = 1; i <= n; ++i) {
        float binomialCoeff = static_cast<float>(tgamma(n + 1) / (tgamma(i + 1) * tgamma(n - i + 1)));
        float powT = glm::pow(t, i);
        float powOneMinusT = glm::pow(1 - t, n - i);
        result = glm::slerp(result, controlPoints[i], binomialCoeff * powT * powOneMinusT);
    }
    return result;
}

void showBezierControlPoints() {
    ImGui::Begin("Bezier Control Points");

    for (int i = 0; i < controlPoints.size(); ++i) {
        ImGui::SliderFloat3(("Control Point " + std::to_string(i)).c_str(), &controlPoints[i].x, 0.0f, 4.0f);
    }

    for (int i = 0; i < cameraControlPoints.size(); ++i) {
        ImGui::SliderFloat3(("Camera Control Point " + std::to_string(i)).c_str(), &cameraControlPoints[i].x, 0.0f, 4.0f);
    }

    for (int i = 0; i < rotationControlPoints.size(); ++i) {
        ImGui::SliderAngle(("Rotation Control Point " + std::to_string(i)).c_str(), &rotationControlPoints[i].x);
    }

    ImGui::End();
}

ImU32 HSVtoRGB(float h, float s, float v) {
    float r, g, b;

    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return IM_COL32(int(r * 255), int(g * 255), int(b * 255), 255);
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

GLuint secondVaoID, secondVboID, secondIboID;

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

    Mesh mesh = Mesh::loadMeshFromFile("src/models/unit_sphere.stl", glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.05f, 1.0f, 0.2f, 100.0f);

    unsigned int vboID;
    glGenBuffers(1, &vboID);                    // generate an id for this buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboID);       // actively working on this buffer id
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * 6 * sizeof(float), &mesh.vertices[0], GL_STATIC_DRAW);  // load vertices to this buffer

    /* ----------------------------------------------------
                           IBO
    -----------------------------------------------------*/
    unsigned int numIndicies = mesh.indices.size();

    unsigned int iboID;
    glGenBuffers(1, &iboID);                        // generate an id for this buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);   // actively working on this buffer id
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(float), &mesh.indices[0], GL_STATIC_DRAW);    // load indices to this buffer

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
    glm::vec3 modelPos = glm::vec3(0.0f, -0.5f, 2.0f);
    glm::mat4 model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)), modelPos);
    glm::vec3 lightPos = glm::vec3(5.0f, 3.0f, 0.0f);
    glm::mat4 lightModel = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)), lightPos);

    float t = 0.0f;
    float tIncrement = (1.0/2000.0); // Adjust this value to control the speed of movement along the curve

    // Calculate the position on the Bezier curve for the camera
    glm::vec3 cameraBezierPoint = calculateBezierPoint(t, cameraControlPoints);

    // View matrix. Also known as camera matrix
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, -1000.0f);
    glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);


    // Projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(90.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);  // perspective projection

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

        showBezierControlPoints();
        std::vector<glm::vec3> curvePoints = generateBezierCurve(controlPoints.data(), 1000);

		// Draw the Bezier curve with depth visualization
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		for (int i = 0; i < curvePoints.size() - 1; ++i) {
			ImVec2 p1 = ImVec2(pos.x + curvePoints[i].x * window_width, pos.y + curvePoints[i].y * window_height);
			ImVec2 p2 = ImVec2(pos.x + curvePoints[i + 1].x * window_width, pos.y + curvePoints[i + 1].y * window_height);

			// Adjust color based on z-coordinate
			float depth1 = (curvePoints[i].z + 1.0f) / 2.0f; // Normalize z to [0, 1]
			float depth2 = (curvePoints[i + 1].z + 1.0f) / 2.0f; // Normalize z to [0, 1]
			ImU32 color1 = HSVtoRGB(depth1 * 0.8f, 1.0f, 1.0f); // Adjust hue range for better visualization
			ImU32 color2 = HSVtoRGB(depth2 * 0.8f, 1.0f, 1.0f); // Adjust hue range for better visualization

			// Adjust thickness based on z-coordinate (thinner as it goes further back)
			float thickness1 = 5.0f - depth1 * 4.0f; // Thickness range from 5 to 1
			float thickness2 = 5.0f - depth2 * 4.0f; // Thickness range from 5 to 1

			draw_list->AddLine(p1, p2, color1, thickness1);
		}

        for (int i = 0; i < cameraControlPoints.size() - 1; ++i) {
            ImVec2 p1 = ImVec2(pos.x + cameraControlPoints[i].x * window_width, pos.y + cameraControlPoints[i].y * window_height);
            ImVec2 p2 = ImVec2(pos.x + cameraControlPoints[i + 1].x * window_width, pos.y + cameraControlPoints[i + 1].y * window_height);
            draw_list->AddLine(p1, p2, IM_COL32(0, 255, 0, 255), 1.0f);
        }

		// Draw control points as draggable handles with smokey gray color
		ImU32 controlPointColor = IM_COL32(169, 169, 169, 255); // Smokey gray color
		for (int i = 0; i < 4; ++i) {
			ImVec2 handle_pos = ImVec2(pos.x + controlPoints[i].x * window_width, pos.y + controlPoints[i].y * window_height);

			draw_list->AddCircleFilled(handle_pos, 5.0f, controlPointColor);

			ImGui::SetCursorScreenPos(ImVec2(handle_pos.x - 5, handle_pos.y - 5)); // Adjust for handle size
			ImGui::InvisibleButton(("Object Control Handle" + std::to_string(i)).c_str(), ImVec2(10, 10));

			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
				ImVec2 mouse_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				controlPoints[i].x += mouse_delta.x / window_width;
				controlPoints[i].y += mouse_delta.y / window_height;
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
			}
		}

        // Draw control points for camera
        for (int i = 0; i < cameraControlPoints.size(); ++i) {
            ImVec2 handle_pos = ImVec2(pos.x + cameraControlPoints[i].x * window_width, pos.y + cameraControlPoints[i].y * window_height);

            draw_list->AddCircleFilled(handle_pos, 5.0f, IM_COL32(0, 255, 0, 255));

            ImGui::SetCursorScreenPos(ImVec2(handle_pos.x - 5, handle_pos.y - 5)); // Adjust for handle size
            ImGui::InvisibleButton(("Camera Control Handle" + std::to_string(i)).c_str(), ImVec2(10, 10));

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 mouse_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                cameraControlPoints[i].x += mouse_delta.x / window_width;
                cameraControlPoints[i].y += mouse_delta.y / window_height;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }

		// Draw lines connecting control points to the curve
		ImU32 controlLineColor = IM_COL32(255, 0, 0, 255); // Red color for control lines
		for (int i = 0; i < 3; ++i) {
			ImVec2 p1 = ImVec2(pos.x + controlPoints[i].x * window_width, pos.y + controlPoints[i].y * window_height);
			ImVec2 p2 = ImVec2(pos.x + controlPoints[i + 1].x * window_width, pos.y + controlPoints[i + 1].y * window_height);
			draw_list->AddLine(p1, p2, controlLineColor, 1.0f);
		}

		ImGui::End();
		ImGui::Render();


        // Render the scene to the ImGUI sub-window
		bind_framebuffer();
        // Clear the color and depth buffers
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {

            /* ----------------------------------------------------
                             Draw the object cube
            -----------------------------------------------------*/

            glm::vec3 cameraBezierPoint = calculateBezierPoint(t, cameraControlPoints);
            view = glm::lookAt(cameraBezierPoint, cameraBezierPoint + camFront, camUp);

            glm::vec3 bezierPoint = calculateBezierPoint(t, controlPoints);
            // cubeModel = glm::rotate(cubeModel, glm::radians(0.3f), glm::vec3(0.0f, 1.0f, 1.0f));    // Rotate the model
            model = glm::translate(glm::mat4(0.5f), bezierPoint);
            glm::quat rotationQuat = slerp(t, rotationControlPoints);
            model = glm::rotate(model, glm::angle(rotationQuat), glm::axis(rotationQuat));


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
                glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);

                location = GetUniformLocation(shaderProgramID, "u_view");
                glUniformMatrix4fv(location, 1, GL_FALSE, &view[0][0]);
                
                location = GetUniformLocation(shaderProgramID, "u_proj");
                glUniformMatrix4fv(location, 1, GL_FALSE, &proj[0][0]);
            }

            // Draw this VAO
            glBindVertexArray(vaoID);

            // Draw
            glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, nullptr);

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
            glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, nullptr);

            // Unbind
            glUseProgram(0);
            glBindVertexArray(0);
        }
		
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
        // Increment t to move along the Bezier curve
        t += tIncrement;
        if (t > 1.0f) {
            t = 0.0f; // Loop back to the start of the curve
        }
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