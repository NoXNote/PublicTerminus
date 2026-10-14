#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  	mat4 projection;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
  	mat4 gunMatrix;
  	vec4 positionOnBarrel;
	float flashLife;
    //bool inWorld;
} push;

vec3 rotateZ(vec3 v, float theta) {
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    mat3 rotationMatrix = mat3(
        cosTheta, -sinTheta, 0.0,
        sinTheta,  cosTheta, 0.0,
        0.0,       0.0,      1.0
    );
    return rotationMatrix * v;
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	
	float y = 1.0+fract(sin(push.flashLife)*10000.);
	
	vec4 positionWorld = push.gunMatrix * vec4(push.positionOnBarrel.xyz + (y*rotateZ(position, y*4.0)*(pow(push.flashLife,2))), 1.0);

    gl_Position = ubo.projection * ubo.view * positionWorld;

    fragColor = color;
   
  	
}