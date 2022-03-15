#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec4 fragColor;

layout (set = 1, binding = 0) uniform varUBO {
    mat4  model;
    vec4  color;
    vec2  srcPos;
    vec2  srcSize;
} var;
layout (set = 1, binding = 1) uniform sampler2D sprite;

void main(){
    fragColor = texture(sprite, inUv) * (var.color/255.0);
}