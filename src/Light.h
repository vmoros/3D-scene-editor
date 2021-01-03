#ifndef LIGHT
#define LIGHT

#include "Shadow_Map.h"

class Light {
    public:
        Light();
        Light(float shadow_width, float shadow_height,
                float r, float g, float b,
                float ambient, float diffuse,
                float dir_x, float dir_y, float dir_z);

        void activate(float light_color_location, float ambient_location,
                    float diffuse_location, float dir_location);

        Shadow_Map* get_shadow_map() {return shadow_map;};
		glm::mat4 get_matrix();
		glm::vec3 direction;

    protected:
        glm::vec3 light_color;
        float ambient;
        float diffuse;

        glm::mat4 light_matrix;
        Shadow_Map* shadow_map;
    };

#endif
