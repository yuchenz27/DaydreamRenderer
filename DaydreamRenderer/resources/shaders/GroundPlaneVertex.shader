#version 330 core
layout(location = 0) in vec3 in_Position;

// fragment position in world space
out vec3 fragPosition;
// fragment position in light clip space
out vec4 fragPositionInLC;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightViewProjection;

void main() {
	vec4 fragPosition4 = model * vec4(in_Position, 1.0);
	fragPosition = fragPosition4.xyz;
	gl_Position = projection * view * fragPosition4;
	fragPositionInLC = lightViewProjection * fragPosition4;
}