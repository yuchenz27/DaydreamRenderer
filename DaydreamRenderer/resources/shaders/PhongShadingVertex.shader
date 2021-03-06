#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

out vec3 fNormalInVS;
out vec3 fPositionInVS;
out vec2 fTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 fPositionInVS4 = view * model * vec4(in_Position, 1.0);
	gl_Position = projection * fPositionInVS4;
	fPositionInVS = fPositionInVS4.xyz;
	fTexCoord = in_TexCoord;
	fNormalInVS = mat3(transpose(inverse(view * model))) * in_Normal;
}