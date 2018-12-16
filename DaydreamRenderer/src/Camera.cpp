#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float fov, float aspectRatio, float near, float far) {
	m_Type = NONE;
	m_Position = position;
	m_Up = up;
	m_FOV = fov;
	m_AspectRatio = aspectRatio;
	m_Near = near;
	m_Far = far;

	m_IsFirstFrame = true;
	m_MouseSensitivity = 0.08f;
	m_DeltaTime = 0.0f;
	m_LastFrame = 0.0f;

	UpdateProjectionMatrix();
}

void Camera::Move(glm::vec3 displacement) {
	m_Position += displacement;
}

void Camera::UpdateProjectionMatrix() {
	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_Near, m_Far);
}

void Camera::UpdateDeltaTime() {
	float currentFrame = glfwGetTime();
	m_DeltaTime = currentFrame - m_LastFrame;
	m_LastFrame = currentFrame;
}

void Camera::ProcessKeyboardInputDefault(GLFWwindow* window) {
	// press ESC to exit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	// report the current world position
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		std::cout << "The current world position is: (" + std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + ", " + std::to_string(m_Position.z) + ")" << std::endl;
	}
}

void Camera::SetPosition(glm::vec3 position) {
	m_Position = position;
}

void Camera::SetUp(glm::vec3 up) {
	m_Up = up;
}

void Camera::SetFOV(float fov) {
	m_FOV = fov;
	UpdateProjectionMatrix();
}

void Camera::SetAspectRatio(float aspectRatio) {
	m_AspectRatio = aspectRatio;
	UpdateProjectionMatrix();
}

void Camera::SetNear(float v) {
	m_Near = v;
	UpdateProjectionMatrix();
}

void Camera::SetFar(float v) {
	m_Far = v;
	UpdateProjectionMatrix();
}

void Camera::GameStart() {
	m_IsFirstFrame = false;
}

void Camera::SetLastMouseX(float mouseX) {
	m_LastMouseX = mouseX;
}

void Camera::SetLastMouseY(float mouseY) {
	m_LastMouseY = mouseY;
}

void Camera::SetMouseSensitivity(float v) {
	m_MouseSensitivity = v;
}

void Camera::SetDeltaTime(float deltaTime) {
	m_DeltaTime = deltaTime;
}

void Camera::SetLastFrame(float lastFrame) {
	m_LastFrame = lastFrame;
}

glm::vec3 Camera::GetPosition() {
	return m_Position;
}

glm::vec3 Camera::GetUp() {
	return m_Up;
}

float Camera::GetFOV() {
	return m_FOV;
}

float Camera::GetAspectRatio() {
	return m_AspectRatio;
}

float Camera::GetNear() {
	return m_Near;
}

float Camera::GetFar() {
	return m_Far;
}

glm::mat4 Camera::GetProjectionMatrix() {
	return m_ProjectionMatrix;
}

bool Camera::IsFirstFrame() {
	return m_IsFirstFrame;
}

float Camera::GetLastMouseX() {
	return m_LastMouseX;
}

float Camera::GetLastMouseY() {
	return m_LastMouseY;
}

float Camera::GetMouseSensitivity() {
	return m_MouseSensitivity;
}

float Camera::GetDeltaTime() {
	return m_DeltaTime;
}

float Camera::GetLastFrame() {
	return m_LastFrame;
}

CameraType Camera::GetType() {
	return m_Type;
}
