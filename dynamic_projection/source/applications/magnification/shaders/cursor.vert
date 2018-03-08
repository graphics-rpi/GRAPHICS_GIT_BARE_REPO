#version 330 core
layout(location=0) in vec2 vertexPosition_modelspace;
layout(location=2) in vec3 color;

out vec3 vert_color;

void main(){
    gl_Position.xy = vertexPosition_modelspace;
    gl_Position.z = -0.1;
    gl_Position.w = 1.0;
    vert_color = color;
}
