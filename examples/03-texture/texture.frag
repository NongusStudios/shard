#version 450

layout (location = 0) in vec3 fColor;
layout (location = 1) in vec2 fUv;

layout (location = 0) out vec4 fragColor;

layout (set = 0, binding = 0) uniform sampler2D tex;

void main(){
    fragColor = texture(tex, fUv);
}