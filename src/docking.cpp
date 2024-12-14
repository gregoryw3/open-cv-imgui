// https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
// https://github.com/ThoSe1990/opengl_imgui

#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "PointLight.hpp"
#include "Renderer.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "mesh.hpp"
#include "quaternion.hpp"
#include "camera.hpp"


const GLint WIDTH = 800;
const GLint HEIGHT = 600;

GLuint VAO;
GLuint VBO;
GLuint EBO;
GLuint FBO;
GLuint RBO;
GLuint texture_id;
GLuint shader;
GLuint shaderProgram;

// Control points for the Bezier curve
glm::vec3 controlPoints[4] = {
    glm::vec3(0.0f, 0.0f, 0.1f),
    glm::vec3(0.1f,  0.1f, 0.2f),
    glm::vec3( 0.2f, 0.2f, 0.3f),
    glm::vec3( 0.3f,  0.3f, 0.4f)
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

void showBezierControlPoints() {
    ImGui::Begin("Bezier Control Points");

    for (int i = 0; i < 4; ++i) {
        ImGui::SliderFloat3(("Control Point " + std::to_string(i)).c_str(), &controlPoints[i].x, 0.0f, 4.0f);
    }

    ImGui::End();
}

struct Quaternion
{
    double w, x, y, z;
};

// Code borrowed from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
// This is not in game format, it is in mathematical format.
Quaternion ToQuaternion(double roll, double pitch, double yaw) // roll (x), pitch (y), yaw (z), angles are in radians
{
    // Abbreviations for the various angular functions

    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

void test_list_sizes() {
    glm::vec3 diffuse_color(1.0f, 0.0f, 1.0f);
    glm::vec3 specular_color(1.0f, 1.0f, 1.0f);
    float ka = 0.05f, kd = 1.0f, ks = 0.2f, ke = 100.0f;

    Mesh mesh = Mesh::from_stl("src/models/unit_cube.stl", diffuse_color, specular_color, ka, kd, ks, ke);

    assert(mesh.verts.size() == 8);
    assert(mesh.faces.size() == 36); // 12 triangles * 3 vertices per triangle
    assert(mesh.normals.size() == 12);
	std::cout << "Test passed!\n";
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

// Takes in a mesh file and binds the appropriate buffers
void create_stl(const Mesh mesh) {

	// Create and bind VBO for vertex positions
	GLuint VBO_positions;
	glGenBuffers(1, &VBO_positions);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
	glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(glm::vec3), &mesh.verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	// Create and bind VBO for vertex normals
	GLuint VBO_normals;
	glGenBuffers(1, &VBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertex_normals.size() * sizeof(glm::vec3), &mesh.vertex_normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(1);

	// Create and bind EBO for faces
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size() * sizeof(GLuint), &mesh.faces[0], GL_STATIC_DRAW);

	// Unbind VAO
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

std::string loadShaderSource(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

void create_shaders()
{
	shader = glCreateProgram();
	if(!shader) {
		std::cout << "Error creating shader program!\n"; 
		exit(1);
	}

	const std::string vertexShaderPath = "src/shaders/PhongShading_vshader.vert";
	const std::string fragmentShaderPath = "src/shaders/PhongShading_fshader.frag";

	std::string vertexShaderSource = loadShaderSource(vertexShaderPath);
	std::string fragmentShaderSource = loadShaderSource(fragmentShaderPath);

	const char* vertexShaderCode = vertexShaderSource.c_str();
	const char* fragmentShaderCode = fragmentShaderSource.c_str();

	add_shader(shader, vertexShaderCode, GL_VERTEX_SHADER);
	add_shader(shader, fragmentShaderCode, GL_FRAGMENT_SHADER);

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

	glUseProgram(shader);
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

void create_mesh(Mesh mesh, GLuint& VAO, GLuint& VBO, GLuint& EBO) {
	
	std::vector<GLfloat> vertices;
	for (size_t i = 0; i < mesh.verts.size(); ++i) {
		const auto& vert = mesh.verts[i];
		const auto& normal = mesh.vertex_normals[i];
		
		vertices.push_back(vert.x);
		vertices.push_back(vert.y);
		vertices.push_back(vert.z);
		vertices.push_back(normal.x);
		vertices.push_back(normal.y);
		vertices.push_back(normal.z);
    }

	std::vector<GLuint> indices = mesh.faces;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// Bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	// Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	// Define the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

	// Unbind the VAO
    glBindVertexArray(0);
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

	// // Print the OpenGL version
    // const GLubyte* version = glGetString(GL_VERSION);
    // std::cout << "OpenGL version: " << version << std::endl;

	glViewport(0, 0, bufferWidth, bufferHeight);

	Mesh mesh = Mesh::from_stl("src/models/unit_cube.stl", glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.05f, 1.0f, 0.2f, 100.0f);

	// create_triangle();
	create_mesh(mesh, VAO, VBO, EBO);
	create_shaders();
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
	ImGui_ImplOpenGL3_Init("#version 410");

	while (!glfwWindowShouldClose(mainWindow))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();    
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ImGui::NewFrame();
		// This is the main window where we draw our graphics
		ImGui::Begin("My Scene", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

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
    	std::vector<glm::vec3> curvePoints = generateBezierCurve(controlPoints, 100);

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

		// Draw control points as draggable handles with smokey gray color
		ImU32 controlPointColor = IM_COL32(169, 169, 169, 255); // Smokey gray color
		for (int i = 0; i < 4; ++i) {
			ImVec2 handle_pos = ImVec2(pos.x + controlPoints[i].x * window_width, pos.y + controlPoints[i].y * window_height);

			draw_list->AddCircleFilled(handle_pos, 5.0f, controlPointColor);

			ImGui::SetCursorScreenPos(ImVec2(handle_pos.x - 5, handle_pos.y - 5)); // Adjust for handle size
			ImGui::InvisibleButton(("handle" + std::to_string(i)).c_str(), ImVec2(10, 10));

			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
				ImVec2 mouse_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				controlPoints[i].x += mouse_delta.x / window_width;
				controlPoints[i].y += mouse_delta.y / window_height;
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
		
		// Any other UI elements you want to draw goes after this
		ImGui::End();

		// Render the curve onto the framebuffer
		for (int i = 0; i < curvePoints.size(); ++i) {
			ImGui::Text("Point %d: (%f, %f, %f)", i, curvePoints[i].x, curvePoints[i].y, curvePoints[i].z);
		}

		// This is the OpenGL window
		ImGui::Render();
		bind_framebuffer();
		
		glUseProgram(shader);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, mesh.faces.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);
		
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
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

    glfwDestroyWindow(mainWindow);
    glfwTerminate();

	return 0;
}