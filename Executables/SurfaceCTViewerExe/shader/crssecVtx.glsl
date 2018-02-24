//based on [http://www.slis.tsukuba.ac.jp/~fujisawa.makoto.fu/cgi-bin/wiki/index.php?GLSL%A4%CB%A4%E8%A4%EB%A5%D5%A5%A9%A5%F3%A5%B7%A5%A7%A1%BC%A5%C7%A5%A3%A5%F3%A5%B0]

uniform vec4 u_eyePos  ;
uniform vec4 u_cuboid  ;

varying vec3 v_posNorm ; //position nomarized in  [0,1]x[0,1]x[0,1]

 
void main(void)
{
	v_posNorm = gl_Vertex.xyz / u_cuboid.xyz;
    gl_Position = ftransform();
}