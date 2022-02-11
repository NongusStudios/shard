#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inColor;

layout (location = 0) out vec4 fragColor;

vec4 calculateLighting(vec4 baseColor){
    return baseColor;
}

void main(){
    fragColor = calculateLighting(vec4(inColor, 1.0));
}