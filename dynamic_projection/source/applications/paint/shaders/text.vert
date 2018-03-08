#version 330 core
layout(location=0) in vec3 vertexPosition_screenspace;
layout(location=1) in vec2 UV;
layout(location=2) in vec4 color;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

out vec2 vUV;
out vec4 vColor;

void main(){
    //vec4 transformed_z = projection_matrix * model_view_matrix * vec4(vertexPosition_screenspace, 1);
    gl_Position = vec4(projection_matrix * model_view_matrix * vec4(vertexPosition_screenspace , 1));

    vUV = UV;
    vColor = color;
}
