#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;


layout(location = 0) out flat vec3 fragColor;


layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	vec4 darken;
	vec4 widthHeightTime;
} push;

void main()
{
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	vec4 viewPosition = ubo.view * positionWorld;
	gl_Position = ubo.projection * viewPosition;

    float depth = viewPosition.z;
	
    // Map depth to a darkness factor
    float vDarknessFactor = clamp(1.0 - (depth/(16*push.darken.w)), 0.0, 1.0);

	// fragColor =	vec3(mix(color.r * vDarknessFactor, color.r, step(0.9, color.r)),
    //     mix(color.g * vDarknessFactor, color.g, step(0.9, color.g)),
    //     mix(color.b * vDarknessFactor, color.b, step(0.9, color.b)));
	fragColor = color;
}
