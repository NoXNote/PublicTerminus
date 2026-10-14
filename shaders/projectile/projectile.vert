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
	vec4 colour;
	float flashLife;
} push;

float rampCurve(float x, float a, float b, float n) {
    if (x < a) 
	{
        return x / a;  // Sharp ramp up
    } else if (x < b) 
	{
        float t = (x - a) / (b - a);
        return 1.0 - pow(t, n);  // Slow descent
    } else 
	{
        return 0.0;
    }
}

void main()
{
	// Makes shards fly out along line
	vec4 positionWorld = push.modelMatrix *  vec4(rampCurve(push.flashLife, 0.01, 0.1, 0.5) *(position-vec3(0,0,64*push.flashLife)), 1.0);

	vec4 viewPosition = ubo.view * positionWorld;
	gl_Position = ubo.projection * viewPosition;

	fragColor = color;
}
