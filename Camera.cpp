#include "Camera.h"

Camera::Camera(glm::vec3 pos){
    position = pos;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -90.0f;
    pitch = -20.0f;
    movementSpeed = 5.0f;
    mouseSensitivity = 0.1f;
    zoom = 45.0f;
    updateCameraVectors();
}

void Camera::processKeyboard(int direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (direction == 0) // FORWARD
        position += front * velocity;
    if (direction == 1) // BACKWARD
        position -= front * velocity;
    if (direction == 2) // LEFT
        position -= right * velocity;
    if (direction == 3) // RIGHT
        position += right * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

float Camera::getZoom() {
    return zoom;
}

void Camera::followBoat(const Boat& boat, float distance, float height) {
    // Update camera position to follow boat
    // Implementation depends on boat class and how it stores position/rotation
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}


