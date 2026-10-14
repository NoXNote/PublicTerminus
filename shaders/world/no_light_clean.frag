#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;


void main() {
	// float contrast = 1.4; // Increase contrast
	// vec3 mean = vec3(0.8);
	// outColor = vec4((fragColor.xyz - mean) * contrast + mean, 1);
  	outColor = vec4(fragColor.xyz, 1);
}