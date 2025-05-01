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

#include "Boat.h"


class Camera {
private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float movementSpeed;
	float mouseSensitivity;
	float zoom;

public:
	Camera(glm::vec3 pos = glm::vec3(0.0f, 5.0f, 15.0f));

	void processKeyboard(int direction, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void processMouseScroll(float yoffset);

	glm::mat4 getViewMatrix();
	float getZoom();

	void followBoat(const Boat& boat, float distance, float height);
private:
	void updateCameraVectors();
};


#endif // !CAMERA_CLASS_H
