#version 330 core

layout(location=0) in vec3 vertexPosition_modelspace;
layout(location=1) in vec2 uv;
layout(location=3) in float radius;

out vec2 vUV;
out float vRadius;

void main(){
    gl_Position = vec4(vertexPosition_modelspace, 1.0f);
    vUV = uv;
    vRadius = radius;
}
