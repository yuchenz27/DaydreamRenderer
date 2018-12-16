#version 330 core
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

// all these vectors are in world space not view space :)
out vec3 position;
out vec3 normal;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 positionInWS4 = model * vec4(in_Position, 1.0);
	position = positionInWS4.xyz;

	normal = mat3(transpose(inverse(model))) * in_Normal;

	texCoord = in_TexCoord;

	gl_Position = projection * view * positionInWS4;
}