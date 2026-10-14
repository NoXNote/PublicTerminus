#version 460

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[32];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 gunMatrix;
	vec4 positionOnBarrel;
	float flashLife;
} push;

const float M_PI = 3.1415926538;

void main() {
	float dis = sqrt(dot(fragOffset, fragOffset));
	if (dis >= 1.0) {
		discard;
	}
	// if(push.flashLife < 0.5)
	// {
	// 	//push.flashLife*(cos(dis*M_PI) + 1)
	// 	outColor = vec4(
	// 		push.flashLife*(cos(dis*M_PI) + 1),
	// 		1.0,
	// 		1.0,
	// 		push.flashLife*(cos(dis*M_PI) + 1) );
	// } else
	// {
	// 	outColor = vec4(
	// 		push.flashLife*(cos(dis*M_PI) + 1),
	// 		1.0,
	// 		1.0,
	// 		push.flashLife*(cos(dis*M_PI) + 1));
	// }

	outColor = vec4(
		//pow(push.flashLife*(cos(dis*M_PI) + 1),1)+0.5, // for no alpha
		pow(push.flashLife*(cos(dis*M_PI) + 1),1),
		1.0,
		1.0,
		pow(push.flashLife,1)*(cos(dis*M_PI) + 1));

}