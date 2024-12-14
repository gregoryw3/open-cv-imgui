import numpy as np
from stl import mesh
from transform import Transform

class Mesh:
    def __init__(self, diffuse_color, specular_color, ka, kd, ks, ke):
        self.verts = []
        self.faces = []
        self.normals = []
        self.vertex_normals = []  
        self.transform = Transform()
        self.diffuse_color = np.array(diffuse_color)
        self.specular_color = np.array(specular_color)
        self.ka = ka
        self.kd = kd
        self.ks = ks
        self.ke = ke

    @staticmethod
    def from_stl(stl_path, diffuse_color, specular_color, ka, kd, ks, ke):
        loaded_mesh = mesh.Mesh.from_file(stl_path)
        mesh_instance = Mesh(diffuse_color, specular_color, ka, kd, ks, ke)

        vertices_map = {}
        vertex_counter = 0

        for i, face in enumerate(loaded_mesh.vectors):
            current_face = []

            for vertex in face:
                vertex_tuple = tuple(vertex)

                if vertex_tuple not in vertices_map:
                    vertices_map[vertex_tuple] = vertex_counter
                    mesh_instance.verts.append(np.array(vertex_tuple))
                    vertex_counter += 1

                current_face.append(vertices_map[vertex_tuple])

            mesh_instance.faces.append(current_face)
            mesh_instance.normals.append(tuple(loaded_mesh.normals[i]))

        mesh_instance.compute_vertex_normals()  
        return mesh_instance

    def compute_vertex_normals(self):
        self.vertex_normals = [np.zeros(3) for _ in self.verts]
        face_contributions = [0 for _ in self.verts]

        for face, normal in zip(self.faces, self.normals):
            for vertex_index in face:
                self.vertex_normals[vertex_index] += np.array(normal)
                face_contributions[vertex_index] += 1

        for i in range(len(self.vertex_normals)):
            if face_contributions[i] > 0:
                self.vertex_normals[i] /= face_contributions[i]
                self.vertex_normals[i] = self.vertex_normals[i] / np.linalg.norm(self.vertex_normals[i])
