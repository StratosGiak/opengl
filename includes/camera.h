#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum cameraDirection {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    CLOCK,
    ANTICLOCK
};

class Camera {
   public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
    float movementSpeed = 2.5f;
    float mouseSensitivity = 0.1f;
    float FOV = 90.0f;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f))
        : position(position), worldUp(up), front(front) {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    void move(cameraDirection direction, float deltaTime, bool fast = false) {
        float velocity = movementSpeed * deltaTime;
        if (fast) velocity *= 5;
        // glm::vec3 directionVector;
        if (direction == FORWARD) position += front * velocity;
        if (direction == BACKWARD) position -= front * velocity;
        if (direction == LEFT) position -= right * velocity;
        if (direction == RIGHT) position += right * velocity;
        if (direction == UP) position += worldUp * velocity;
        if (direction == DOWN) position -= worldUp * velocity;
        if (direction == CLOCK) {
            roll += 2 * mouseSensitivity;
            updateCameraVectors();
        }
        if (direction == ANTICLOCK) {
            roll -= 2 * mouseSensitivity;
            updateCameraVectors();
        }
    }

    void resetUp() {
        roll = 0;
        updateCameraVectors();
    }

    void processMouseMovement(float offsetX, float offsetY) {
        offsetX *= mouseSensitivity;
        offsetY *= mouseSensitivity;

        yaw += offsetX * cos(glm::radians(roll)) +
               offsetY * sin(glm::radians(roll));
        pitch += -offsetX * sin(glm::radians(roll)) +
                 offsetY * cos(glm::radians(roll));
        if (pitch > 85.0f) pitch = 85.0f;
        if (pitch < -85.0f) pitch = -85.0f;

        updateCameraVectors();
    }

    void processMouseScroll(float scrollAmount) {
        FOV -= scrollAmount;
        if (FOV < 30.0f) FOV = 30.0f;
        if (FOV > 110.0f) FOV = 110.0f;
    }

   private:
    void updateCameraVectors() {
        front = glm::normalize(
            glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                      sin(glm::radians(pitch)),
                      sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
        right = glm::normalize(glm::cross(
            front,
            glm::vec3(sin(glm::radians(roll)), cos(glm::radians(roll)), 0.0f)));
        up = glm::normalize(glm::cross(right, front));
    }
};

#endif