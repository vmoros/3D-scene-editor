#ifndef SHADER
#define SHADER

#include "Helpers.h"

class Shader {
public:
    Shader();
    void init_from_strings(const std::string &vertex_code, const std::string &fragment_code);
    void validate();

    GLuint get_model_location();

    void set_texture(GLuint texture);
    void set_shadow_map(GLuint texture);
    void set_light_matrix(glm::mat4 light_matrix);
    void set_mat4(const std::string &name, const glm::mat4 &mat) const;

    void use();
    GLuint shader_ID;
private:
    GLuint uniform_model;
    GLuint uniform_light_matrix;
    GLuint uniform_shadow_map;
    GLuint uniform_tex;

    void compile();
};

#endif