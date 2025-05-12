#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H
// Define GLM_ENABLE_EXPERIMENTAL before including the header
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "shaderClass.h"


class Camera {
public:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 cameraMatrix = glm::mat4(1.0f);

	int width;
	int height;

	float speed = 0.0005f;
	float sensitivity = 100.0f;

	bool firstClick = true;


	Camera(int width, int height, glm::vec3 position);

	void updateCameraMatrix(float FOVdeg, float nearPlane, float farPlane);
	void Matrix(Shader& shader, const char* uniform);
	void Input(GLFWwindow* window);
    
};


#endif // !CAMERA_CLASS_H
