#include "mesh.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace Assimp;

// template<>
// struct hash<glm::vec3> {
//     std::size_t operator()(const glm::vec3& v) const noexcept {
//         std::size_t h1 = std::hash<float>{}(v.x);
//         std::size_t h2 = std::hash<float>{}(v.y);
//         std::size_t h3 = std::hash<float>{}(v.z);
//         return h1 ^ (h2 << 1) ^ (h3 << 2);
//     }
// };

Mesh Mesh::loadMeshFromFile(const std::string& stl_path, const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke) {
    Mesh new_mesh(diffuse_color, specular_color, ka, kd, ks, ke);
    //Load the model
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(stl_path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiShadingMode_Phong);

    //Check for errors
    if (!scene || !scene->HasMeshes()) {
            std::cerr << "Error loading STL file: " << importer.GetErrorString() << std::endl;
            return new_mesh;
    }

    //Create vectors for the vertices and indices
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    //Iterate over the meshes
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        //Get the mesh
        aiMesh* mesh = scene->mMeshes[i];

        //Iterate over the vertices of the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            //Create a vertex to store the mesh's vertices temporarily
            glm::vec3 tempVertex;

            /// Set the positions
            tempVertex.x = mesh->mVertices[j].x;
            tempVertex.y = mesh->mVertices[j].y;
            tempVertex.z = mesh->mVertices[j].z;

            // Add the vertex to the vertices vector
            vertices.push_back(tempVertex);

            // Set the normals
            glm::vec3 tempNormal;
            tempNormal.x = mesh->mNormals[j].x;
            tempNormal.y = mesh->mNormals[j].y;
            tempNormal.z = mesh->mNormals[j].z;

            // Add the normal to the normals vector
            new_mesh.vertex_normals.push_back(tempNormal);
        }

        //Iterate over the faces of the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            //Get the face
            aiFace face = mesh->mFaces[j];
            //Add the indices of the face to the vector
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {indices.push_back(face.mIndices[k]);}
        }
    }
    new_mesh.vertices = vertices;
    new_mesh.indices = indices;
    return new_mesh;
}

Mesh::Mesh(const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke)
    : diffuse_color(diffuse_color), specular_color(specular_color), ka(ka), kd(kd), ks(ks), ke(ke) {}

// Mesh Mesh::from_stl(const std::string& stl_path, const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke) {
//     Mesh new_mesh(diffuse_color, specular_color, ka, kd, ks, ke);

//     // Load STL file
//     std::ifstream file(stl_path, std::ios::binary);

//     // Check if file is open
//     if (!file.is_open()) {
//         std::cerr << "Error: Unable to open file '" << stl_path << "'" << std::endl;
//     }

//     // Deserialize STL file
//     std::vector<openstl::Triangle> triangles = openstl::deserializeStl(file);
//     // Close file
//     file.close();

//     for (const auto& triangle : triangles) {
//         new_mesh.verts.push_back({triangle.v0.x, triangle.v0.y, triangle.v0.z});
//         new_mesh.verts.push_back({triangle.v1.x, triangle.v1.y, triangle.v1.z});
//         new_mesh.verts.push_back({triangle.v2.x, triangle.v2.y, triangle.v2.z});
//     }

//     // Convert to vertices and faces
//     const auto& [vertices, faces] = convertToVerticesAndFaces(triangles);
//     for (const auto& vertex : vertices) {
//         new_mesh.verts.push_back({vertex.x, vertex.y, vertex.z});
//     }

//     // Extract unique vertices and generate indices
//     std::unordered_map<glm::vec3, GLuint, std::hash<glm::vec3>> vert_indices;
//     for (const auto& triangle : triangles) {
//         glm::vec3 vertices[3] = {
//             {triangle.v0.x, triangle.v0.y, triangle.v0.z},
//             {triangle.v1.x, triangle.v1.y, triangle.v1.z},
//             {triangle.v2.x, triangle.v2.y, triangle.v2.z}
//         };
//         for (const auto& vertex : vertices) {
//             if (vert_indices.find(vertex) == vert_indices.end()) {
//                 vert_indices[vertex] = new_mesh.verts.size();
//                 new_mesh.verts.push_back(vertex);
//             }
//             new_mesh.indices.push_back(vert_indices[vertex]);
//         }
//     }


//     // Extract faces
//     for (const auto& triangle : triangles) {
//         glm::vec3 vertices[3] = {
//             {triangle.v0.x, triangle.v0.y, triangle.v0.z},
//             {triangle.v1.x, triangle.v1.y, triangle.v1.z},
//             {triangle.v2.x, triangle.v2.y, triangle.v2.z}
//         };
//         std::array<GLuint, 3> face;
//         for (int i = 0; i < 3; ++i) {
//             face[i] = vert_indices[vertices[i]];
//         }
//         new_mesh.faces.push_back(face[0]);
//         new_mesh.faces.push_back(face[1]);
//         new_mesh.faces.push_back(face[2]);
//     }

//     // Compute vertex normals with weighted average
//     new_mesh.vertex_normals.resize(new_mesh.verts.size(), glm::vec3(0.0f));
//     for (size_t i = 0; i < new_mesh.faces.size(); i += 3) {
//         GLuint idx0 = new_mesh.faces[i];
//         GLuint idx1 = new_mesh.faces[i + 1];
//         GLuint idx2 = new_mesh.faces[i + 2];

//         glm::vec3 v0 = new_mesh.verts[idx0];
//         glm::vec3 v1 = new_mesh.verts[idx1];
//         glm::vec3 v2 = new_mesh.verts[idx2];

//         glm::vec3 edge1 = v1 - v0;
//         glm::vec3 edge2 = v2 - v0;
//         glm::vec3 face_normal = glm::cross(edge1, edge2);

//         // Compute face area as the weight
//         float face_area = 0.5f * glm::length(face_normal);

//         // Normalize face normal
//         face_normal = glm::normalize(face_normal);
//         new_mesh.normals.push_back(face_normal);

//         // Weighted normals for each vertex
//         new_mesh.vertex_normals[idx0] += face_normal * face_area;
//         new_mesh.vertex_normals[idx1] += face_normal * face_area;
//         new_mesh.vertex_normals[idx2] += face_normal * face_area;
//     }

//     // Normalize vertex normals
//     for (auto& normal : new_mesh.normals) {
//         normal = glm::normalize(normal);
//     }

//     return new_mesh;
// }

// std::vector<float> Mesh::get_vertices() const {
//     std::vector<float> vertices;
//     for (size_t i = 0; i < verts.size(); ++i) {
//         // Add position
//         vertices.push_back(verts[i].x);
//         vertices.push_back(verts[i].y);
//         vertices.push_back(verts[i].z);
//         // Add normal
//         vertices.push_back(vertex_normals[i].x);
//         vertices.push_back(vertex_normals[i].y);
//         vertices.push_back(vertex_normals[i].z);
//     }
//     return vertices;
// }

// std::vector<float> Mesh::get_indices() const  {
//     std::vector<float> indices;
//     for (size_t i = 0; i < faces.size(); ++i) {
//         indices.push_back(faces[i]);
//     }
//     return indices;
// }
