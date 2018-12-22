#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Model.h"

struct ShadowMap {
	unsigned int FBO;
	unsigned int texture;
};

ShadowMap CreateShadowMap(const unsigned int width, const unsigned int height) {
	unsigned int shadowMapFBO;
	glGenFramebuffers(1, &shadowMapFBO);

	unsigned int shadowMapTexture;
	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ShadowMap result;
	result.FBO = shadowMapFBO;
	result.texture = shadowMapTexture;

	return result;
}

glm::mat4 ComputeOptimalShadowMapProjectionMatrix(const AABB modelAABB, const glm::mat4 lightView) {
	AABB lightSpaceAABB = Model::TransformAABB(modelAABB, lightView);
	return glm::ortho(lightSpaceAABB.xmin, lightSpaceAABB.xmax, lightSpaceAABB.ymin, lightSpaceAABB.ymax, -lightSpaceAABB.zmax, -lightSpaceAABB.zmin);
}