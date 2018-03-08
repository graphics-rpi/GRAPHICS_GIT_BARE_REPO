uniform sampler2D tex;
uniform vec3 weights;
	
void main()
{
	vec4 color = texture2D(tex,gl_TexCoord[0].st);
	gl_FragColor = gl_Color * color * vec4(weights, 1.0);
}
