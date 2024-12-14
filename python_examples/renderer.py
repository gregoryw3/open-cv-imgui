import numpy as np


class Renderer:
    def __init__(self, screen, camera, meshes, light):
        self.screen = screen
        self.camera = camera
        self.meshes = meshes
        self.light = light
        self.z_buffer = np.full((self.screen.height, self.screen.width), -np.inf, float)

    def render(self, shading, bg_color, ambient_light) -> None:
        """
        Render the mesh to the screen with bg_color as the background color, construct an image buffer
        :param bg_color: three element list that is to be the background color of the render
        :return: None
        """
        self.z_buffer.fill(-np.inf)
        image_buffer = np.full((self.screen.height, self.screen.width, 3), bg_color)
        ambient_light = np.array(ambient_light)

        for mesh in self.meshes:
            for face_idx, face in enumerate(mesh.faces):
                verts = [mesh.verts[i] for i in face]
                transformed_verts = [mesh.transform.apply_to_point(p) for p in verts]

                face_normal = mesh.normals[face_idx]

                view_dir = self.camera.transform.apply_to_normal([0.0, 0.0, -1.0])
                if np.dot(face_normal, view_dir) > 0:  # Back face
                    continue

                # Project vertices to screen space
                verts_device_coords = [self.camera.project_point(p) for p in transformed_verts]

                # Clip against near/far planes
                if any(v[2] < -1.0 or v[2] > 1.0 for v in verts_device_coords):
                    continue

                verts_screen_coords = self.screen.device_to_screen(verts_device_coords)

                # Calculate bounding box
                min_x = max(0, int(np.floor(min(v[0] for v in verts_screen_coords))))
                max_x = min(self.screen.width - 1, int(np.ceil(max(v[0] for v in verts_screen_coords))))
                min_y = max(0, int(np.floor(min(v[1] for v in verts_screen_coords))))
                max_y = min(self.screen.height - 1, int(np.ceil(max(v[1] for v in verts_screen_coords))))

                # Get vertex normals (transformed)
                transformed_normals = [mesh.transform.apply_to_normal(mesh.vertex_normals[i]) for i in face]
                # transformed_normals = mesh.vertex_normals
                for x in range(min_x, max_x + 1):
                    for y in range(min_y, max_y + 1):
                        bary = self.screen.barycentric_coordinates(x, y, verts_screen_coords)

                        # Back face culling by using barry coordinates and normalized device coordinates (NDC)
                        # Skip points outside the triangle
                        if any(b < 0 or b > 1 for b in bary):
                            continue

                        # Interpolate depth
                        depth = sum(bary[i] * verts_device_coords[i][2] for i in range(3))

                        # Depth test
                        if depth < self.z_buffer[x, y]:
                            continue
                        self.z_buffer[x, y] = depth

                        if shading == "barycentric":
                            vertex_colors = np.array([
                                [0.0, 0.0, 255.0],  # Red
                                [0.0, 255.0, 0.0],  # Green
                                [255.0, 0.0, 0.0]   # Blue
                            ])
                            color = sum(bary[i] * vertex_colors[i] for i in range(3))
                            image_buffer[x, y] = color
                        elif shading == "flat":
                            world_pos = sum(bary[i] * transformed_verts[i] for i in range(3))
                            diffuse = self.compute_diffuse(
                                world_pos,
                                face_normal,
                                mesh.diffuse_color,
                                mesh.kd,
                            )
                            ambient = self.compute_ambient(ambient_light, mesh.ka, mesh.diffuse_color)
                            color = ambient + diffuse
                            image_buffer[x, y] = color * 255.0
                        elif shading == "phong":
                            world_pos = sum(bary[i] * verts_device_coords[i] for i in range(3))
                            world_pos = self.camera.inverse_project_point(world_pos)
                            normal = sum(bary[i] * transformed_normals[i] for i in range(3))
                            normal = normal / np.linalg.norm(normal)
                            view_dir = self.camera.transform.position - world_pos
                            color = self.compute_phong(world_pos, normal, view_dir, mesh, ambient_light)
                            image_buffer[x, y] = color * 255.0
                        elif shading == "depth":
                            normalized_depth = (depth + 1) / 2
                            grayscale = normalized_depth * 255
                            image_buffer[x, y] = [grayscale, grayscale, grayscale]
                        else:
                            raise ValueError(f"Unknown shading mode: {shading}")


        # Draw the image buffer to the screen
        self.screen.draw(image_buffer)

    def compute_ambient(self, ambient_light, ka, object_color):
        # Ambient = Irradiance * Reflectance
        # Irradiance = Ambient Light Color * Ambient Light Intensity (ka)
        # Reflectance = object color
        return ambient_light * ka * object_color

    def compute_diffuse(self, point, normal, diffuse_color, kd):
        light_dir = self.light.transform.position - point
        distance = np.linalg.norm(light_dir)
        light_dir = light_dir / distance

        diffuse_intensity = max(np.dot(normal, light_dir), 0.0)
        diffuse = diffuse_intensity * kd / (np.pi * distance ** 2)

        return diffuse * self.light.intensity * self.light.color * diffuse_color

    def compute_phong(self, point, normal, view_dir, mesh, ambient_light):
        """
        Compute Phong shading for a point

        :param point: World space point being shaded
        :param normal: Surface normal at the point
        :param view_dir: Direction from point to camera
        :param diffuse_color: Diffuse color of the object
        :param specular_color: Specular color of the object
        :param ka: Ambient coefficient
        :param kd: Diffuse coefficient
        :param ks: Specular coefficient
        :param ke: Specular exponent (shininess)
        :param ambient_light: Ambient light color
        :return: Computed color for the point
        """

        # Get material properties
        diffuse_color = mesh.diffuse_color
        specular_color = mesh.specular_color
        ka = mesh.ka
        kd = mesh.kd
        ks = mesh.ks
        ke = mesh.ke

        # Normalize input vectors
        normal = normal / np.linalg.norm(normal)
        view_dir = view_dir / np.linalg.norm(view_dir)

        # Compute light direction
        light_dir = self.light.transform.position - point
        distance = np.linalg.norm(light_dir)
        light_dir = light_dir / distance
        reflect_dir = self.reflect(light_dir, normal)

        # Ambient component
        ambient = self.compute_ambient(ambient_light, ka, diffuse_color)

        # Diffuse component
        # diffuse_intensity = max(np.dot(normal, light_dir), 0.0)
        # diffuse_term = diffuse_intensity * kd * diffuse_color
        diffuse = self.compute_diffuse(point, normal, diffuse_color, kd)

        # Specular component
        specular = self.compute_specular(normal, view_dir, reflect_dir, distance, mesh)

        # Combine lighting components and account for light intensity and distance falloff
        # light_intensity = self.light.intensity / (np.pi * distance ** 2)
        # color = (ambient + (diffuse_term + specular_term) * light_intensity * self.light.color)

        return ambient + diffuse + specular

    def compute_specular(self, normal, view_dir, reflect_dir, distance, mesh):
        specular_strength = max(np.dot(view_dir, reflect_dir), 0.0) ** mesh.ke
        light_intensity = self.light.intensity / (np.pi * distance ** 2)
        specular = specular_strength * mesh.ks * light_intensity * self.light.color * mesh.specular_color
        return specular

    def reflect(self, L, N):
        # Compute reflection vector using the formula R = -L + 2(N . L)N
        return -L + 2 * np.dot(L, N) * N