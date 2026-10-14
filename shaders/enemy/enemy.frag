#version 460
#extension GL_EXT_fragment_shader_barycentric  : require
layout(constant_id = 0) const int partID = 2;  // Default to Torso

layout(push_constant) uniform PushConstants {
	mat4 modelMatrix;
    float time;
    float speed;
    float amplitude;
    float turningToRight;
} push;

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
} ubo;


const int PART_THIGH = 0;
const int PART_SHIN = 1;
const int PART_FOOT = 2;
const int PART_TORSO = 3;
const int PART_UPPER_ARM = 4;
const int PART_FOREARM = 5;

void main() 
{
	vec3 ditheredColor = fragColor;
   	float level = 8.0;
    ditheredColor = (ditheredColor - 0.5) * 1.3 + 0.5;

	ditheredColor.r = round(ditheredColor.r *level)/level;
	ditheredColor.g = round(ditheredColor.g *level)/level;
	ditheredColor.b = round(ditheredColor.b *level)/level;



	outColor = vec4(vec3(ditheredColor), 1.0);
}