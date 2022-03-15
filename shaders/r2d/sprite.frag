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

vec4 calcTexture(){
    float minx = var.srcPos.x - var.srcSize.x / 2;
    float maxx = var.srcPos.x + var.srcSize.x / 2;

    float miny = var.srcPos.y - var.srcSize.y / 2;
    float maxy = var.srcPos.y + var.srcSize.y / 2;

    vec2 correctUv = inUv;

    correctUv.x = minx + (maxx - minx) * inUv.x;
    correctUv.y = maxy - (maxy - miny) * inUv.y;
    correctUv.y = 1 - correctUv.y;

    return texture(sprite, correctUv);
}

void main(){
    fragColor = calcTexture();
}