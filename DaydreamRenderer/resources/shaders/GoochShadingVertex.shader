#version 330 core
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

out vec3 positionInVS;
out vec3 normalInVS;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 fragPositionInVS4 = view * model * vec4(in_Position, 1.0);
	gl_Position = projection * fragPositionInVS4;

	positionInVS = fragPositionInVS4.xyz;
	normalInVS = mat3(transpose(inverse(view * model))) * in_Normal;
}