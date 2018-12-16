#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

class SphereCamera : public Camera {
private:
	glm::vec3 m_Target;

	// phi is polar angle, and theta is azimuth
	float m_Phi;
	float m_Theta;
	float m_Radius;

	bool m_IsUpsidedown;
	int m_PhiRange;

public:
	SphereCamera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov, float aspectRatio, float near, float far, float radius);
	void UpdatePosition();
	void UpdateTheta(float v);
	void UpdatePhi(float v);
	void UpdateRadius(float v);
	void ProcessKeyboardInput(GLFWwindow* window) override;
	glm::mat4 GetViewMatrix() override;

	void SetTarget(glm::vec3 target);
	void SetRadius(float v);
	void SetTheta(float v);
	void SetPhi(float v);

	glm::vec3 GetTarget();
	float GetRadius();
	float GetTheta();
	float GetPhi();
};
