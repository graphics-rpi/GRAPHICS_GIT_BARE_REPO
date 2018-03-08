#version 330 core

layout(points) in;

// Max Verticies is calculated by 3 * CIRCLE_RESOLUTION, this must be defined at compile time unfortunately,
// so this value has to change if we want to change the button resolution
layout(triangle_strip, max_vertices=60) out;

// Input from vertex shader
in vec4 vColor[];
in float vRadius[];

// Output to fragment shader
out vec4 gColor;

const float PI = 3.14159;
const int CIRCLE_RESOLUTION = 20;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

uniform int magnify;

uniform int num_cursors;
uniform vec2 screen_resolution;

uniform vec2[16] cursor_positions;
uniform float[16] cursor_strengths;
uniform int[16] cursor_statuses;

void main(){
    gColor = vColor[0];
    for(int i = 0; i < CIRCLE_RESOLUTION; i++){
        float current_angle = (i % CIRCLE_RESOLUTION) * 2.0f * PI / float(CIRCLE_RESOLUTION);
        float next_angle = (i+1 % CIRCLE_RESOLUTION) * 2.0f * PI / float(CIRCLE_RESOLUTION);

        vec3 current_point = gl_in[0].gl_Position.xyz + 
            (vRadius[0] * vec3( cos(current_angle), sin(current_angle), 0.0f));
        
        gl_Position = (projection_matrix * model_view_matrix * vec4(current_point, 1.0f)) ; 
        EmitVertex();

        gl_Position = (projection_matrix * model_view_matrix * vec4(gl_in[0].gl_Position.xyz, 1.0f)) ;
        EmitVertex();

        vec3 next_point = gl_in[0].gl_Position.xyz + 
            (vRadius[0] * vec3( cos(next_angle), sin(next_angle), 0.0f));
        gl_Position = (projection_matrix * model_view_matrix * vec4(next_point, 1.0f)) ;
        EmitVertex();

        EndPrimitive();
    }
}
