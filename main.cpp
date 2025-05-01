#ifndef M_PI  
#define M_PI 3.14159265358979323846  
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>

#include "Shader.h"
#include "Sea.h"
#include "Boat.h"
#include "Camera.h"

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Camera instance
Camera camera(glm::vec3(0.0f, 3.0f, 10.0f));
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Boat movement
bool moveBoat = false;
bool rotateBoatLeft = false;
bool rotateBoatRight = false;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Boat Simulation", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);

    // Create and compile shaders
    Shader seaShader("Shaders/sea.vert", "Shaders/sea.frag");
    Shader boatShader("Shaders/boat.vert", "Shaders/boat.frag");

    // Create sea mesh
    Sea sea(100, 100.0f);

    // Create boat
    Boat boat(1.0f, 3.0f, 0.8f);
    boat.setPosition(0.0f, 0.0f);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Update sea waves
        sea.updateWaves(deltaTime);

        // Update boat position and rotation
        boat.update(sea, deltaTime);

        // Handle boat movement
        if (moveBoat) {
            boat.moveForward(2.0f * deltaTime);
        }
        if (rotateBoatLeft) {
            boat.rotate(-50.0f * deltaTime);
        }
        if (rotateBoatRight) {
            boat.rotate(50.0f * deltaTime);
        }

        // Clear buffers
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), 800.0f / 600.0f, 0.1f, 100.0f);

        // Create view matrix
        glm::mat4 view = camera.getViewMatrix();

        // Draw sea
        seaShader.use();
        seaShader.setMat4("projection", projection);
        seaShader.setMat4("view", view);
        seaShader.setMat4("model", glm::mat4(1.0f));
        seaShader.setVec3("lightPos", glm::vec3(10.0f, 10.0f, 10.0f));
        seaShader.setVec3("viewPos", camera.getViewMatrix()[3]);
        seaShader.setFloat("time", currentFrame);
        sea.draw();

        // Draw boat
        boatShader.use();
        boatShader.setMat4("projection", projection);
        boatShader.setMat4("view", view);
        boatShader.setVec3("lightPos", glm::vec3(10.0f, 10.0f, 10.0f));
        boatShader.setVec3("viewPos", camera.getViewMatrix()[3]);
        boat.draw(boatShader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processMouseScroll(yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(3, deltaTime);

    // Boat controls
    moveBoat = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    rotateBoatLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    rotateBoatRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
}