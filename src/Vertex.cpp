#include "Vertex.h"

Vertex::Vertex(float x, float y, float z, float r, float g, float b,
        float x_norm, float y_norm, float z_norm, float x_tex, float y_tex)
        : x(x), y(y), z(z), r(r), g(g), b(b), x_norm(x_norm), y_norm(y_norm), z_norm(z_norm), x_tex(x_tex), y_tex(y_tex)
        {}

Triangle_Indices::Triangle_Indices(int i1,  int i2, int i3): i1(i1), i2(i2), i3(i3) {}

void Vertex::clear_normals() {
    x_norm = 0;
    y_norm = 0;
    z_norm = 0;
}

glm::vec3 Vertex::to_vec3() {
    return glm::vec3(x, y, z);
}

void load_off(std::vector<Triangle_Indices> &indices, std::vector<Vertex> &vertices, std::string file_name) {
    std::ifstream file(file_name);
    if (!file) {
        std::cout << "I couldn't load the file: " << file_name << std::endl;
        exit(1);
    }

    std::string header;
    int num_vertices, num_faces, _;
    file >> header >> num_vertices >> num_faces >> _;

    for (int i = 0; i < num_vertices; i++) {
        float x, y, z;
        file >> x >> y >> z;

        switch (i % 3) {
            case 0:
                vertices.push_back(Vertex(x, y, z, 1, 1, 1, 0, 0, 0, 0, 0));
                break;
            case 1:
                vertices.push_back(Vertex(x, y, z, 1, 1, 1, 0, 0, 0, 1, 0));
                break;
            case 2:
                vertices.push_back(Vertex(x, y, z, 1, 1, 1, 0, 0, 0, 0.5, 1));
                break;
            default: // this case should never happen
                return;
        }
    }

    for (int i = 0; i < num_faces; i++) {
        int trash, i1, i2, i3;
        file >> trash >> i1 >> i2 >> i3;
        indices.push_back(Triangle_Indices(i1, i2, i3));
    }

    file.close();
}

void find_face_normals(std::vector<Triangle_Indices> &indices, std::vector<Vertex> &vertices) {
    for(Vertex& v : vertices) {
        v.clear_normals();
    }

    for(Triangle_Indices& tri : indices) {
        int i1 = tri.i1, i2 = tri.i2, i3 = tri.i3;
        glm::vec3 v1 = vertices[tri.i2].to_vec3() - vertices[tri.i1].to_vec3();
        glm::vec3 v2 = vertices[tri.i3].to_vec3() - vertices[tri.i1].to_vec3();
        glm::vec3 norm = glm::normalize(glm::cross(v1, v2));

        vertices[i1].x_norm += norm.x;
        vertices[i2].x_norm += norm.x;
        vertices[i3].x_norm += norm.x;
        vertices[i1].y_norm += norm.y;
        vertices[i2].y_norm += norm.y;
        vertices[i3].y_norm += norm.y;
        vertices[i1].z_norm += norm.z;        
        vertices[i2].z_norm += norm.z;
        vertices[i3].z_norm += norm.z;
    }
}