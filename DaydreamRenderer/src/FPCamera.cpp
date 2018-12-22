#include "FPCamera.h"

FPCamera::FPCamera(glm::vec3 position, glm::vec3 front, glm::vec3 up, float fov, float aspectRatio, float near, float far) : Camera(position, up, fov, aspectRatio, near, far) {
	m_Type = FP;
	m_Front = front;

	m_Pitch = 0.0f;
	m_Yaw = -90.0f;
	m_CameraSpeed = 5.0f;
}

void FPCamera::UpdatePitch(float v) {
	m_Pitch += v;
}

void FPCamera::UpdateYaw(float v) {
	m_Yaw += v;
}

void FPCamera::UpdataFront() {
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	newFront.y = sin(glm::radians(m_Pitch));
	newFront.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front = glm::normalize(newFront);
}

void FPCamera::ProcessKeyboardInput(GLFWwindow * window) {
	ProcessKeyboardInputCommon(window);

	float thisFrameSpeed = m_CameraSpeed * m_DeltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		glm::vec3 displacement = thisFrameSpeed * m_Front;
		Move(displacement);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		glm::vec3 displacement = thisFrameSpeed * m_Front;
		Move(-displacement);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		glm::vec3 displacement = thisFrameSpeed * glm::normalize(glm::cross(m_Front, m_Up));
		Move(-displacement);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		glm::vec3 displacement = thisFrameSpeed * glm::normalize(glm::cross(m_Front, m_Up));
		Move(displacement);
	}
	// go up and down
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		glm::vec3 displacement = thisFrameSpeed * m_Up;
		Move(displacement);
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		glm::vec3 displacement = thisFrameSpeed * m_Up;
		Move(-displacement);
	}
}

glm::mat4 FPCamera::GetViewMatrix() {
	return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void FPCamera::SetFront(glm::vec3 front) {
	m_Front = front;
}

void FPCamera::SetPitch(float v) {
	m_Pitch = v;
}

void FPCamera::SetYaw(float v) {
	m_Yaw = v;
}

void FPCamera::SetCameraSpeed(float v) {
	m_CameraSpeed = v;
}

glm::vec3 FPCamera::GetFront() {
	return m_Front;
}

float FPCamera::GetPitch() {
	return m_Pitch;
}

float FPCamera::GetYaw() {
	return m_Yaw;
}

float FPCamera::GetCameraSpeed() {
	return m_CameraSpeed;
}
