#include "Camera.h"
#include "shaderClass.h"

Camera::Camera(int width, int height, glm::vec3 position) {
	this->width = width;
	this->height = height;
	this->Position = position;
}

void Camera::updateCameraMatrix(float FOVdeg, float nearPlane, float farPlane) {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Make sure camera is looking in the right direction
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);
	view = glm::lookAt(Position, Position + Orientation, Up);

	cameraMatrix = projection * view; // Combine the two matrices
}

void Camera::Matrix(Shader& shader, const char* uniform) {
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Input(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Position += Orientation * speed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Position -= Orientation * speed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Position -= glm::normalize(glm::cross(Orientation, Up)) * speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Position += glm::normalize(glm::cross(Orientation, Up)) * speed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		Position += Up * speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		Position -= Up * speed;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, width / 2, height / 2);
			firstClick = false;
		}

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		float rotx = (float)(ypos - height / 2) / height;
		float roty = (float)(xpos - height / 2) / height;

		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotx * sensitivity), glm::normalize(glm::cross(Orientation, Up)));
		if (glm::angle(newOrientation, Up) - glm::radians(90.0f) <= glm::radians(85.0f)) {
			Orientation = newOrientation;
		}

		Orientation = glm::rotate(Orientation, glm::radians(-roty * sensitivity), Up);

		glfwSetCursorPos(window, width / 2, height / 2);
	}	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		firstClick = true;
	}
}


