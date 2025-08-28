#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum MoveDirection {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;
const float NEAR = 0.1f;
const float FAR = 100.0f;

class Camera {

public:
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 worldUp;
	glm::vec3 right;
	float yaw, pitch;
	float speed;
	float sensitivity;
	float fov;
	float near;
	float far;

	Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float near = NEAR, float far = FAR);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float near = NEAR, float far = FAR);
	
	void ProcessMousePan(float xOffset, float yOffset);
	void ProcessMouseScroll(float yOffset);
	void ProcessKeyBoard(MoveDirection direction, float deltaTime);

	glm::mat4 GetViewMat();

private:
	void UpdateCamera();


};

#endif