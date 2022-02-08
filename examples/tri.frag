#version 450

// in
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

// out
layout (location = 0) out vec4 frag_color;

void main(){
    frag_color = vec4(color, 1.0);
}