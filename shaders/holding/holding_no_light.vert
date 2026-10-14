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
} push;

void main()
{
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * positionWorld;

    fragColor = color;
}
