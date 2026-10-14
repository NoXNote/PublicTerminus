#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	vec4 darken;
	vec4 widthHeightTime;
} push;

const float NOISE_GRANULARITY = 0.03; //0/.015
const float CONTRAST = 1.3;
const float BRIGHTNESS = 0.00;

vec2 resolution = vec2(push.widthHeightTime.x,push.widthHeightTime.y);

float random(highp vec2 coords) {
	return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}



void main() {
	vec3 ditheredColor = fragColor;
	vec2 boil = vec2(sin(push.widthHeightTime.z), cos(push.widthHeightTime.z))/1638000;
	float level = 8.0;
	highp vec2 coordinates = (gl_FragCoord.xy) / resolution;

	ditheredColor += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, random(boil + coordinates));

    vec2 centeredUV = (coordinates - 0.5) * 2.0;  
    float dist = length(centeredUV); // Get radial distance from center

    float vignette = smoothstep(2, 0.6, dist);

    ditheredColor += BRIGHTNESS;
    ditheredColor = (ditheredColor - 0.5) * CONTRAST + 0.5;

	ditheredColor.r = round(ditheredColor.r *level)/level;
	ditheredColor.g = round(ditheredColor.g *level)/level;
	ditheredColor.b = round(ditheredColor.b *level)/level;

	float vignette2 = smoothstep(2, 1, dist);

	outColor = vec4(vec3(ditheredColor), 1.0);
}