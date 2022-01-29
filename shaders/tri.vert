#version 450

// in
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

// out
layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outColor;

void main(){
    gl_Position = vec4(inPos, 1.0);
    outPos = inPos;
    outColor = inColor;
}