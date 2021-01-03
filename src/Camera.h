#ifndef CAMERA
#define CAMERA

#include "Helpers.h"
const float pi = 3.14159265;

class Camera {
    public:
        Camera();

        Camera(glm::vec3 start_pos, glm::vec3 start_up);

        glm::vec3 get_position();
        glm::mat4 get_view_matrix();
        void update_position();

        glm::vec3 position;
        glm::vec3 up;
        float size;
        float theta;
        float phi;
};

#endif