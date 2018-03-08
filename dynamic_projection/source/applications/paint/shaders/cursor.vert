#version 330 core

layout(location=0) in vec2 vertex_screenSpace;
layout(location=2) in vec3 color;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

out vec3 vColor;

void main(){
    gl_Position = projection_matrix * model_view_matrix * vec4(vertex_screenSpace, 0.0, 1.0);
    vColor = color;
}
