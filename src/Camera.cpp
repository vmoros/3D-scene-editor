#include "Camera.h"
#include <math.h>

Camera::Camera() {}

Camera::Camera(glm::vec3 start_pos, glm::vec3 start_up) : position(start_pos), up(start_up) {
    size = glm::length(position);
    theta = acos(position[1] / size);
    phi = atan(position[0] / position[2]);
}

void Camera::update_position() {
    position.x = size * sin(phi) * sin(theta);
    position.y = size * cos(theta);
    position.z = size * cos(phi) * sin(theta);
}

glm::mat4 Camera::get_view_matrix() {
    return glm::lookAt(position, glm::vec3(0), up);
}

glm::vec3 Camera::get_position() {
    return position;
}