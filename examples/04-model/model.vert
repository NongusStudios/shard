#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inColor;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec3 outNormal;

layout (set = 0, binding = 0) uniform UBO {
    mat4 proj;
    mat4 model;
} ubo;

void main(){
    gl_Position = ubo.proj * ubo.model * vec4(inPos, 1.0);
    outPos = vec3(ubo.model * vec4(inPos, 1.0));
    outUv = inUv;
    outNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
}