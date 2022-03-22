#version 450

struct InstanceData{
    mat4  model;
    vec4  color;
    vec4  borderColor;
    vec2  rectSize;
    bool  hasBorder;
    float borderSize;
};

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;
layout (location = 3) in mat4  model;
layout (location = 4) in vec4  color;
layout (location = 5) in vec4  borderColor;
layout (location = 6) in vec2  rectSize;
layout (location = 7) in bool  hasBorder;
layout (location = 8) in float borderSize;

layout (location = 0) out vec4 fragColor;

vec4 getBorder(float aspect, float borderSize, vec2 uv){
    float maxX = 1.0 - borderSize;
    float minX = borderSize;
    float maxY = maxX / aspect;
    float minY = minX / aspect;

    if (uv.x < maxX && uv.x > minX && uv.y < maxY && uv.y > minY)
        return color;
    return borderColor;
}

void main(){
    if(hasBorder){
        float aspect = rectSize.x/rectSize.y;
        fragColor = getBorder(aspect, borderSize, inUv);
        return;
    }
    fragColor = color;
}