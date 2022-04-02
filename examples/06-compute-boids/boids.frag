#version 450

layout (location = 0) in vec3 color;
layout (location = 0) out vec4 fColor;

void main(){
    fColor = vec4(0.1, 0.8, 0.6, 1.0);
}