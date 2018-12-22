#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

class FPCamera : public Camera {
private:
	glm::vec3 m_Front;

	float m_Pitch;
	float m_Yaw;

	// for keyboard input
	float m_CameraSpeed;

public:
	FPCamera(glm::vec3 position, glm::vec3 front, glm::vec3 up, float fov, float aspectRatio, float near, float far);
	void UpdatePitch(float v);
	void UpdateYaw(float v);
	void UpdataFront();
	void ProcessKeyboardInput(GLFWwindow* window) override;
	glm::mat4 GetViewMatrix() override;

	void SetFront(glm::vec3 front);
	void SetPitch(float v);
	void SetYaw(float v);
	void SetCameraSpeed(float v);

	glm::vec3 GetFront();
	float GetPitch();
	float GetYaw();
	float GetCameraSpeed();
};