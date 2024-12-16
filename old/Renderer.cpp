#include "Renderer.hpp"
#include "camera.hpp"

Renderer::Renderer(int screenWidth, int screenHeight, const PerspectiveCamera& camera, const std::vector<Mesh>& meshes, const PointLight& light)
    : screenWidth(screenWidth), screenHeight(screenHeight), camera(camera), meshes(meshes), light(light) {
    zBuffer.resize(screenWidth * screenHeight, -std::numeric_limits<float>::infinity());
}

void Renderer::render(GLFWwindow* window, const std::string& shading, const glm::vec3& bgColor, const glm::vec3& ambientLight) {
    std::fill(zBuffer.begin(), zBuffer.end(), -std::numeric_limits<float>::infinity());

    // Clear the screen with the background color
    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program
    GLuint shaderProgram = glCreateProgram();
    glUseProgram(shaderProgram);

    // Set up the camera matrices
    glm::mat4 view = camera.transform.transformationMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    for (const auto& mesh : meshes) {
        glm::mat4 model = mesh.transform.transformationMatrix();
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Bind the VAO and draw the mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.faces.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Swap buffers
    glfwSwapBuffers(window);
}

glm::vec3 Renderer::computeAmbient(const glm::vec3& ambientLight, float ka, const glm::vec3& objectColor) {
    return ambientLight * ka * objectColor;
}

glm::vec3 Renderer::computeDiffuse(const glm::vec3& point, const glm::vec3& normal, const glm::vec3& diffuseColor, float kd) {
    glm::vec3 lightDir = glm::normalize(light.transform.position - point);
    float distance = glm::length(lightDir);
    float diffuseIntensity = std::max(glm::dot(normal, lightDir), 0.0f);
    float diffuse = diffuseIntensity * kd / (glm::pi<float>() * distance * distance);
    return diffuse * light.intensity * light.color * diffuseColor;
}

glm::vec3 Renderer::computePhong(const glm::vec3& point, const glm::vec3& normal, const glm::vec3& viewDir, const Mesh& mesh, const glm::vec3& ambientLight) {
    glm::vec3 diffuseColor = mesh.diffuse_color;
    glm::vec3 specularColor = mesh.specular_color;
    float ka = mesh.ka;
    float kd = mesh.kd;
    float ks = mesh.ks;
    float ke = mesh.ke;

    glm::vec3 lightDir = glm::normalize(light.transform.position - point);
    float distance = glm::length(lightDir);
    glm::vec3 reflectDir = reflect(lightDir, normal);

    glm::vec3 ambient = computeAmbient(ambientLight, ka, diffuseColor);
    glm::vec3 diffuse = computeDiffuse(point, normal, diffuseColor, kd);
    glm::vec3 specular = computeSpecular(normal, viewDir, reflectDir, distance, mesh);

    return ambient + diffuse + specular;
}

glm::vec3 Renderer::computeSpecular(const glm::vec3& normal, const glm::vec3& viewDir, const glm::vec3& reflectDir, float distance, const Mesh& mesh) {
    float specularStrength = std::pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), mesh.ke);
    float lightIntensity = light.intensity / (glm::pi<float>() * distance * distance);
    return specularStrength * mesh.ks * lightIntensity * light.color * mesh.specular_color;
}

glm::vec3 Renderer::reflect(const glm::vec3& L, const glm::vec3& N) {
    return -L + 2.0f * glm::dot(L, N) * N;
}