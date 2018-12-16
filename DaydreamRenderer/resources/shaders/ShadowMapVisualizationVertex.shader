#version 330 core
layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out vec2 fTexCoord;

void main() {
	gl_Position = vec4(in_Position.x, in_Position.y, 0.0, 1.0);
	fTexCoord = in_TexCoord;
}