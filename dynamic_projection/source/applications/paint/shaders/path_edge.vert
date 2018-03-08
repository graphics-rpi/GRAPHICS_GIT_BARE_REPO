#version 330 core
layout(location=0) in vec3 vertexPosition_modelspace;
layout(location=2) in vec4 vertexColor;
layout(location=3) in float vertexWidth;

// Transformation Matricies
uniform mat4 projection_matrix;
uniform mat4 model_view_matrix;

out vec4 vColor;
out float vWidth;

void main(){
    gl_Position = vec4(vertexPosition_modelspace, 1);
    vColor = vertexColor;
    vWidth = vertexWidth;
}
