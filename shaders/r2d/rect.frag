#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec4 fragColor;

layout (set = 1, binding = 0) uniform varUBO {
    mat4  model;
    vec4  color;
    vec4  borderColor;
    vec2  rectSize;
    bool  hasBorder;
    float borderSize;
} var;

vec4 getBorder(float aspect, float borderSize, vec2 uv){
    float maxX = 1.0 - borderSize;
    float minX = borderSize;
    float maxY = maxX / aspect;
    float minY = minX / aspect;

    if (uv.x < maxX && uv.x > minX && uv.y < maxY && uv.y > minY)
        return var.color/255.0;
    return var.borderColor/255.0;
}

void main(){
    if(var.hasBorder){
        float aspect = var.rectSize.x/var.rectSize.y;
        fragColor = getBorder(aspect, var.borderSize, inUv);
        return;
    }
    fragColor = var.color/255;
}