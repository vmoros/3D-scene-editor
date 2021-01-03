#include "Shader.h"

Shader::Shader() {
    shader_ID = 0;
    uniform_model = 0;
}

void Shader::init_from_strings(const std::string &vertex_code, const std::string &fragment_code) {
    shader_ID = glCreateProgram();

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertex_string_const = vertex_code.c_str();
    glShaderSource(vertex_shader, 1, &vertex_string_const, NULL);
    glCompileShader(vertex_shader);
    glAttachShader(shader_ID, vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragment_string_const = fragment_code.c_str();
    glShaderSource(fragment_shader, 1, &fragment_string_const, NULL);
    glCompileShader(fragment_shader);
    glAttachShader(shader_ID, fragment_shader);

    compile();
}

void Shader::validate() {
    GLint result = 0;
    GLchar buffer[512] = { 0 };

    glValidateProgram(shader_ID);
    glGetProgramiv(shader_ID, GL_VALIDATE_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shader_ID, 512, NULL, buffer);
        printf("Error validating program: '%s'\n", buffer);
        return;
    }
}

void Shader::compile() {
    glLinkProgram(shader_ID);
    uniform_model = glGetUniformLocation(shader_ID, "model");
    uniform_tex = glGetUniformLocation(shader_ID, "tex");
    uniform_light_matrix = glGetUniformLocation(shader_ID, "light_transform");
    uniform_shadow_map = glGetUniformLocation(shader_ID, "shadow_map");
}

GLuint Shader::get_model_location() {
    return uniform_model;
}

void Shader::set_texture(GLuint texture) {
    glUniform1i(uniform_tex, texture);
}

void Shader::set_shadow_map(GLuint texture) {
    glUniform1i(uniform_shadow_map, texture);
}

void Shader::set_light_matrix(glm::mat4 light_matrix) {
    glUniformMatrix4fv(uniform_light_matrix, 1, GL_FALSE, glm::value_ptr(light_matrix));
}

void Shader::set_mat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(shader_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::use() {
    glUseProgram(shader_ID);
}