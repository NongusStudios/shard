#version 450

// in
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inColor;

// out
layout (location = 0) out vec3 outPos;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec3 outColor;

// uniform
layout (set = 0, binding = 0) uniform UBO {
    mat4 proj;
    mat4 view;
    mat4 model;
} ubo;

void main(){
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
    outPos = inPos;
    outUv = inUv;
    outColor = inColor;
}