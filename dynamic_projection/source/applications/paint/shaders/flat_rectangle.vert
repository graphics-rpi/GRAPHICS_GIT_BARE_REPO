#version 330 core

layout(location=0) in vec3 vertexPosition_modelspace;
layout(location=2) in vec4 color;

uniform int magnify;

uniform int num_cursors;
uniform vec2 screen_resolution;

uniform vec2[16] cursor_positions;
uniform float[16] cursor_strengths;
uniform int[16] cursor_statuses;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

out vec4 vColor;

void main(){
    gl_Position = projection_matrix * model_view_matrix * vec4(vertexPosition_modelspace, 1.0f);
    vColor = color;
}
