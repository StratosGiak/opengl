#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shader.h>
#include <camera.h>
#include <model.h>

#include <algorithm>
#include <format>
#include <random>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

void error_cb(int error, const char *description) {
    std::cerr << "ERROR: " << description << std::endl;
}

void closeWindow_cb(GLFWwindow *window) {
    std::cout << "Closing window..." << std::endl;
    glfwDestroyWindow(window);
}

GLFWwindow *createBorderlessWindow(const char *title) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    return glfwCreateWindow(mode->width, mode->height, title, monitor, nullptr);
}

void createTexture(const char *path, GLuint &texture, GLint format) {
    int texture_width, texture_height, channels;
    unsigned char *texture_data =
        stbi_load(path, &texture_width, &texture_height, &channels, 0);
    if (!texture_data) {
        stbi_image_free(texture_data);
        exit(EXIT_FAILURE);
    }
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0,
                 format, GL_UNSIGNED_BYTE, texture_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texture_data);
}

int width = 800;
int height = 600;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float deltaTime;
float lastFrame;
float lastX = width / 2;
float lastY = height / 2;
bool firstMouse = true;

glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::perspective(glm::radians(camera.FOV),
                                        (float)width / height, 0.1f, 100.0f);

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float offsetX = xpos - lastX;
    float offsetY = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(offsetX, offsetY);
}

void scroll_callback(GLFWwindow *window, double offsetX, double offsetY) {
    camera.processMouseScroll(offsetY);
    projection = glm::perspective(glm::radians(camera.FOV),
                                  (float)width / height, 0.1f, 100.0f);
}

static void processInput(GLFWwindow *window) {
    const bool fast = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.move(cameraDirection::FORWARD, deltaTime, fast);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.move(cameraDirection::BACKWARD, deltaTime, fast);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.move(cameraDirection::LEFT, deltaTime, fast);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.move(cameraDirection::RIGHT, deltaTime, fast);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.move(cameraDirection::UP, deltaTime, fast);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.move(cameraDirection::DOWN, deltaTime, fast);
}

static void framebuffer_callback(GLFWwindow *window, int newWidth,
                                 int newHeight) {
    width = newWidth;
    height = newHeight;
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(camera.FOV),
                                  (float)width / height, 0.1f, 100.0f);
}

float lastFrameFPS;
int frames = 0;
static void printFPS() {
    float current = glfwGetTime();
    float dt = current - lastFrameFPS;
    ++frames;
    if (dt >= 1.0) {
        float fps = frames / dt;
        std::cout << fps << std::endl;
        frames = 0;
        lastFrameFPS = current;
    }
}

// clang-format off
GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
};


glm::vec3 pointLightPositions[] = {
	glm::vec3( 0.7f,  0.2f,  2.0f),
	glm::vec3( 2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3( 0.0f,  0.0f, -3.0f)
};
// clang-format on

float lightSpeed = 30.0f;
float radius = 5.0f;

std::random_device rand_dev;
std::mt19937 rng(rand_dev());
std::uniform_real_distribution<float> dist0_1(0.0f, 1.0f);
std::uniform_real_distribution<float> dist0_10(0.0f, 50.0f);

int main(int argc, char **argv) {
    glfwSetErrorCallback(error_cb);
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    stbi_set_flip_vertically_on_load(true);

    GLFWwindow *window =
        glfwCreateWindow(width, height, "Hello!", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    glewInit();

    glfwSetFramebufferSizeCallback(window, framebuffer_callback);
    glClearColor(0.05f, 0.08f, 0.1f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    const GLubyte *renderer = glGetString(GL_RENDERER);  // get renderer string
    const GLubyte *version = glGetString(GL_VERSION);    // version as a string
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    Shader shader("./shaders/vertex.vert", "./shaders/fragment.frag");
    Shader lightShader("./shaders/vertex.vert", "./shaders/fragment2.frag");

    GLuint textureDiffuse;
    createTexture("./textures/container.png", textureDiffuse, GL_RGBA);
    GLuint textureSpecular;
    createTexture("./textures/container_specular.png", textureSpecular,
                  GL_RGBA);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void *)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    for (size_t i = 0; i < 10; i++) {
        std::string propertyString = std::format("pointLights[{}].", i);
        std::string strings[] = {
            propertyString + "position",     propertyString + "ambient",
            propertyString + "diffuse",      propertyString + "specular",
            propertyString + "coefficients",
        };
        glProgramUniform3fv(shader.id,
                            glGetUniformLocation(shader.id, strings[0].data()),
                            1, &pointLightPositions[i][0]);
        glProgramUniform3f(shader.id,
                           glGetUniformLocation(shader.id, strings[1].data()),
                           0.0f, 0.0f, 0.0f);
        glProgramUniform3f(shader.id,
                           glGetUniformLocation(shader.id, strings[2].data()),
                           0.5f, 0.5f, 0.5f);
        glProgramUniform3f(shader.id,
                           glGetUniformLocation(shader.id, strings[3].data()),
                           1.0f, 1.0f, 1.0f);
        glProgramUniform3f(shader.id,
                           glGetUniformLocation(shader.id, strings[4].data()),
                           1.0f, 0.09f, 0.002f);
    }

    glProgramUniform3f(shader.id,
                       glGetUniformLocation(shader.id, "directedLight.ambient"),
                       0.05f, 0.05f, 0.05f);
    glProgramUniform3f(shader.id,
                       glGetUniformLocation(shader.id, "directedLight.diffuse"),
                       0.4f, 0.4f, 0.4f);
    glProgramUniform3f(
        shader.id, glGetUniformLocation(shader.id, "directedLight.specular"),
        0.5f, 0.5f, 0.5f);
    glProgramUniform3f(
        shader.id, glGetUniformLocation(shader.id, "directedLight.direction"),
        -0.2f, -1.0f, -0.3f);

    glProgramUniform3f(shader.id,
                       glGetUniformLocation(shader.id, "spotLight.ambient"),
                       0.0f, 0.0f, 0.0f);
    glProgramUniform3f(shader.id,
                       glGetUniformLocation(shader.id, "spotLight.diffuse"),
                       0.5f, 0.5f, 0.5f);
    glProgramUniform3f(shader.id,
                       glGetUniformLocation(shader.id, "spotLight.specular"),
                       1.0f, 1.0f, 1.0f);
    glProgramUniform3f(
        shader.id, glGetUniformLocation(shader.id, "spotLight.coefficients"),
        1.0f, 0.09f, 0.002f);
    glProgramUniform1f(shader.id,
                       glGetUniformLocation(shader.id, "spotLight.cutoff"),
                       cos(glm::radians(20.0f)));
    glProgramUniform1f(shader.id,
                       glGetUniformLocation(shader.id, "spotLight.outerCutoff"),
                       cos(glm::radians(30.0f)));

    glProgramUniform1i(shader.id,
                       glGetUniformLocation(shader.id, "material.diffuse"), 0);
    glProgramUniform1i(shader.id,
                       glGetUniformLocation(shader.id, "material.specular"), 1);
    glProgramUniform3f(shader.id,
                       glGetUniformLocation(shader.id, "material.specular"),
                       0.5f, 0.5f, 0.5f);
    glProgramUniform1f(
        shader.id, glGetUniformLocation(shader.id, "material.shiny"), 32.0f);

    float angles[10][3];
    for (size_t i = 0; i < 10; i++) {
        for (size_t j = 0; j < 3; j++) {
            angles[i][j] = dist0_1(rng);
        }
    }
    float speeds[10];
    for (size_t i = 0; i < 10; i++) {
        speeds[i] = dist0_10(rng) + 10;
    }

    lastFrame = lastFrameFPS = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double time = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const float currentFrame = time;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        printFPS();
        processInput(window);

        // cubePositions[0] =
        //     glm::vec3(radius * cos(glm::radians(time * lightSpeed)),
        //               radius / 4 * sin(glm::radians(time * lightSpeed
        //               * 4.0f)), -7 + radius / 4 * sin(glm::radians(time *
        //               lightSpeed)));

        view = camera.getViewMatrix();

        glProgramUniformMatrix4fv(lightShader.id,
                                  glGetUniformLocation(lightShader.id, "view"),
                                  1, GL_FALSE, glm::value_ptr(view));
        glProgramUniformMatrix4fv(
            lightShader.id, glGetUniformLocation(lightShader.id, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        glProgramUniformMatrix4fv(shader.id,
                                  glGetUniformLocation(shader.id, "view"), 1,
                                  GL_FALSE, glm::value_ptr(view));
        glProgramUniformMatrix4fv(shader.id,
                                  glGetUniformLocation(shader.id, "projection"),
                                  1, GL_FALSE, glm::value_ptr(projection));
        glProgramUniform3fv(
            shader.id, glGetUniformLocation(shader.id, "spotLight.position"), 1,
            &camera.position[0]);
        glProgramUniform3fv(
            shader.id, glGetUniformLocation(shader.id, "spotLight.direction"),
            1, &camera.front[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureSpecular);

        lightShader.use();
        glBindVertexArray(lightVAO);
        for (size_t i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            glProgramUniformMatrix4fv(
                lightShader.id, glGetUniformLocation(lightShader.id, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        shader.use();
        glBindVertexArray(vao);
        for (size_t i = 0; i < 10; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(
                model, (float)time * glm::radians(speeds[i]),
                glm::vec3(angles[i][0], angles[i][1], angles[i][2]));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            glProgramUniformMatrix4fv(shader.id,
                                      glGetUniformLocation(shader.id, "model"),
                                      1, GL_FALSE, glm::value_ptr(model));
            glm::mat3 normalModel =
                glm::mat3(glm::transpose(glm::inverse(model)));
            glProgramUniformMatrix3fv(
                shader.id, glGetUniformLocation(shader.id, "normalModel"), 1,
                GL_FALSE, glm::value_ptr(normalModel));
            glProgramUniform3fv(shader.id,
                                glGetUniformLocation(shader.id, "viewPos"), 1,
                                &camera.position[0]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "Closing window..." << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}