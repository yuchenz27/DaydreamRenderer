#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gDiffuse;
layout(location = 3) out vec3 gSpecular;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;

	sampler2D texture_specular1;
	sampler2D texture_specular2;
	sampler2D texture_specular3;
};

uniform Material material;

void main() {
	gPosition = position;
	gNormal = normalize(normal);
	gDiffuse = texture(material.texture_diffuse1, texCoord).rgb;
	gSpecular = texture(material.texture_specular1, texCoord).rgb;
}