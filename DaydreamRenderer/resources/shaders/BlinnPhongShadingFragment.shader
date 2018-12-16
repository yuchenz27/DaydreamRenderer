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
in vec4 fPositionInLC;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform bool enableAttenuation;
uniform bool enableGammaCorrection;
uniform float gamma;
uniform bool enableShadowMap;
uniform sampler2D shadowMap;
uniform float shadowBias;

// output 1 when the fragment is lit, 0 otherwise
float ShadowDetermination(vec4 fPositionInLC) {
	// perspective division
	vec3 fragCoord = fPositionInLC.xyz / fPositionInLC.w;
	// transform the range from [-1, 1] to [0, 1]
	fragCoord = fragCoord * 0.5 + 0.5;
	float isLit = fragCoord.z > (texture(shadowMap, fragCoord.xy).r + shadowBias) ? 0.0 : 1.0f;

	return isLit;
}

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
	// the halfway vector
	vec3 h = normalize(e + l);
	vec3 specular = pow(max(dot(n, h), 0.0), material.shininess) * light.specular * vec3(texture(material.texture_specular1, fTexCoord));

	// compute the distance attenuation
	if (enableAttenuation) {
		float attenuation = 1.0 / (light.constantFactor + light.linearFactor * distance + light.quadraticFactor * (distance * distance));
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}

	// shadow map operation
	float isLit = 1.0f;
	if (enableShadowMap)
		isLit = ShadowDetermination(fPositionInLC);

	// the resulting color
	FragColor = vec4(ambient + ((diffuse + specular) * isLit), 1.0);
	
	// gamma correction
	if (enableGammaCorrection)
		FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
}