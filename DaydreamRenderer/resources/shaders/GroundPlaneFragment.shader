#version 330 core
in vec3 fragPosition;
in vec4 fragPositionInLC;

out vec4 FragColor;

uniform vec3 backgroundColor;
uniform vec3 groundPlaneColor;
uniform float innerRadius;
uniform float outerRadius;

uniform bool useShadowMap;
uniform sampler2D shadowMap;
uniform float shadowBias;
uniform int pcfRadius;

// output 1 when the fragment is lit, 0 in shadow
float IsLit(vec4 fragPositionInLC) {
	// perspective division
	vec3 fragCoord = fragPositionInLC.xyz / fragPositionInLC.w;
	// check if the fragment is in the shadow map
	if (fragCoord.x > 1.0 || fragCoord.x < -1.0 || fragCoord.y > 1.0 || fragCoord.y < -1.0) {
		return 1;
	}
	// transform the range from [-1, 1] to [0, 1]
	fragCoord = fragCoord * 0.5 + 0.5;
	float isLit;
	float xUnit = 1.0 / (textureSize(shadowMap, 0).x + 1);
	float yUnit = 1.0 / (textureSize(shadowMap, 0).y + 1);
	if (pcfRadius == 0) {
		isLit = fragCoord.z >(texture(shadowMap, fragCoord.xy).r + shadowBias) ? 0.0f : 1.0f;
	} else {
		for (int i = -pcfRadius; i < pcfRadius + 1; i++) {
			for (int j = -pcfRadius; j < pcfRadius + 1; j++) {
				isLit += fragCoord.z >(texture(shadowMap, vec2(fragCoord.x + i * xUnit, fragCoord.y + j * yUnit)).r + shadowBias) ? 0.0f : 1.0f;
			}
		}
		isLit /= pow((1 + 2 * pcfRadius), 2);
	}
	return isLit;
}

void main() {
	float distance = length(fragPosition);
	if (distance < innerRadius) {
		FragColor = vec4(groundPlaneColor, 1.0);
	} else if (distance > outerRadius) {
		FragColor = vec4(backgroundColor, 1.0);
	} else {
		float coefficient = pow((distance - innerRadius) / (outerRadius - innerRadius), 1);
		FragColor = vec4(groundPlaneColor - (groundPlaneColor - backgroundColor) * coefficient, 1.0);
	}

	if (useShadowMap) {
		float isLit = IsLit(fragPositionInLC);
		FragColor.xyz = FragColor.xyz * isLit;
	}
}