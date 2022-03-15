#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec2 outPos;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec4 outColor;

layout (set = 0, binding = 0) uniform constUBO {
    mat4 projection;
    mat4 view;
} constant;
layout (set = 1, binding = 0) uniform varUBO {
    mat4  model;
    vec4  color;
    vec2  srcPos;
    vec2  srcSize;
} var;

void main(){
    gl_Position = constant.projection * constant.view * var.model * vec4(inPos, 0.0, 1.0);
    outPos   = inPos;
    outUv    = inUv;
    outColor = inColor;
}