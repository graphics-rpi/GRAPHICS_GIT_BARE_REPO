uniform sampler2D tex;
void main()
{
	vec4 color = texture2D(tex,gl_TexCoord[0].st);
  //vec4 color = vec4( 0.0, 0.0,  1.0, 1.0);
	gl_FragColor = gl_Color * color;
}
