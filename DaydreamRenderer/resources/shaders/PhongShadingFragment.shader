#version 330 core

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;

	sampler2D texture_specular1;
	sampler2D texture_specular2;
	sampler2D texture_specular3;

	float shininess;
};

struct Light {
	vec3 positionInVS;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constantFactor;
	float linearFactor;
	float quadraticFactor;
};

in vec3 fPositionInVS;
in vec3 fNormalInVS;
in vec2 fTexCoord;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform bool enableAttenuation;

void main() {
	// ambient component
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fTexCoord));

	// diffuse component
	vec3 n = normalize(fNormalInVS);
	vec3 fragToLight = light.positionInVS - fPositionInVS;
	float distance = length(fragToLight);
	vec3 l = normalize(fragToLight);
	vec3 diffuse = max(dot(n, l), 0.0) * light.diffuse * vec3(texture(material.texture_diffuse1, fTexCoord));

	//specular component
	vec3 e = normalize(-fPositionInVS);
	vec3 reflectionDirection = reflect(-l, n);
	vec3 specular = pow(max(dot(e, reflectionDirection), 0.0), material.shininess) * light.specular * vec3(texture(material.texture_specular1, fTexCoord));

	// compute the distance attenuation
	if (enableAttenuation) {
		float attenuation = 1.0 / (light.constantFactor + light.linearFactor * distance + light.quadraticFactor * (distance * distance));
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}

	FragColor = vec4(ambient + diffuse + specular, 1.0);
}