#version 330 core
in vec2 gUV;
out vec4 color;

uniform sampler2D texture_sampler;

void main(){
    color = texture(texture_sampler, gUV).rgba;
}
