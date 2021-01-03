#include "Model_3D.h"
#include <algorithm>

Model_3D::Model_3D() {
    VAO = 0;
    VBO = 0;
    IBO = 0;
    model = glm::mat4(1.0f);
}

Model_3D::Model_3D(std::vector<Vertex> verts, std::vector<Triangle_Indices> inds) : Model_3D() {
    vertices = verts;
    indices = inds;
    fit_in_unit_cube();
    
    env_mapping = false;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Triangle_Indices), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(3 * i * sizeof(float)));
        glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

glm::vec3 Model_3D::get_barycenter() {
    glm::vec3 bary(0.0f);

    for (Vertex vert : vertices) {
        bary += (glm::vec3(model * glm::vec4(vert.to_vec3(), 1)) / (float)vertices.size());
    }
    return bary;
}

void Model_3D::fit_in_unit_cube() {
    // find max dimension
    std::vector<float> xs, ys, zs;
    for (size_t i = 0; i < vertices.size(); i++) {
        Vertex curr_vert = vertices.at(i);
        if (i % 3 == 0) {
            xs.push_back(curr_vert.x);
        } else if (i % 3 == 1) {
            ys.push_back(curr_vert.y);
        } else {
            zs.push_back(curr_vert.z);
        }
    }

    float x_range = *std::max_element(xs.begin(), xs.end()) - *std::min_element(xs.begin(), xs.end());
    float y_range = *std::max_element(ys.begin(), ys.end()) - *std::min_element(ys.begin(), ys.end());
    float z_range = *std::max_element(zs.begin(), zs.end()) - *std::min_element(zs.begin(), zs.end());
    float max_dim = std::max(x_range, std::max(y_range, z_range));

    // move barycenter to origin and divide all vertices by max dimension
    glm::vec3 bary = get_barycenter();
    for (Vertex& v : vertices) {
        v.x -= bary.x;
        v.x /= max_dim;
        v.y -= bary.y;
        v.y /= max_dim;
        v.z -= bary.z;
        v.z /= max_dim;
    }
}


void Model_3D::render_model() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Model_3D::translate(glm::vec3 translate) {
    model = glm::translate(glm::mat4(1), translate) * model;
}

void Model_3D::translate(float xTranslate, float yTranslate, float zTranslate) {
    translate(glm::vec3(xTranslate, yTranslate, zTranslate));
}

void Model_3D::scale(float scale_factor) {
    glm::mat4 trans(1), scale(1);
    trans = glm::translate(trans, -get_barycenter());
    scale = glm::scale(scale, glm::vec3(scale_factor));
    model = glm::inverse(trans) * scale * trans * model;
}

void Model_3D::rotate(float angle, int given_axis) {
    if (given_axis < 0 || given_axis > 2) {
        std::cout << "Invalid axis given: " << given_axis << std::endl;
        exit(1);
    }
    glm::vec3 axis(0);
    axis[given_axis] = 1.0f;

    glm::mat4 trans(1), rotate(1);
    trans = glm::translate(trans, -get_barycenter());
    rotate = glm::rotate(rotate, glm::radians(angle), axis);
    model = glm::inverse(trans) * rotate * trans * model;
}

glm::mat4 Model_3D::get_model() {
    return model;
}