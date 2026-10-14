#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
} push;

const float NOISE_GRANULARITY = 0.02; //0/.015
vec2 resolution = vec2(1280,720);

float random(highp vec2 coords) {
	return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
	vec3 ditheredColor = fragColor;
	float level = 2.0;
	highp vec2 coordinates = gl_FragCoord.xy / resolution;

	ditheredColor += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, random(coordinates));
	ditheredColor.r = round(ditheredColor.r *level)/level;
	ditheredColor.g = round(ditheredColor.g *level)/level;
	ditheredColor.b = round(ditheredColor.b *level)/level;

	outColor = vec4(vec3(ditheredColor), 1.0);

	//outColor = vec4(fragColor, 1.0);
}