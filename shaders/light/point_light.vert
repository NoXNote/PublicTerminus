#version 460

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

// struct PointLight {
// 	vec4 position; // ignore w
// 	vec4 color; // w is intensity
// };

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor; // w is intensity
} ubo;

layout(push_constant) uniform Push {
	mat4 gunMatrix;
	vec4 positionOnBarrel;
	float flashLife;
} push;

void main()
{
	fragOffset = OFFSETS[gl_VertexIndex];




	vec4 lightInCameraSpace = push.gunMatrix * vec4(push.positionOnBarrel.xyz, 1.0);
	//vec4 positionInCameraSpace = lightInCameraSpace + push.radius * vec4(fragOffset, 0.0, 0.0);
    vec4 positionInCameraSpace = lightInCameraSpace + (push.flashLife/3.0) * vec4(fragOffset, 0.0, 0.0);
	gl_Position = ubo.projection * positionInCameraSpace;

	// vec4 positionWorld = push.gunMatrix * vec4(push.positionOnBarrel + (y*rotateZ(position, y*4.0)*(pow(push.flashLife,2))), 1.0);
	// gl_Position = ubo.projection * positionWorld;
}