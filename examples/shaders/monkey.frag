#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inColor;

layout (location = 0) out vec4 fragColor;

const vec3 sunPos = vec3(0.0, 5.0, 0.0);
const vec3 sunColor = vec3(0.9, 0.9, 0.9);
const vec3 ambientColor = vec3(0.5, 0.5, 0.5);
const vec3 viewDir = vec3(0.0, 0.0, 0.0);

vec4 calculateLighting(vec4 baseColor){
    const float ambientStength = 0.5;
    vec3 ambient = ambientStength * ambientColor;

    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(sunPos - inPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunColor;

    float specularStrength = 0.05;
    vec3 viewDir = normalize(viewDir - inPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * sunColor;

    return vec4((ambient + diffuse + specular), 1.0) * baseColor;
}

void main(){
    fragColor = calculateLighting(vec4(inColor, 1.0));
}