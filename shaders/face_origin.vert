#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;


layout(location = 0) out vec3 fragColor;


layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
    vec4 playerPos;
} push;

void main()
{
    // Extract model's world position from the model matrix
    vec3 modelPosition = vec3(push.modelMatrix[3][0], push.modelMatrix[3][1], push.modelMatrix[3][2]);

    // Calculate direction vector from model to origin
    vec3 directionToOrigin = normalize(modelPosition-push.playerPos.xyz);

    // Compute the angle to rotate around the Y-axis
    float angle = atan(directionToOrigin.x, directionToOrigin.z);

    // Create a Y-axis rotation matrix
    mat4 rotationY = mat4(
        vec4(cos(angle), 0.0, sin(angle), 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(-sin(angle), 0.0, cos(angle), 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    mat4 rotatedModel = push.modelMatrix * rotationY;

	vec4 positionWorld = rotatedModel* vec4(position, 1.0);
	vec4 viewPosition = ubo.view * positionWorld;
	gl_Position = ubo.projection * viewPosition;

    float depth = viewPosition.z;
	
    // Map depth to a darkness factor (adjust scale as needed)
    float vDarknessFactor = clamp(1.5 - (depth/16), 0.0, 1.5);
	fragColor = color *vDarknessFactor;

	
}
