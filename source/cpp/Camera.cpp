#include "../header/Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch, float near, float far)
	:forward(glm::vec3(0.0f, 0.0f, -1.0f)), speed(SPEED), sensitivity(SENSITIVITY), fov(FOV) 
{
	this->pos = pos;
	this->up = up;
	this->worldUp = up;
	this->yaw = yaw;
	this->pitch = pitch;
	this->near = near;
	this->far = far;
	UpdateCamera();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float near, float far)
	:forward(glm::vec3(0.0f, 0.0f, -1.0f)), speed(SPEED), sensitivity(SENSITIVITY), fov(FOV) 
{
	this->pos = glm::vec3(posX, posY, posZ);
	this->up = glm::vec3(upX, upY, upZ);
	this->worldUp = up;
	this->yaw = yaw;
	this->pitch = pitch;
	this->near = near;
	this->far = far;
	UpdateCamera();
}

glm::mat4 Camera::GetViewMat()
{
	return glm::lookAt(pos, pos + forward, up);
}

void Camera::ProcessMousePan(float xOffset, float yOffset)
{
	yaw += xOffset;
	pitch += yOffset;

	//Constraints camera not to flip axis
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	UpdateCamera();
}

void Camera::ProcessMouseScroll(float yOffset)
{
	fov -= (float)yOffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
	UpdateCamera();
}

void Camera::ProcessKeyBoard(MoveDirection direction, float deltaTime)
{
	float cameraSpeedNormalized = speed * deltaTime;
	switch (direction) {
		case FORWARD:
			pos += cameraSpeedNormalized * forward;
			return;
		case BACKWARD:
			pos -= cameraSpeedNormalized * forward;
			return;
		case LEFT:
			pos -= cameraSpeedNormalized * right;
			return;
		case RIGHT:
			pos += cameraSpeedNormalized * right;
			return;
		default:
			return;
			
	}
}

void Camera::UpdateCamera()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(direction);
	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
}
