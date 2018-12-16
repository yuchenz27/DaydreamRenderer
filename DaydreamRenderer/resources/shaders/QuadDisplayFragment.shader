#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D thisTexture;

void main() {
	FragColor = vec4(texture(thisTexture, TexCoord).xyz, 1.0);
}