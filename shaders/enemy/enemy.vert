#version 460
layout(constant_id = 0) const int partID = 0;  // Default to Torso

layout(push_constant) uniform PushConstants {
	mat4 modelMatrix;
    vec4 timeSpeedAmplitudeTurning;
} push;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
} ubo;

// Body part definitions
const int PART_TORSO = 0;
const int PART_THIGH = 1;
const int PART_SHIN = 2;
const int PART_FOOT = 3;
const int PART_HIP = 4;
const int PART_THREEARM = 5;
const int PART_FOREARM = 6;
const int PART_HAND = 7;

#define PI 3.1415926538

vec3 pos = position;
float t = push.timeSpeedAmplitudeTurning.x;
float amp = push.timeSpeedAmplitudeTurning.z;
float sp = push.timeSpeedAmplitudeTurning.y;

float nonUniformRun(float time) {
    return sin(time) * pow(abs(sin(time * 0.5)), 0.5);  // Squash upward, accelerate downward
}

void footTransform()
{
    float theta = 0.4 * amp * (sin(t-1.5)-1);
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    float ankleY = 0.145885; 

    theta = 0.1 * amp * (sin(t) + push.timeSpeedAmplitudeTurning.w/2.0);
    c = cos(theta);
    s = sin(theta);
    rot = mat2(c, -s, s, c);

    pos.y += ankleY;
    pos.xy = rot * pos.xy; // rotate left and right
    pos.y -= ankleY;
}

void shinTransform()
{
    float theta = sp * 0.06 * amp * (nonUniformRun(t + 0.5)+1);
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    float kneeY = 0.845943; 

    // Move to knee origin, rotate, then move back

    pos.y += kneeY;
    pos.yz = rot * pos.yz;
    pos.y -= kneeY;
}

void thighTransform()
{
    const float reverseSwingBias = 0.05;
    const float overalSwingAmount = 1.6;
    float theta = overalSwingAmount * amp * (nonUniformRun(t - PI/2) + reverseSwingBias);
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    float hipY = 1.75894;  

    pos.y += hipY;
    pos.yz = rot * pos.yz;
    pos.y -= hipY;

    theta = 0.1 * amp * (sin(t) + push.timeSpeedAmplitudeTurning.w/2.0);
    c = cos(theta);
    s = sin(theta);
    rot = mat2(c, -s, s, c);

    pos.y += hipY;
    pos.xy = rot * pos.xy; // rotate left and right
    pos.y -= hipY;

}

void freeArmTransform()
{
    float theta = 0.7 * amp * (cos(t - PI/2) - 0.2);
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    float freeY = 2.59814; 

    // Move to hip origin, rotate, then move back

    pos.y += freeY;
    pos.yz = rot * pos.yz;
    pos.y -= freeY;
}

void foreArmTransform()
{
    float theta = 0.5 * amp * (cos(t - PI/2) + 0.2);
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    float foreY = 2.0824; 

    // Move to hip origin, rotate, then move back

    pos.y += foreY;
    pos.yz = rot * pos.yz;
    pos.y -= foreY;
}

void hipTransform()
{
    float theta = 0.35 * amp * (cos(t));
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    pos.xz = rot * pos.xz;
}

void torsoRotTransform()
{
    float theta = 0.25 * amp * (cos(t));
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    pos.xz = rot * pos.xz;
}

void torsoTransform()
{
    const float torsoForwardLean = 48.f;
    if(partID < 0) // do for other leg
    {
        t = t + PI;
    }

    float theta = 0.1 * amp * (sin(t) - push.timeSpeedAmplitudeTurning.w);
    float c = cos(theta);
    float s = sin(theta);
    mat2 rot = mat2(c, -s, s, c);

    pos.xy = rot * pos.xy; // rotate left and right

    theta = 0.005 * (sin(2*t)+(torsoForwardLean * amp));
    c = cos(theta);
    s = sin(theta);
    rot = mat2(c, -s, s, c);

    pos.yz = rot * pos.yz;
    pos.y += 0.1 * amp * (sin(2*t));


}

void main() {
    if(partID < 0) // do for other leg
    {
        t = t + PI;
        pos.x *= -1;
    }

    if (abs(partID) == PART_HAND)
    {
        foreArmTransform();
        freeArmTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_FOREARM)
    {
        foreArmTransform();
        freeArmTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_THREEARM)
    {
        freeArmTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_HIP)
    {
        hipTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_FOOT)
    {
        footTransform();
        shinTransform();
        thighTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_SHIN)
    {
        shinTransform();
        thighTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_THIGH)
    {
        thighTransform();
        torsoTransform();
    }
    else if (abs(partID) == PART_TORSO)
    {
        torsoRotTransform();
        torsoTransform();
    }


	vec4 positionWorld =  push.modelMatrix * vec4(pos, 1.0);
	vec4 viewPosition = ubo.view * positionWorld;
	gl_Position = ubo.projection * viewPosition;

	// Use the negative Z value (depth) in view space as a proxy for distance
    float depth = viewPosition.z;
	
    // Map depth to a darkness factor
    float vDarknessFactor = clamp(1.0 - (depth/16), 0.0, 1.0);

	fragColor =	vec3(mix(color.r * vDarknessFactor, color.r, step(0.9, color.r)),
        mix(color.g * vDarknessFactor, color.g, step(0.9, color.g)),
        mix(color.b * vDarknessFactor, color.b, step(0.9, color.b)));
        
    fragPos = gl_Position.xyz;
}
