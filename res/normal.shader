//startfrag
#version 460 core
out vec4 gColor;

layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform Sampler2D tex;

void main(){
    	gColor = texture(tex, uv);
}
//endfrag

//startvert
#version 460 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUv;
layout(location = 2) in vec3 vertexNormal;

layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 normal;

uniform mat4 transform;
uniform mat4 view;

void main(){
	normal = mat3(transpose(inverse(transform))) * vertexNormal;
    	uv = vertexUv;
	gl_Position = view * transform * vec4(vertexPos, 1);	
}
//endvert
