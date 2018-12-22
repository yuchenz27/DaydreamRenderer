#include "SphereCamera.h"

SphereCamera::SphereCamera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov, float aspectRatio, float near, float far, float radius) : Camera(position, up, fov, aspectRatio, near, far) {
	m_Type = Sphere;
	m_Target = target;

	m_Phi = 90.0f;
	m_Theta = 0.0f;
	m_Radius = radius;
	m_IsUpsidedown = false;
	m_PhiRange = 0;

	UpdatePosition();
}

void SphereCamera::SetTarget(glm::vec3 target)
{
	m_Target = target;
}

void SphereCamera::SetRadius(float v) {
	m_Radius = v;
}

void SphereCamera::UpdateTheta(float v)
{
	m_Theta += v;
}

void SphereCamera::UpdatePhi(float v)
{
	m_Phi += v;
	// TODO: is there is better way to do this?
	if (m_Phi < (m_PhiRange * 180.0f)) {
		m_IsUpsidedown = !m_IsUpsidedown;
		m_PhiRange--;
	}
	else if (m_Phi > ((m_PhiRange + 1) * 180.0f)) {
		m_IsUpsidedown = !m_IsUpsidedown;
		m_PhiRange++;
	}
}

void SphereCamera::SetTheta(float v)
{
	m_Theta = v;
}

void SphereCamera::SetPhi(float v)
{
	m_Phi = v;
}

float SphereCamera::GetTheta()
{
	return m_Theta;
}

float SphereCamera::GetPhi()
{
	return m_Phi;
}

glm::vec3 SphereCamera::GetTarget()
{
	return m_Target;
}

void SphereCamera::UpdatePosition()
{
	// http://tutorial.math.lamar.edu/Classes/CalcIII/SphericalCoords.aspx
	m_Position.z = m_Radius * sin(glm::radians(m_Phi)) * cos(glm::radians(m_Theta));
	m_Position.x = m_Radius * sin(glm::radians(m_Phi)) * sin(glm::radians(m_Theta));
	m_Position.y = m_Radius * cos(glm::radians(m_Phi));
	
	// the camera view direction cannot be parallel to its up vector
	// TODO: is there is better way to solve the precision problem?
	//if (abs(m_CameraPosition.x) < 0.00000001f && abs(m_CameraPosition.z) < 0.00000001f) {
	if (m_Phi == 0.0f || m_Phi == 180.0f) {
		m_Up = glm::normalize(glm::vec3(sin(glm::radians(m_Theta)), 0.0f, cos(glm::radians(m_Theta))));
		if (m_Position.y > 0.0f)
			m_Up = -m_Up;
	}
	else {
		m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (m_IsUpsidedown)
		m_Up = -m_Up;
}

void SphereCamera::UpdateRadius(float v)
{
	m_Radius += v;
	if (m_Radius <= 0.0f)
		m_Radius = 0.1f;
}

void SphereCamera::ProcessKeyboardInput(GLFWwindow * window) {
	ProcessKeyboardInputCommon(window);
}

glm::mat4 SphereCamera::GetViewMatrix() {
	return glm::lookAt(m_Position, m_Target, m_Up);
}

float SphereCamera::GetRadius()
{
	return m_Radius;
}
