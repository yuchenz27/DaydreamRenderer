#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
	vec3 position;
	vec3 color;

	float Constant;
	float Linear;
	float Quadratic;
};
float shininess;
const int NUM_LIGHTS = 32;
uniform Light lights[NUM_LIGHTS];
uniform vec3 viewPos;

void main() {
	// retrieve data from G-buffer
	vec3 position = texture(gPosition, TexCoord).rgb;
	vec3 normal = texture(gNormal, TexCoord).rgb;
	vec3 albedo = texture(gAlbedoSpec, TexCoord).rgb;
	float specular = texture(gAlbedoSpec, TexCoord).a;

	// iterate through all light sources to compute the total light
	vec3 lighting = albedo * 0.1; // hard-coded ambient component
	vec3 e = normalize(viewPos - position); // the view direction
	for (int i = 0; i < NUM_LIGHTS; i++) {
		// difuse component
		vec3 l = normalize(lights[i].position - position);
		vec3 diffuseComponent = max(dot(normal, l), 0.0) * albedo * lights[i].color;
		// specular component
		vec3 h = normalize(e + l);
		vec3 specularComponent = pow(max(dot(normal, h), 0.0), shininess) * specular * lights[i].color;
		// distance attenuation
		float distance = length(lights[i].position - position);
		float attenuation = 1.0 / (lights[i].Constant + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
		//diffuseComponent *= attenuation;
		//specularComponent *= attenuation;
		lighting += diffuseComponent + specularComponent;
	}

	FragColor = vec4(lighting, 1.0);
}