#version 330 core

layout(location=0) in vec3 vertexPosition_modelspace;
layout(location=2) in vec4 color;
layout(location=3) in float radius;

out vec4 vColor;
out float vRadius;

void main(){
    gl_Position = vec4(vertexPosition_modelspace, 1.0f);
    vColor = color;
    vRadius = radius;
}
