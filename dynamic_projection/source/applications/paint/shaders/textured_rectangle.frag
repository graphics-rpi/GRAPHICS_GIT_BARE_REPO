#version 330 core
in vec2 vUV;

out vec3 color;

uniform sampler2D texture_sampler;

void main(){
    color = texture( texture_sampler, vUV ).rgb;
    //color = vec3(1.0f, 0.0f, 0.0f);
}
