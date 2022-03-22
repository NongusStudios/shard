#version 450

// binding 0
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;

struct InstanceData{
    mat4  model;
    vec4  color;
    vec4  borderColor;
    vec2  rectSize;
    bool  hasBorder;
    float borderSize;
};

// binding 1
layout (location = 3) in mat4  model;
layout (location = 4) in vec4  color;
layout (location = 5) in vec4  borderColor;
layout (location = 6) in vec2  rectSize;
layout (location = 7) in bool  hasBorder;
layout (location = 8) in float borderSize;

layout (location = 0) out vec2  outPos;
layout (location = 1) out vec2  outUv;
layout (location = 2) out vec4  outColor;
layout (location = 3) out mat4  outModel;
layout (location = 4) out vec4  outRectColor;
layout (location = 5) out vec4  outBorderColor;
layout (location = 6) out vec2  outRectSize;
layout (location = 7) out bool  outHasBorder;
layout (location = 8) out float outBorderSize;

layout (set = 0, binding = 0) uniform constUBO {
    mat4 projection;
    mat4 view;
} constant;

void main(){
    gl_Position = constant.projection * constant.view *
                  model * vec4(inPos, 0.0, 1.0);
    outPos         = inPos;
    outUv          = inUv;
    outColor       = inColor;
    outModel       = model;
    outRectColor   = color;
    outBorderColor = borderColor;
    outRectSize    = rectSize;
    outHasBorder   = hasBorder;
    outBorderSize  = borderSize;
}