#version 330 core
in vec2 fTexCoord;

out vec4 FragColor;

uniform sampler2D shadowMap;

void main() {
	float depthValue = texture(shadowMap, fTexCoord).r;
	FragColor = vec4(vec3(depthValue), 1.0);
}