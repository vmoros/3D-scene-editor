#ifndef VERTEX
#define VERTEX

#include "Helpers.h"

class Vertex {
    public:
        float x, y, z;
        float r, g, b;
        float x_norm, y_norm, z_norm;
        float x_tex, y_tex;
        Vertex(float x, float y, float z, float r, float g, float b,
        float x_norm, float y_norm, float z_norm, float x_tex, float y_tex);
        void clear_normals();
        glm::vec3 to_vec3();
};

class Triangle_Indices{
    public:
        int i1;
        int i2;
        int i3;
        Triangle_Indices(int i1,  int i2, int i3);
};

void load_off(std::vector<Triangle_Indices> &indices, std::vector<Vertex> &vertices, std::string file_name);

void find_face_normals(std::vector<Triangle_Indices> &indices, std::vector<Vertex> &vertices);

#endif