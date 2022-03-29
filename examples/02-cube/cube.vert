#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform TransformData {
    mat4 proj;
    mat4 model;
} t;

void main(){
    gl_Position = t.proj * t.model * vec4(inPos, 1.0);
    outColor = inColor;
}