#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;

struct PointLight {
	vec3 position;
	vec3 color;
};
const int MAX_POINTLIGHTS = 32;
uniform int NUM_POINTLIGHTS;
uniform PointLight pointLights[MAX_POINTLIGHTS];

struct DirectionalLight {
	vec3 direction;
	vec3 color;
};
const int MAX_DIRECTIONALLIGHTS = 1;
uniform int NUM_DIRECTIONALLIGHTS;
uniform DirectionalLight directionalLights[MAX_DIRECTIONALLIGHTS];

uniform float shininess;
// the attenuation factors
uniform float constantFactor;
uniform float linearFactor;
uniform float quadraticFactor;
uniform vec3 viewPos;
uniform vec3 backgroundColor;

void main() {
	// retrieve data from G-buffer
	vec3 position = texture(gPosition, TexCoord).rgb;
	vec3 normal = texture(gNormal, TexCoord).rgb;
	vec3 diffuse = texture(gDiffuse, TexCoord).rgb;
	vec3 specular = texture(gSpecular, TexCoord).rgb;

	if (position == vec3(1.0, 1.0, 1.0) && normal == vec3(1.0, 1.0, 1.0)) {
		FragColor = vec4(backgroundColor, 1.0);
	} else {
		vec3 lighting = diffuse * 0.1; // hard-coded ambient component
		vec3 e = normalize(viewPos - position); // the view direction
		// compute directional lights
		for (int i = 0; i < NUM_DIRECTIONALLIGHTS; i++) {
			// diffuse component
			vec3 l = -normalize(directionalLights[i].direction);
			vec3 directionalDiffuse = max(dot(normal, l), 0.0) * diffuse * directionalLights[i].color;
			// specular component
			vec3 h = normalize(e + l);
			vec3 directionalSpecular = pow(max(dot(normal, h), 0.0), shininess) * specular * directionalLights[i].color;
			// I presume directional lights don't have attenuation
			lighting += directionalDiffuse + directionalSpecular;
		}
		// compute point lights
		for (int i = 0; i < NUM_POINTLIGHTS; i++) {
			// difuse component
			vec3 l = normalize(pointLights[i].position - position);
			vec3 diffuseComponent = max(dot(normal, l), 0.0) * diffuse * pointLights[i].color;
			// specular component
			vec3 h = normalize(e + l);
			vec3 specularComponent = pow(max(dot(normal, h), 0.0), shininess) * specular * pointLights[i].color;
			// distance attenuation
			float distance = length(pointLights[i].position - position);
			float attenuation = 1.0 / (constantFactor + linearFactor * distance + quadraticFactor * distance * distance);
			diffuseComponent *= attenuation;
			specularComponent *= attenuation;
			lighting += diffuseComponent + specularComponent;
		}
		FragColor = vec4(lighting, 1.0);
	}
}