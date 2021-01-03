#ifndef MODEL_3D
#define MODEL_3D

#include "Vertex.h"

class Model_3D {
public:
    Model_3D();
    Model_3D(std::vector<Vertex> verts, std::vector<Triangle_Indices> inds);

    void translate(float xTranslate, float yTranslate, float zTranslate);
    void translate(glm::vec3 translate);
    void scale(float scale_factor);
    void rotate(float angle, int given_axis);    
    glm::vec3 get_barycenter();
    glm::mat4 get_model();
    void render_model();
    void fit_in_unit_cube();

    bool env_mapping;
private:
    glm::mat4 model;
    GLuint VAO, VBO, IBO;
    std::vector<Vertex> vertices;
    std::vector<Triangle_Indices> indices;
};

#endif