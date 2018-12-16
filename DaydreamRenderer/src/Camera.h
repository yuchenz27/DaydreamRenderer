#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>

enum CameraType {
	NONE, FP, Sphere
};

class Camera {
protected:
	// TODO: do enum here for camera type
	CameraType m_Type;
	// extrinsic parameters
	glm::vec3 m_Position;
	// for FP camera, this should be front; for Sphere camera, this should be target
	glm::vec3 m_Up;

	// intrinsic parameters
	float m_FOV;
	float m_AspectRatio;
	float m_Near;
	float m_Far;
	// we have to compute view matrix every frame, but we don't need to do it for projection matrix, luckily
	glm::mat4 m_ProjectionMatrix;

	// for mouse input
	bool m_IsFirstFrame;
	float m_LastMouseX;
	float m_LastMouseY;
	float m_MouseSensitivity;

	// delta time, to ensure a device-independent frame rate
	float m_DeltaTime;
	float m_LastFrame;

public:
	Camera(glm::vec3 position, glm::vec3 up, float fov, float aspectRatio, float near, float far);
	void Move(glm::vec3 displacement);
	// this method should be called every time the intrinsic parameters modified
	void UpdateProjectionMatrix();
	void UpdateDeltaTime();
	void ProcessKeyboardInputDefault(GLFWwindow* window);
	virtual void ProcessKeyboardInput(GLFWwindow* window) = 0;

	// setters
	void SetPosition(glm::vec3 position);
	void SetUp(glm::vec3 up);
	void SetFOV(float fov);
	void SetAspectRatio(float aspectRatio);
	void SetNear(float v);
	void SetFar(float v);
	void GameStart();
	void SetLastMouseX(float mouseX);
	void SetLastMouseY(float mouseY);
	void SetMouseSensitivity(float v);
	void SetDeltaTime(float deltaTime);
	void SetLastFrame(float lastFrame);

	// getters
	glm::vec3 GetPosition();
	glm::vec3 GetUp();
	float GetFOV();
	float GetAspectRatio();
	float GetNear();
	float GetFar();
	virtual glm::mat4 GetViewMatrix() = 0;
	glm::mat4 GetProjectionMatrix();
	bool IsFirstFrame();
	float GetLastMouseX();
	float GetLastMouseY();
	float GetMouseSensitivity();
	float GetDeltaTime();
	float GetLastFrame();
	CameraType GetType();
};