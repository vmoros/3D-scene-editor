#include "Light.h"

Light::Light() {
    light_color = glm::vec3(1, 1, 1);
    ambient = 1.0f;
    diffuse = 0.0f;
    direction = glm::vec3(0, -1, 0);
}

Light::Light(float shadow_width, float shadow_height,
        float r, float g, float b,
        float ambient, float diffuse,
        float dir_x, float dir_y, float dir_z) {
    shadow_map = new Shadow_Map();
    shadow_map->init(shadow_width, shadow_height);

    light_color = glm::vec3(r, g, b);
    this->diffuse = diffuse;
    this->ambient = ambient;

	direction = glm::vec3(dir_x, dir_y, dir_z);
	light_matrix = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 100.0f);
}

glm::mat4 Light::get_matrix() {
    return light_matrix * glm::lookAt(-direction, glm::vec3(0), glm::vec3(0, 1, 0));
}

void Light::activate(float light_color_location, float ambient_location,
                     float diffuse_location, float dir_location) {
    glUniform3f(light_color_location, light_color.x, light_color.y, light_color.z);
    glUniform1f(ambient_location, ambient);
    glUniform1f(diffuse_location, diffuse);
    glUniform3f(dir_location, direction.x, direction.y, direction.z);
}
