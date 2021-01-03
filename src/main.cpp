#include <iostream>

#include "Helpers.h"
#include "Vertex.h"
#include "Model_3D.h"
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

glm::mat4 proj(1.0f);
glm::mat4 view(1.0f);
glm::mat4 model(1.0f);

std::vector<Triangle_Indices> bunny_indices;
std::vector<Vertex> bunny_vertices;
std::vector<Triangle_Indices> bumpy_cube_indices;
std::vector<Vertex> bumpy_cube_vertices;
std::vector<Triangle_Indices> cube_indices;
std::vector<Vertex> cube_vertices;

GLuint uniform_proj = 0;
GLuint uniform_model = 0;
GLuint uniform_color = 0;
GLuint uniform_view = 0;
GLuint uniform_camera_pos = 0;
GLuint uniform_shininess = 0;
GLuint uniform_specular = 0;
GLuint uniform_red_shadows = 0;
GLuint uniform_is_env_mapping = 0;
GLuint uniform_refract = 0;

GLuint plane_VAO, plane_VBO;

int window_width, window_height;

GLuint uniform_light_color = 0;
GLuint uniform_light_dir = 0;
GLuint uniform_ambient = 0;
GLuint uniform_diffuse = 0;

bool red_shadows = false;
bool env_mapping = false;
float refraction = 0.6;
float circle_progress = 0;

std::vector<Model_3D*> models;
Shader main_shader;
Shader shadow_shader;
Shader skybox_shader;
Camera camera;
Light light;

int selected_object = -1;

std::string bumpyCube = "../data/bumpy_cube.off";
std::string bunny = "../data/bunny.off";
std::string unitCube = "../data/unit_cube.off";

void key_callback(GLFWwindow* window, int key, int code, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void load_uniforms();
void load_shaders();
void load_model_files();

void render_models();
void render_plane_and_models();
void render_light();
void render_world();

void make_cube();
void make_bunny();
void make_bumpy_cube();

std::vector<std::string> skybox_faces{
    "../data/night_posx.png", // right
    "../data/night_negx.png", // left
    "../data/night_posy.png", // top    
    "../data/night_negy.png", // bottom
    "../data/night_posz.png", // back
    "../data/night_negz.png", // front
};

// citation: https://learnopengl.com/Advanced-OpenGL/Cubemaps
GLuint loadCubemap(std::vector<std::string> faces) {
    stbi_set_flip_vertically_on_load(true);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

bool clicked_in_object(GLFWwindow* window, Model_3D object) {
    double click_x, click_y;
    glfwGetCursorPos(window, &click_x, &click_y);
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float xmin = width;
    float xmax = 0;
    float ymin = height;
    float ymax = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                glm::vec4 world_coords(i - 0.5, j - 0.5, k - 0.5, 1.0);
                glm::vec4 canonical_coords = proj * view * (object.get_model()) * world_coords;                

                // perspective division
                float x = canonical_coords[0] / canonical_coords[3];
                float y = canonical_coords[1] / canonical_coords[3];

                // go from [-1, 1] to window coordinates
                x = (x / 2 + 0.5f) * width;
                y = (y / 2 + 0.5f) * height;

                y = height - 1 - y; // GLFW flips the y axis

                xmin = std::min(xmin, x);
                ymin = std::min(ymin, y);
                xmax = std::max(xmax, x);
                ymax = std::max(ymax, y);
            }
        }
    }

    bool x_in_bounds = click_x > xmin && click_x < xmax;
    bool y_in_bounds = click_y > ymin && click_y < ymax;
    return x_in_bounds && y_in_bounds;
}

int main() {
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1600, 900, "Viktor's 3D Editor", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
        glewExperimental = true;
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err)); /* Problem: glewInit failed, something is seriously wrong. */
        }
        glGetError(); // pull and savely ignore unhandled errors like GL_INVALID_ENUM
        fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    //handle current context
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glEnable(GL_DEPTH_TEST);

    glfwGetWindowSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    load_shaders();
    load_model_files();

    camera = Camera(glm::vec3(-1, 3, 5), glm::vec3(0, 1, 0));
    light = Light(10000, 10000, 1.0f, 1.0f, 1.0f, 0.2f, 0.5f, 3.0f, -1.0f, -3.0f);

    proj = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);

    float skybox_vertices[] = {     
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    GLuint skybox_VAO, skybox_VBO;
    glGenVertexArrays(1, &skybox_VAO);
    glGenBuffers(1, &skybox_VBO);
    glBindVertexArray(skybox_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    GLuint skybox_texture = loadCubemap(skybox_faces);
    skybox_shader.use();
    skybox_shader.set_texture(skybox_texture);

    float plane_vertices[] = { // position, color, normal
            2.0f, -0.7f,  2.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
            -2.0f, -0.7f,  2.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
            -2.0f, -0.7f, -2.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
            2.0f, -0.7f,  2.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
            -2.0f, -0.7f, -2.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            2.0f, -0.7f, -2.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &plane_VAO);
    glGenBuffers(1, &plane_VBO);
    glBindVertexArray(plane_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

    for (int i = 0; i < 3; i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(3 * i * sizeof(float)));
    }

    glBindVertexArray(0);

    // Loop until window closed
    while (!glfwWindowShouldClose(window)) {      
        glfwGetWindowSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        proj = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);

        view = camera.get_view_matrix();

        // Clear window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_light();
        render_world();

        glDepthFunc(GL_LEQUAL);
        skybox_shader.use();
        skybox_shader.set_mat4("view", glm::mat4(glm::mat3(view)));
        skybox_shader.set_mat4("proj", proj);
        glBindVertexArray(skybox_VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

void make_cube() {
    Model_3D *mesh = new Model_3D(cube_vertices, cube_indices);
    models.push_back(mesh);
}

void make_bunny() {
    Model_3D *mesh = new Model_3D(bunny_vertices, bunny_indices);
    models.push_back(mesh);
}

void make_bumpy_cube() {
    Model_3D *mesh = new Model_3D(bumpy_cube_vertices, bumpy_cube_indices);
    models.push_back(mesh);
}

void key_callback(GLFWwindow* window, int key, int code, int action, int mode) {
    switch (key) {
        case GLFW_KEY_1:
            if (action == GLFW_PRESS) {
                make_cube();
            }
            break;
        case GLFW_KEY_2:
            if (action == GLFW_PRESS) {
                make_bumpy_cube();
            }
            break;
        case GLFW_KEY_3:
            if (action == GLFW_PRESS) {
                make_bunny();
            }
            break;
        case GLFW_KEY_W:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.theta = std::max(camera.theta - 0.1, 0.01);
                camera.update_position();
            }
            break;
        case GLFW_KEY_A:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.phi -= 0.1;
                camera.update_position();
            }
            break;            
        case GLFW_KEY_X:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.theta = std::min(camera.theta + 0.1f, pi - 0.01f);
                camera.update_position();
            }
            break;
        case GLFW_KEY_D:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.phi += 0.1;
                camera.update_position();
            }
            break;
        case GLFW_KEY_O:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.size += 0.1;
                camera.update_position();
            }
            break;
        case GLFW_KEY_P:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.size -= 0.1;
                camera.update_position();
            }
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->translate(0.2f, 0.0f, 0.0f);
                }
            }
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->translate(-0.2f, 0.0f, 0.0f);
                }
            }
            break;
        case GLFW_KEY_UP:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->translate(0.0f, 0.2f, 0.0f);
                }
            }
            break;
        case GLFW_KEY_DOWN:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->translate(0.0f, -0.2f, 0.0f);
                }
            }
            break;
        case GLFW_KEY_MINUS:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->translate(0.0f, 0.0f, 0.2f);
                }
            }
            break;
        case GLFW_KEY_EQUAL:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->translate(0.0f, 0.0f, -0.2f);
                }
            }
            break;
        case GLFW_KEY_9:
            if (action == GLFW_PRESS) {
                if (selected_object >= 0) {
                    models[selected_object]->scale(1.2);
                }
            }
            break;
        case GLFW_KEY_0:
            if (action == GLFW_PRESS) {
                if (selected_object >= 0) {
                    models[selected_object]->scale(0.8);
                }
            }
            break;
        case GLFW_KEY_LEFT_BRACKET:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->rotate(15, 0);
                }
            }
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->rotate(-15, 0);
                }
            }
            break;
        case GLFW_KEY_SEMICOLON:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->rotate(15, 1);
                }
            }
            break;
        case GLFW_KEY_APOSTROPHE:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->rotate(-15, 1);
                }
            }
            break;
        case GLFW_KEY_PERIOD:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->rotate(15, 2);
                }
            }
            break;
        case GLFW_KEY_SLASH:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (selected_object >= 0) {
                    models[selected_object]->rotate(-15, 2);
                }
            }
            break;
        case GLFW_KEY_DELETE:
            if (action == GLFW_PRESS) {
                if (selected_object >= 0) {
                    models.erase(models.begin() + selected_object);
                    selected_object = -1;
                }
            }
            break;
        case GLFW_KEY_S:
            if (action == GLFW_PRESS) {
                red_shadows = !red_shadows;
            }
            break;
        case GLFW_KEY_V:
            if (action == GLFW_PRESS && selected_object >= 0) {
                models[selected_object]->env_mapping = !models[selected_object]->env_mapping;
            }
            break;
        case GLFW_KEY_PAGE_UP:
            if (action == GLFW_PRESS) {
                refraction += 0.1;
            }
            break;
        case GLFW_KEY_PAGE_DOWN:
            if (action == GLFW_PRESS) {
                refraction -= 0.1;
            }
            break;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        selected_object = -1;
        for (size_t i = 0; i < models.size(); i++) {
            if (clicked_in_object(window, *(models.at(i)))) {
                selected_object = i;
                return;
            }
        }
    }
}

// citation: https://learnopengl.com/Advanced-OpenGL/Cubemaps
// citation 2: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
void load_shaders() {
    const std::string vertex_shader = R"glsl(
        #version 330

        in vec3 pos;
        in vec3 color;
        in vec3 norm;
        in vec2 tex;

        out vec4 v_color;
        out vec3 normal;
        out vec3 frag_pos;
        out vec2 tex_coords;
        out vec4 light_pos;

        uniform mat4 model;
        uniform mat4 proj;
        uniform mat4 view;
        uniform mat4 light_transform;

        void main() {
            gl_Position = proj * view * model * vec4(pos, 1.0);
            light_pos = light_transform * model * vec4(pos, 1.0);
            v_color = vec4(color, 1.0f);
            normal = mat3(transpose(inverse(model))) * norm;
            frag_pos = (model * vec4(pos, 1.0)).xyz;
            tex_coords = tex;
        })glsl";

    const std::string fragment_shader = R"glsl(
        #version 330

        in vec4 v_color;
        in vec3 normal;
        in vec3 frag_pos;
        in vec2 tex_coords;
        in vec4 light_pos;

        out vec4 outColor;
        uniform vec3 color;
        uniform bool red_shadows;
        uniform bool is_env_mapping;
        uniform float refraction;

        struct Light{
            vec3 light_color;
            float ambient;
            vec3 light_direction;
            float diffuse;
        };

        uniform vec3 camera_pos;
        uniform Light light;
        uniform float specular;
        uniform float shininess;
        uniform sampler2D tex;
        uniform sampler2D shadow_map;
        uniform samplerCube skybox;

        float find_shadow_intensity(Light light) {
            vec3 persp_division = light_pos.xyz / light_pos.w; // perspective division
            persp_division = (persp_division * 0.5) + 0.5; // map to [0, 1]

            vec3 normal = normalize(normal);
            vec3 lightDirection = normalize(light.light_direction);
            float depth = persp_division.z;
            float bias = max(0.01 * (1.0 - dot(normal, lightDirection)), 0.005);

            float shadow = 0.0;

            vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
            for(int x = -1; x <= 1; ++x) {
                for(int y = -1; y <= 1; ++y) {
                    float pcfDepth = texture(shadow_map, persp_division.xy + vec2(x, y) * texelSize).r;
                    shadow += depth - bias > pcfDepth ? 1.0 : 0.0;
                }
            }
            shadow /= 9.0;

            if (persp_division.z > 1.0) {
                shadow = 0.0f;
            }

            return shadow;
        }

        vec4 find_light_color() {
            float shadowFactor = find_shadow_intensity(light);
            if (red_shadows && shadowFactor > 0.0005) {
                return vec4(1, 0, 0, 1);
            }

            vec3 direction = normalize(-light.light_direction);
            vec4 ambientColor = vec4(light.light_color, 1.0f) * light.ambient;
            float diffuseFactor = max(dot(normalize(normal), normalize(direction)), 0.0f);
            vec4 diffuseColor = vec4(light.light_color * light.diffuse * diffuseFactor, 1.0f);
            vec4 specularColor = vec4(0, 0, 0, 0);

            if (diffuseFactor > 0.0f) {
                vec3 cameraToFrag = normalize(camera_pos - frag_pos);
                vec3 reflectedVertex = normalize(reflect(-direction, normalize(normal)));

                float specularFactor = max(dot(cameraToFrag, reflectedVertex), 0.0f);
                specularFactor = pow(specularFactor, shininess);
                specularColor = vec4(light.light_color * specular * specularFactor, 1.0f);
            }

            return (ambientColor + (1 - shadowFactor) * (diffuseColor + specularColor));
        }

        void main() {
            vec3 I = normalize(frag_pos - camera_pos);
            vec3 R = refract(I, normalize(normal), refraction);
            vec4 finalColor = find_light_color();

            if (is_env_mapping) {
                outColor = vec4(texture(skybox, R).rgb, 1.0);
            } else {
                outColor = v_color * vec4(color, 1.0f) * finalColor;
            }
    })glsl";

    const std::string shadow_vertex_shader = R"glsl(
        #version 330

        in vec3 pos;

        uniform mat4 model;
        uniform mat4 light_transform;

        void main() {
            gl_Position = light_transform * model * vec4(pos, 1.0);
        })glsl";

    const std::string shadow_fragment_shader = R"glsl(
        #version 330 core
        void main() {} // deliberately blank
    )glsl";

    const std::string skybox_vertex_shader = R"glsl(
        #version 330 core

        in vec3 aPos;

        out vec3 TexCoords;

        uniform mat4 proj;
        uniform mat4 view;

        void main() {
            TexCoords = aPos;
            vec4 pos = proj * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        })glsl";

    const std::string skybox_fragment_shader = R"glsl(
        #version 330 core
        out vec4 FragColor;

        in vec3 TexCoords;

        uniform samplerCube skybox;

        void main() {    
            FragColor = texture(skybox, TexCoords);
        })glsl";

    main_shader.init_from_strings(vertex_shader, fragment_shader);

    shadow_shader.init_from_strings(shadow_vertex_shader, shadow_fragment_shader);
    skybox_shader.init_from_strings(skybox_vertex_shader, skybox_fragment_shader);
}

void load_model_files() {
    load_off(bunny_indices, bunny_vertices, bunny);
    load_off(bumpy_cube_indices, bumpy_cube_vertices, bumpyCube);
    load_off(cube_indices, cube_vertices, unitCube);
    find_face_normals(bunny_indices, bunny_vertices);
    find_face_normals(bumpy_cube_indices, bumpy_cube_vertices);
    find_face_normals(cube_indices, cube_vertices);
}

void render_models() {
    for(size_t i = 0; i < models.size(); i++) {
        models[i]->env_mapping ? glUniform1i(uniform_is_env_mapping, GL_TRUE) : glUniform1i(uniform_is_env_mapping, GL_FALSE);
        glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(models[i]->get_model()));
        if (selected_object == i) {
            glUniform3f(uniform_color, 0.5f, 0.9f, 0.5f);
            glUniform1i(uniform_is_env_mapping, GL_FALSE);
            models[i]->render_model();
        }
        else {
            glUniform3f(uniform_color, 1.0f, 1.0f, 1.0f);
            models[i]->render_model();
        }
    }
}

void render_plane_and_models() { // render the plane for the shadows
    glUniform3f(uniform_color, 1.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(uniform_is_env_mapping, GL_FALSE);
    glBindVertexArray(plane_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    render_models();
}

void render_light() {
    shadow_shader.use();
    uniform_model = shadow_shader.get_model_location();
    shadow_shader.set_light_matrix(light.get_matrix());

    glViewport(0, 0, light.get_shadow_map()->shadow_width, light.get_shadow_map()->shadow_height);
    light.get_shadow_map()->bindFramebuffer();
    glClear(GL_DEPTH_BUFFER_BIT);

    shadow_shader.validate();
    render_plane_and_models();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void load_uniforms() {
    GLuint shader_ID = main_shader.shader_ID;
    uniform_light_color = glGetUniformLocation(shader_ID, "light.light_color");
    uniform_light_dir = glGetUniformLocation(shader_ID, "light.light_direction");
    uniform_ambient = glGetUniformLocation(shader_ID, "light.ambient");
    uniform_diffuse = glGetUniformLocation(shader_ID, "light.diffuse");
    
    uniform_model = glGetUniformLocation(shader_ID, "model");
    uniform_proj = glGetUniformLocation(shader_ID, "proj");
    uniform_view = glGetUniformLocation(shader_ID, "view");
    uniform_color = glGetUniformLocation(shader_ID, "color");
    uniform_camera_pos = glGetUniformLocation(shader_ID, "camera_pos");
    uniform_specular = glGetUniformLocation(shader_ID, "specular");
    uniform_shininess = glGetUniformLocation(shader_ID, "shininess");
    uniform_red_shadows = glGetUniformLocation(shader_ID, "red_shadows");
    uniform_is_env_mapping = glGetUniformLocation(shader_ID, "is_env_mapping");
    uniform_refract = glGetUniformLocation(shader_ID, "refraction");
}

void render_world() {
    glViewport(0, 0, window_width, window_height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    load_uniforms();
    main_shader.use();

    glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(camera.get_view_matrix()));
    glUniform3f(uniform_camera_pos, camera.get_position().x, camera.get_position().y, camera.get_position().z);
    red_shadows ? glUniform1i(uniform_red_shadows, GL_TRUE) : glUniform1i(uniform_red_shadows, GL_FALSE);
    env_mapping ? glUniform1i(uniform_is_env_mapping, GL_TRUE) : glUniform1i(uniform_is_env_mapping, GL_FALSE);
    glUniform1f(uniform_refract, refraction);

    light.direction.x = 3 * glm::sin(circle_progress);
    light.direction.z = 3 * glm::cos(circle_progress);
    circle_progress += 0.01;

    light.activate(uniform_light_color, uniform_ambient, uniform_diffuse, uniform_light_dir);

    glUniform1f(uniform_specular, 0.9f);
    glUniform1f(uniform_shininess, 120);

    main_shader.set_texture(1);
    main_shader.set_shadow_map(2);

    main_shader.set_light_matrix(light.get_matrix());
    light.get_shadow_map()->useTexture(GL_TEXTURE2);

    main_shader.validate();
    render_plane_and_models();
}