#version 330 core

in vec3 positionInVS;
in vec3 normalInVS;

out vec4 FragColor;

uniform vec3 lightPositionInVS;
uniform vec3 color;

void main() {
	// all operations are in view space
	vec3 n = normalize(normalInVS);
	vec3 l = normalize(lightPositionInVS - positionInVS);
	vec3 v = normalize(-positionInVS);

	// Gooch shading model
	vec3 c_cool = vec3(0.0, 0.0, 0.55) + 0.25 * color;
	vec3 c_warm = vec3(0.3, 0.3, 0.0) + 0.25 * color;
	vec3 c_highlight = vec3(1.0, 1.0, 1.0);

	float t = (dot(n, l) + 1.0) / 2.0;
	vec3 r = reflect(-l, n);
	float s = clamp(100.0 * dot(r, v) - 97.0, 0.0, 1.0);

	vec3 c_shaded = s * c_highlight + (1.0 - s) * (t * c_warm + (1.0 - t) * c_cool);
	FragColor = vec4(c_shaded, 1.0);
}