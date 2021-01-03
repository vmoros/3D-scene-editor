#ifndef SHADOWMAP
#define SHADOWMAP

#include "Helpers.h"

class Shadow_Map {
public:
    Shadow_Map();
    bool init(GLuint width, GLuint height);

    void bindFramebuffer();
    void useTexture(GLenum texture);

    GLuint shadow_map, shadow_width, shadow_height, FBO;
};

#endif