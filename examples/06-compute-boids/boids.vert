#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;

layout (set = 0, binding = 0) uniform UBO {
    mat4 projection;
} ubo;

struct Boid{
    vec2  position;
    vec2  direction;
    float noiseOffset;
    vec3  color;
};
layout (std140,set = 0, binding = 1) readonly buffer BoidBuffer {
    Boid boids[];
} boids;

layout (location = 0) out vec3 color;

mat4 translate(mat4 model, vec2 pos){
    mat4 translateMat = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        pos.x, pos.y, 0.0, 1.0
    );
    return model * translateMat;
}
mat4 rotate(mat4 model, float rot){
    mat4 rotMat = mat4(
        cos(rot), -sin(rot), 0.0, 0.0,
        sin(rot),  cos(rot), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return model * rotMat;
}
mat4 scale(mat4 model, vec2 scale){
    mat4 scaleMat = mat4(
        scale.x, 0.0, 0.0, 0.0,
        0.0, scale.y, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return model * scaleMat;
}

void main(){
    Boid boid = boids.boids[gl_InstanceIndex];

    float angle = -atan(boid.direction.x, -boid.direction.y);
    
    mat4 model = mat4(1.0);
    model = translate(model, boid.position);
    model = rotate(model, angle);
    model = scale(model, vec2(16.0, 32.0));

    gl_Position = ubo.projection * model * vec4(inPos, 0.0, 1.0);

    color = boid.color;
}