#version 330 core

layout(location=0) in vec3 vertexPosition_modelspace;
layout(location=1) in vec2 uv;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

out vec2 vUV;

void main(){
    gl_Position = projection_matrix * model_view_matrix * vec4(vertexPosition_modelspace, 1.0f);
    vUV = uv;
}

