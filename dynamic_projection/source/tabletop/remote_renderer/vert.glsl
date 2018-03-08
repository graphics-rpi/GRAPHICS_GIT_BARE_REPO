void main(){
     gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
     gl_Normal = gl_NormalMatrix * gl_Normal;
     gl_FrontColor = vec4(pow(gl_Color.r, 1/2.2), pow(gl_Color.g, 1/2.2), pow(gl_Color.b, 1/2.2), gl_Color.a);
     gl_TexCoord[0] = gl_MultiTexCoord0;
     
}
