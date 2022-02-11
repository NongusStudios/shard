#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inColor;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outColor;

layout (set = 0, binding = 0) uniform UBO {
    mat4 proj;
    mat4 view;
    mat4 model;
} ubo;

void main(){
    vec3 pos = vec3(inPos);
    pos.z += 0.5;
    gl_Position = /* ubo.proj * ubo.view * ubo.model */ vec4(pos, 1.0);
    outPos = inPos;
    outUv = inUv;
    outNormal = inNormal;
    outColor = inColor;
}