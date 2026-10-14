#version 460

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	vec4 colour;
} push;

void main() {
	outColor = push.colour;
}