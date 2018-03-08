#version 330 core

layout(lines) in;

//layout(line_strip, max_vertices=5) out;
layout(triangle_strip, max_vertices=6) out;

in vec4 vColor[];
in float vWidth[];

out vec4 gColor;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform vec2 screen_resolution;

void main(){
    gColor = vColor[0];
    vec2 line = gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy;
    vec2 normal = normalize( vec2(-1.0f * line.y, line.x) ) / screen_resolution;

    vec4 half_width = vec4(vWidth[0] * normal, 0.0f, 0.0f);

    vec4 a = (projection_matrix * model_view_matrix * gl_in[0].gl_Position) - half_width;
    vec4 b = (projection_matrix * model_view_matrix * gl_in[0].gl_Position) + half_width;
    vec4 c = (projection_matrix * model_view_matrix * gl_in[1].gl_Position) - half_width;
    vec4 d = (projection_matrix * model_view_matrix * gl_in[1].gl_Position) + half_width;

    gl_Position = b;
    EmitVertex();
    gl_Position = d;
    EmitVertex();
    gl_Position = a;
    EmitVertex();
    gl_Position = c;
    EmitVertex();
    EndPrimitive();

}
