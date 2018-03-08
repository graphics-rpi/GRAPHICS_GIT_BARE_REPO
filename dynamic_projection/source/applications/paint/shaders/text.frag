#version 330 core

in vec2 vUV;
in vec4 vColor;

out vec4 color;

uniform sampler2D texture_sampler;

void main()
{
    color = vec4(1.0f, 1.0f, 1.0f, texture( texture_sampler, vUV ).r) * vColor;
}

/*
void main()
{
    vec4 smoothedColor;
    smoothedColor.rgb = vColor.rgb;

    float mask = texture(texture_sampler, vUV).r ;
    if( mask < 0.5 ){
        smoothedColor.a = 0.0;
    }
    else{
        smoothedColor.a = 1.0;
    }

    smoothedColor.a *= smoothstep(0.25, 0.75, mask);
    color = smoothedColor;
}
*/
