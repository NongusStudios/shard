#version 450

// in
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 color;
layout (set = 0, binding = 1) uniform sampler2D texSampler;
layout (set = 0, binding = 2) uniform sampler2D texSampler2;

// out
layout (location = 0) out vec4 frag_color;

void main(){
    frag_color = texture(texSampler, uv);//* vec4(color, 1.0);
    float gray = dot(frag_color.xyz, vec3(0.299, 0.587, 0.114));
    frag_color = vec4(vec3(gray), 1.0) * texture(texSampler2, uv);
}