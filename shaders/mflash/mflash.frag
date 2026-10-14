#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

vec2 resolution = vec2(1280,720);

layout(push_constant) uniform Push {
	mat4 gunMatrix;
	vec4 positionOnBarrel;
	float flashLife;
    //bool inWorld;
} push;

const float NOISE_GRANULARITY = 0.015;

float random(highp vec2 coords) {
	return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}

#define iterations 2
void main()
{
	float iTime = push.flashLife;
	vec2 uv = gl_FragCoord.xy / resolution.xy;

	float res = 1.;
    for (int i = 0; i < iterations; i++) {
        res += cos(uv.y*12.345 - iTime*4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + cos(uv.x*12.345);
    }
        
	vec3 c = mix(vec3(1,0.8,.2),
                 vec3(1.,1.,1.),
                 cos(res+cos(uv.y*24.3214)*.1+cos(uv.x*6.324+iTime*4.)+iTime)*.5+.5);
    
	vec3 ditheredColor = c;
	float level = 1.0;
	highp vec2 coordinates = gl_FragCoord.xy / resolution;

	ditheredColor += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, random(coordinates));
	ditheredColor.r = round(ditheredColor.r *level)/level;
	ditheredColor.g = round(ditheredColor.g *level)/level;
	ditheredColor.b = round(ditheredColor.b *level)/level;

	outColor = vec4(vec3(ditheredColor), 1.0);
    
    //outColor = vec4(c,1.0);
}