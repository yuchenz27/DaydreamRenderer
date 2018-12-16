#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include "stb_image.h"

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_glfw.h"
#include "ImGui\imgui_impl_opengl3.h"

#include "Shader.h"
#include "SphereCamera.h"
#include "Helper.h"
#include "PrefabInitialization.h"
#include "Model.h"
#include "GBuffer.h"

#include <iostream>
#include <string>

void FramebufferSizeCallback(GLFWwindow* window, const int width, const int height);
void SphereCameraMouseCallback(GLFWwindow* window, double xpos, double ypos);
void SphereCameraScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

const int SCR_WIDTH = 1920 * 1.6;
const int SCR_HEIGHT = 1080 * 1.6;
SphereCamera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f, 5.0f);

int main()
{
	GLFWwindow* window;
	if (!glfwInit())
		return -1;
	// Enable GLFW multisample buffer
	//glfwWindowHint(GLFW_SAMPLES, 4);4
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Daydream Renderer", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// GLFW configurations
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(window, SphereCameraMouseCallback);
	glfwSetScrollCallback(window, SphereCameraScrollCallback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//-----------------------------------------------------------------------------------------------------------------
	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW ERROR" << std::endl;
		return -1;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	// enable multisample AA
	//glEnable(GL_MULTISAMPLE);
	// enable OpenGL built-in gamma correction for all fragment shaders
	//glEnable(GL_FRAMEBUFFER_SRGB);
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	// ----------------------------------------------------------------------------------------------------------------
	//Model theModel("resources/models/dragon.ply/dragon.obj");
	//Model theModel("resources/models/Dragon/Dragon.obj");
	Model theModel("resources/models/nanosuit/nanosuit.obj");
	theModel.LogTextureInfo();

	// ----------------------------------------------------------------------------------------------------------------
	Shader GoochShader("resources/shaders/GoochShading");
	Shader PhongShader("resources/shaders/PhongShading");
	Shader BlinnPhongShader("resources/shaders/BlinnPhongShading");
	Shader NormalShader("resources/shaders/ShowNormal");
	Shader ShadowMapRenderingShader("resources/shaders/ShadowMapRendering");
	Shader ShadowMapVisualizationShader("resources/shaders/ShadowMapVisualization");
	Shader GeometryPassShader("resources/shaders/GeometryPass");
	Shader QuadDisplayShader("resources/shaders/QuadDisplay");
	Shader DeferredBlinnPhongShader("resources/shaders/DeferredBlinnPhongShading");
	Shader DyedCubeShader("resources/shaders/DyedCube");

	// ----------------------------------------------------------------------------------------------------------------
	// set up shadow map
	const unsigned int SHADOWMAP_WIDTH = SCR_WIDTH;
	const unsigned int SHADOWMAP_HEIGHT = SCR_HEIGHT;
	ShadowMapData shadowMapData = CreateShadowMapData(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	// the shadow map projection matrix
	// left, right, bottom, top, near and far
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 12.5f);
	// set up the quad VAO for visualization purposes
	unsigned int quadVAO = CreateQuadVAO();
	unsigned int cubeVAO = CreateCubeVAO();
	
	// ---------------------------------------------------------------------------------------------------------
	// ImGui setup
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();

	// ---------------------------------------------------------------------------------------------------------
	// set up shading parameters
	// common parameters
	glm::vec3 lightPosition = glm::vec3(15.0f, 15.0f, 15.0f); // currently I am using directional light
	// TODO: add directional light
	// TODO: add multiple light sources
	float scalingFactor = .1f;
	glm::vec3 translatingFactor = theModel.GetCenteringVector();
	// normal visualization
	bool enableNormalVisualization = false;
	float normalMagnitude = 0.05f;
	glm::vec3 normalColor(1.0f, 1.0f, 0.0f);
	// gamma correction
	bool enableGammaCorrection = false;
	float gamma = 2.2f;
	// ground plane
	bool enableGroundPlane = false;
	float planeScalingFactor = 0.2f;
	float planeY = -0.5f;
	// the shader index
	// 0 for Gooch shading, 1 for Phong shading, 2 for BlinnPhong shading
	int shaderIndex = 2;
	// parameters for Gooch shading
	glm::vec3 meshColor(0.8f, 0.8f, 0.9f);
	// parameters for Phong shading
	float brightness = 1.0f;
	float constantFactor = 1.0f;
	float linearFactor = 0.09f;
	float quadraticFactor = 0.032f;
	float shininess = 16.0f;
	bool enableAttenuation = false;
	bool show_demo_window = true;
	bool ifShow = true;
	// shadow map parameters
	bool enableShadowMap = true;
	float shadowBias = .1f;
	
	// ---------------------------------------------------------------------------------------------------------
	GBuffer gBuffer = InitGBuffer(SCR_WIDTH, SCR_HEIGHT);

	// set up all light sources
	const unsigned int NUM_LIGHTS = 32;
	DyedLightSources lightSources = CreateDyedLightSources(NUM_LIGHTS);

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		camera.UpdateDeltaTime();
		camera.ProcessKeyboardInput(window);
		// Start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// --------------------------------------------------------------------------------------------------------------
		// draw calls
		// --------------------------------------------------------------------------------------------------------------12 Dec
		// THE GEOMETRY PASS:
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GeometryPassShader.Bind();
		// figure the uniforms of the shader
		glm::mat4 model;
		model = glm::scale(model, glm::vec3(scalingFactor));
		model = glm::translate(model, translatingFactor);
		GeometryPassShader.SetUniformMatrix4fv("model", model);
		GeometryPassShader.SetUniformMatrix4fv("view", camera.GetViewMatrix());
		GeometryPassShader.SetUniformMatrix4fv("projection", camera.GetProjectionMatrix());
		// draw the model
		theModel.Draw(GeometryPassShader);
		// unbind the G-buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// OPTIONAL: visualize the G-buffer
		/*
		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		QuadDisplayShader.Bind();
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		*/

		// THE LIGHTING PASS:
		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.positionTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.normalTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpecTexture);
		// shader configuration
		DeferredBlinnPhongShader.Bind();
		DeferredBlinnPhongShader.SetUniform1i("gPosition", 0);
		DeferredBlinnPhongShader.SetUniform1i("gNormal", 1);
		DeferredBlinnPhongShader.SetUniform1i("gAlbedoSpec", 2);
		DeferredBlinnPhongShader.SetUniformVector3fv("viewPos", camera.GetPosition());
		DeferredBlinnPhongShader.SetUniform1f("shininess", shininess);
		for (int i = 0; i < 2; i++) {
			std::string thisLightPosition = "lights[" + std::to_string(i) + "].position";
			std::string thisLightColor = "lights[" + std::to_string(i) + "].color";
			std::string thisLightConstant = "lights[" + std::to_string(i) + "].Constant";
			std::string thisLightLinear = "lights[" + std::to_string(i) + "].Linear";
			std::string thisLightQuadratic = "lights[" + std::to_string(i) + "].Quadratic";
			DeferredBlinnPhongShader.SetUniformVector3fv(thisLightPosition.c_str(), lightSources.positions[i]);
			DeferredBlinnPhongShader.SetUniformVector3fv(thisLightColor.c_str(), lightSources.colors[i]);
			DeferredBlinnPhongShader.SetUniform1f(thisLightConstant.c_str(), constantFactor);
			DeferredBlinnPhongShader.SetUniform1f(thisLightLinear.c_str(), linearFactor);
			DeferredBlinnPhongShader.SetUniform1f(thisLightQuadratic.c_str(), quadraticFactor);
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// the following forward shading approach
		// we first copy the depth information of the previous framebuffer into the default framebuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// render lighting cubes
		DyedCubeShader.Bind();
		glBindVertexArray(cubeVAO);
		for (int i = 0.; i < NUM_LIGHTS; i++) {
			glm::mat4 lightModel;
			lightModel = glm::translate(lightModel, lightSources.positions[i]);
			lightModel = glm::scale(lightModel, glm::vec3(0.125f));
			DyedCubeShader.SetUniformMatrix4fv("mvp", camera.GetProjectionMatrix() * camera.GetViewMatrix() * lightModel);
			DyedCubeShader.SetUniformVector3fv("color", lightSources.colors[i]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		/*
		// render the shadow map
		// --------------------------------------------------------------------------------------------------------------
		ShadowMapRenderingShader.Bind();
		glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightProjection = ComputeOptimalShadowMapProjectionMatrix(theModel.GetAABB(), lightView * model);
		glm::mat4 lightViewProjection = lightProjection * lightView;
		ShadowMapRenderingShader.SetUniformMatrix4fv("lightViewProjection", lightViewProjection);
		ShadowMapRenderingShader.SetUniformMatrix4fv("model", model);
		glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapData.shadowMapFBO);
		// TODO: is this line necessary?
		glClear(GL_DEPTH_BUFFER_BIT);
		theModel.Draw(ShadowMapRenderingShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// reset viewport, get ready for the second pass
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		*/
		// OPTIONAL: draw the shadow map texture on the screen for visualization
		// --------------------------------------------------------------------------------------------------------------
		/*
		ShadowMapVisualizationShader.Bind();
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowMapData.shadowMapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		*/
		
		// -----------------------------------------------------------------------------------------------------------
		// ImGui stuff
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Control Panel");                          
			ImGui::Text("General:");
			// control the scaling
			ImGui::SliderFloat("Scaling", &scalingFactor, 0.0f, 6.0f);
			// control the mesh position
			ImGui::SliderFloat3("Translating", &translatingFactor[0], -10.0f, 10.0f);
			// gamma correction
			ImGui::Checkbox("Enable gamma correction", &enableGammaCorrection);
			if (enableGammaCorrection) {
				ImGui::InputFloat("Gamma", &gamma, 0.1f, 1.0f);
			}
			// normal visualization
			ImGui::Checkbox("Show normals", &enableNormalVisualization);
			if (enableNormalVisualization) {
				ImGui::InputFloat("Normal magnitude", &normalMagnitude, 0.01f, 1.0f);
				ImGui::ColorEdit3("Normal color", (float*)&normalColor);
			}
			// ground plane
			ImGui::Checkbox("Display ground plane", &enableGroundPlane);
			if (enableGroundPlane) {
				ImGui::InputFloat("Plane scale", &planeScalingFactor, 0.01f, 1.0f);
				ImGui::InputFloat("Plane height", &planeY, 0.1f, 1.0f);
			}
			// shadow map
			ImGui::Checkbox("Enable shadow map", &enableShadowMap);
			if (enableShadowMap) {
				ImGui::InputFloat("Shadow bias", &shadowBias, 0.01f, 1.0f);
			}
			ImGui::Separator();

			// select the shader
			ImGui::Text("Shading model:");
			const char* items[] = { "Gooch shading", "Phong shading", "Blinn-Phong shading"};
			ImGui::Combo("", &shaderIndex, items, IM_ARRAYSIZE(items));
			if (shaderIndex == 0) {           
				ImGui::ColorEdit3("Mesh color", (float*)&meshColor);
			}
			else if (shaderIndex == 1 || shaderIndex == 2) {
				ImGui::InputFloat("Brightness", &brightness, 0.1f, 1.0f);
				ImGui::InputFloat("Shininess", &shininess, 1.0f, 1.0f);
				ImGui::Checkbox("Enable distance attenuation", &enableAttenuation);
				if (enableAttenuation) {
					ImGui::InputFloat("Constant factor", &constantFactor);
					ImGui::InputFloat("Linear factor", &linearFactor);
					ImGui::InputFloat("Quadratic factor", &quadraticFactor);
				}
			} 
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// DELETE ME
		//ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::Render();
		// draw the UI on the screen
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

// callback functions
void FramebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
	glViewport(0, 0, width, height);
}

// the mouse input
void SphereCameraMouseCallback(GLFWwindow* window, double xpos, double ypos) {
	// calibrate the cursor position in the first frame
	if (camera.IsFirstFrame()) {
		camera.SetLastMouseX(xpos);
		camera.SetLastMouseY(ypos);
		camera.GameStart();
	}

	float xOffset = xpos - camera.GetLastMouseX();
	float yOffset = ypos - camera.GetLastMouseY();
	camera.SetLastMouseX(xpos);
	camera.SetLastMouseY(ypos);

	// Update the camera position :)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		camera.UpdatePhi(-camera.GetMouseSensitivity() * yOffset);
		camera.UpdateTheta(-camera.GetMouseSensitivity() * xOffset);
		camera.UpdatePosition();
	}
}

void SphereCameraScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	// changed code in imgui_impl_glfw.cpp
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	camera.UpdateRadius(-camera.GetMouseSensitivity() * yoffset);
	camera.UpdatePosition();
}