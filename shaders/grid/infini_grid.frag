#version 460
layout (location = 1) in vec3 nearPoint;
layout (location = 2) in vec3 farPoint;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	vec4 color;
    vec4 position;
} push;

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    //vec4 color = vec4(push.color.xyz, (1.00 - min(line*4, 1.0))/ length(fragPos3D.xz-push.position.xz)); 
    //vec4 color = vec4(push.color.xyz, (1.00 - min(line*4, 1.0)));
	vec3 lineCol = {1.0,1.0,1.0};
    vec4 color = vec4(push.color.xyz+(1-(length(fragPos3D.xz-push.position.xz)/8)), (1.00 - min((line*4), 1.0))); 
    //color += vec4(1,1,1, (1.00 - min(line*4, 1.0)));
    // // z axis
    // if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
    //     color.z = 1.0;
    // // x axis
    // if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
    //     color.x = 1.0;
    return color;
}
void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    if (t < 0) 
	{
        discard;
	}
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    //fragPos3D -= push.position.xyz; // scuffed
    outColor = grid(fragPos3D, 1);
}