#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_glfw.h"
#include "ImGui\imgui_impl_opengl3.h"
#include "stb_image.h"
#include "Shader.h"
#include "FPCamera.h"
#include "SphereCamera.h"
#include "ShadowMap.h"
#include "PrefabsCreation.h"
#include "Model.h"
#include "DeferredShading.h"
#include "TextureLoading.h"

void FramebufferSizeCallback(GLFWwindow* window, const int width, const int height);
void SphereCameraMouseCallback(GLFWwindow* window, double xpos, double ypos);
void SphereCameraScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

const int SCR_WIDTH = 3000;
const int SCR_HEIGHT = 1800;
SphereCamera sphereCamera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f, 5.0f);

int main() {
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
	if (glewInit() != GLEW_OK) {
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

	// ImGui setup
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();

	// ----------------------------------------------------------------------------------------------------------------
	//Model theModel("resources/models/nanosuit/nanosuit.obj");
	Model theModel("resources/models/Myriam/10016_w_Myriam.fbx");
	//Model theModel("resources/models/cube.obj");
	//theModel.LogTextureInfo();
	//std::cout << "Number of vertices in the model: " << theModel.GetNumVertices() << std::endl;
	//Model::LogAABB(theModel.GetAABB());

	unsigned int quadVAO = CreateQuadVAO();
	unsigned int cubeVAO = CreateCubeVAO();
	unsigned int planeVAO = CreatePlaneVAO();

	// ----------------------------------------------------------------------------------------------------------------
	Shader ShadowMapRenderingShader("resources/shaders/ShadowMapRendering");
	Shader ShadowMapVisualizationShader("resources/shaders/ShadowMapVisualization");
	Shader GeometryPassShader("resources/shaders/GeometryPass");
	Shader QuadDisplayShader("resources/shaders/QuadDisplay");
	Shader DeferredBlinnPhongShader("resources/shaders/DeferredBlinnPhongShading");
	Shader SingleColorShader("resources/shaders/SingleColor");
	Shader GroundPlaneShader("resources/shaders/GroundPlane");

	// ----------------------------------------------------------------------------------------------------------------
	// set up shadow map
	const unsigned int SHADOWMAP_WIDTH = 3000;
	const unsigned int SHADOWMAP_HEIGHT = 3000;
	ShadowMap shadowMap = CreateShadowMap(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	// the shadow map projection matrix
	// left, right, bottom, top, near and far
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 12.5f);

	// ---------------------------------------------------------------------------------------------------------
	// set up shading parameters
	float scalingFactor = .01f;
	glm::vec3 centeringVector = -theModel.GetModelCenter();
	// parameters for Gooch shading
	glm::vec3 meshColor(0.8f, 0.8f, 0.9f);
	// parameters for Phong shading
	float brightness = 1.0f;
	float constantFactor = 1.0f;
	float linearFactor = 0.09f;
	float quadraticFactor = 0.032f;
	float shininess = 16.0f;
	
	// new parameters
	bool visualizeGBuffers = false;
	int gBufferIndex = 0;
	glm::vec3 backgroundColor(0.86f, 0.86f, 0.86f);
	glm::vec3 groundPlaneColor(0.95f, 0.95f, 0.95f);
	float groundPlaneInnerRadius = 1.5f;
	float groundPlaneOuterRadius = 4.0f;
	// set up all light sources
	const int NUM_POINTLIGHTS = 0;
	PointLights pointLights = CreateRandomPointLights(NUM_POINTLIGHTS);
	const int NUM_DIRECTIONALLIGHTS = 1;
	glm::vec3 directionalLightPosition(10.0f, 10.0f, 10.0f);
	DirectionalLights directionalLights = CreateSingleDirectionalLight(glm::vec3(0.0f, 0.0f, 0.0f) - directionalLightPosition, glm::vec3(1.0f, 1.0f, 1.0f));
	bool useShadowMap = true;
	bool visualizeShadowMap = false;
	float shadowBias = 0.002f;
	int pcfRadius = 3;

	// ---------------------------------------------------------------------------------------------------------
	GBuffer gBuffer = InitGBuffer(SCR_WIDTH, SCR_HEIGHT);

	// the rendering loop
	while (!glfwWindowShouldClose(window)) {

		sphereCamera.UpdateDeltaTime();
		sphereCamera.ProcessKeyboardInput(window);
		// Start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// THE GEOMETRY PASS--------------------------------------------------------------------------------------------------
		// set up the model matrix first
		// NOTICE: if we scale first and then translate, the following translation will also be scaled!
		glm::mat4 model;
		model = glm::scale(model, glm::vec3(scalingFactor));
		model = glm::translate(model, centeringVector);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.framebuffer);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GeometryPassShader.Bind();
		GeometryPassShader.SetUniformMatrix4fv("model", model);
		GeometryPassShader.SetUniformMatrix4fv("view", sphereCamera.GetViewMatrix());
		GeometryPassShader.SetUniformMatrix4fv("projection", sphereCamera.GetProjectionMatrix());
		// draw the model
		theModel.Draw(GeometryPassShader);
		
		// OPTIONAL: visualize the G-buffer-----------------------------------------------------------------------------------
		if (visualizeGBuffers) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			QuadDisplayShader.Bind();
			glBindVertexArray(quadVAO);
			glActiveTexture(GL_TEXTURE0);
			if (gBufferIndex == 0) {
				glBindTexture(GL_TEXTURE_2D, gBuffer.positionTexture);
			} else if (gBufferIndex == 1) {
				glBindTexture(GL_TEXTURE_2D, gBuffer.normalTexture);
			} else if (gBufferIndex == 2) {
				glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseTexture);
			} else if (gBufferIndex == 3) {
				glBindTexture(GL_TEXTURE_2D, gBuffer.specularTexture);
			}
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// I need the ground plane model matrix so as to draw it in the shadow map
		glm::mat4 groundPlaneModel;
		groundPlaneModel = glm::translate(groundPlaneModel, glm::vec3(0, Model::TransformAABB(theModel.GetAABB(), model).ymin, 0));
		groundPlaneModel = glm::scale(groundPlaneModel, glm::vec3(0.5f));

		// RENDERING THE SHADOW MAP------------------------------------------------------------------------------------------
		glm::mat4 lightViewProjection; // need to use this in other scope
		if (useShadowMap) {
			ShadowMapRenderingShader.Bind();
			glm::mat4 lightView = glm::lookAt(directionalLightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			//lightProjection = ComputeOptimalShadowMapProjectionMatrix(theModel.GetAABB(), lightView * model);
			lightProjection = ComputeOptimalShadowMapProjectionMatrix(CreatePlaneAABB(), lightView * groundPlaneModel);
			lightViewProjection = lightProjection * lightView;
			ShadowMapRenderingShader.SetUniformMatrix4fv("lightViewProjection", lightViewProjection);
			ShadowMapRenderingShader.SetUniformMatrix4fv("model", model);
			glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.FBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			theModel.Draw(ShadowMapRenderingShader);
			// also draw the ground plane
			ShadowMapRenderingShader.SetUniformMatrix4fv("model", groundPlaneModel);
			glBindVertexArray(planeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// reset the view port
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		}
		
		// OPTIONAL: visualize the shadow map
		if (visualizeShadowMap) {

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ShadowMapVisualizationShader.Bind();
			glBindVertexArray(quadVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMap.texture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		
		// THE LIGHTING PASS-----------------------------------------------------------------------------------------------
		if (!visualizeGBuffers && !visualizeShadowMap) {
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gBuffer.positionTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gBuffer.normalTexture);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseTexture);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, gBuffer.specularTexture);
			// shader configuration
			DeferredBlinnPhongShader.Bind();
			DeferredBlinnPhongShader.SetUniform1i("gPosition", 0);
			DeferredBlinnPhongShader.SetUniform1i("gNormal", 1);
			DeferredBlinnPhongShader.SetUniform1i("gDiffuse", 2);
			DeferredBlinnPhongShader.SetUniform1i("gSpecular", 3);
			DeferredBlinnPhongShader.SetUniformVector3fv("viewPos", sphereCamera.GetPosition());
			DeferredBlinnPhongShader.SetUniform1f("shininess", shininess);
			DeferredBlinnPhongShader.SetUniform1f("constantFactor", constantFactor);
			DeferredBlinnPhongShader.SetUniform1f("linearFactor", linearFactor);
			DeferredBlinnPhongShader.SetUniform1f("quadraticFactor", quadraticFactor);
			// directional lights
			DeferredBlinnPhongShader.SetUniform1i("NUM_DIRECTIONALLIGHTS", NUM_DIRECTIONALLIGHTS);
			for (int i = 0; i < NUM_DIRECTIONALLIGHTS; i++) {
				std::string thisLightPosition = "directionalLights[" + std::to_string(i) + "].direction";
				std::string thisLightColor = "directionalLights[" + std::to_string(i) + "].color";
				DeferredBlinnPhongShader.SetUniformVector3fv(thisLightPosition.c_str(), directionalLights.directions[i]);
				DeferredBlinnPhongShader.SetUniformVector3fv(thisLightColor.c_str(), directionalLights.colors[i]);
			}
			// point lights
			DeferredBlinnPhongShader.SetUniform1i("NUM_POINTLIGHTS", NUM_POINTLIGHTS);
			for (int i = 0; i < NUM_POINTLIGHTS; i++) {
				std::string thisLightPosition = "pointLights[" + std::to_string(i) + "].position";
				std::string thisLightColor = "pointLights[" + std::to_string(i) + "].color";
				DeferredBlinnPhongShader.SetUniformVector3fv(thisLightPosition.c_str(), pointLights.positions[i]);
				DeferredBlinnPhongShader.SetUniformVector3fv(thisLightColor.c_str(), pointLights.colors[i]);
			}
			DeferredBlinnPhongShader.SetUniformVector3fv("backgroundColor", backgroundColor);
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			//----------------------------------------------------------------------------------------------------------
			// the following forward shading approach
			// we first copy the depth information of the previous framebuffer into the default framebuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.framebuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// render lighting cubes
			SingleColorShader.Bind();
			glBindVertexArray(cubeVAO);
			for (int i = 0.; i < NUM_POINTLIGHTS; i++) {
				glm::mat4 lightModel;
				lightModel = glm::translate(lightModel, pointLights.positions[i]);
				lightModel = glm::scale(lightModel, glm::vec3(0.125f));
				SingleColorShader.SetUniformMatrix4fv("mvp", sphereCamera.GetProjectionMatrix() * sphereCamera.GetViewMatrix() * lightModel);
				SingleColorShader.SetUniformVector3fv("color", pointLights.colors[i]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			// rendering the ground plane
			GroundPlaneShader.Bind();
			GroundPlaneShader.SetUniformMatrix4fv("model", groundPlaneModel);
			GroundPlaneShader.SetUniformMatrix4fv("view", sphereCamera.GetViewMatrix());
			GroundPlaneShader.SetUniformMatrix4fv("projection", sphereCamera.GetProjectionMatrix());
			GroundPlaneShader.SetUniformVector3fv("backgroundColor", backgroundColor);
			GroundPlaneShader.SetUniformVector3fv("groundPlaneColor", groundPlaneColor);
			GroundPlaneShader.SetUniform1f("innerRadius", groundPlaneInnerRadius);
			GroundPlaneShader.SetUniform1f("outerRadius", groundPlaneOuterRadius);
			GroundPlaneShader.SetUniform1i("useShadowMap", useShadowMap);
			if (useShadowMap) {
				GroundPlaneShader.SetUniformMatrix4fv("lightViewProjection", lightViewProjection);
				GroundPlaneShader.SetUniform1i("shadowMap", 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, shadowMap.texture);
				GroundPlaneShader.SetUniform1f("shadowBias", shadowBias);
				GroundPlaneShader.SetUniform1i("pcfRadius", pcfRadius);
			}
			glBindVertexArray(planeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		
		// -----------------------------------------------------------------------------------------------------------
		// ImGui stuff
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Control Panel");                          
			ImGui::Text("General:");
			// control the scaling
			ImGui::SliderFloat("Scaling", &scalingFactor, 0.0f, 0.5f);
			ImGui::Checkbox("G-buffer", &visualizeGBuffers);
			const char* items[] = { "position", "normal", "diffuse", "specular" };
			ImGui::Combo("", &gBufferIndex, items, IM_ARRAYSIZE(items));
			ImGui::Checkbox("Visualize shadow map", &visualizeShadowMap);

			/*
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
			*/
			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
	if (sphereCamera.IsFirstFrame()) {
		sphereCamera.SetLastMouseX(xpos);
		sphereCamera.SetLastMouseY(ypos);
		sphereCamera.GameStart();
	}

	float xOffset = xpos - sphereCamera.GetLastMouseX();
	float yOffset = ypos - sphereCamera.GetLastMouseY();
	sphereCamera.SetLastMouseX(xpos);
	sphereCamera.SetLastMouseY(ypos);

	// Update the camera position :)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		sphereCamera.UpdatePhi(-sphereCamera.GetMouseSensitivity() * yOffset);
		sphereCamera.UpdateTheta(-sphereCamera.GetMouseSensitivity() * xOffset);
		sphereCamera.UpdatePosition();
	}
}

void SphereCameraScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	// changed code in imgui_impl_glfw.cpp
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	sphereCamera.UpdateRadius(-sphereCamera.GetMouseSensitivity() * yoffset);
	sphereCamera.UpdatePosition();
}