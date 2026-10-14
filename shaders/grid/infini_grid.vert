#version 460

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;


layout(set = 0, binding = 0) uniform GlobalUbo 
{
	mat4 projection;
	mat4 view;
} ubo;
// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
	vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) 
{
	mat4 viewInv = inverse(view);
	mat4 projInv = inverse(projection);
	vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
	return unprojectedPoint.xyz / unprojectedPoint.w;
}

// normal vertice projection
void main() 
{
	vec3 p = gridPlane[gl_VertexIndex].xyz;
	nearPoint = UnprojectPoint(p.x, p.y, 0.0, ubo.view, ubo.projection).xyz; // unprojecting on the near plane
	farPoint = UnprojectPoint(p.x, p.y, 1.0, ubo.view, ubo.projection).xyz; // unprojecting on the far plane
	gl_Position = vec4(p, 1.0); // using directly the clipped coordinates


	//gl_Position = ubo.projection * ubo.view * vec4(gridPlane[gl_VertexIndex].xyz, 1.0);
}

