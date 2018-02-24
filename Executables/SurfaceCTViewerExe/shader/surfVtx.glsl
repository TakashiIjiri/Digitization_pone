//based on [http://www.slis.tsukuba.ac.jp/~fujisawa.makoto.fu/cgi-bin/wiki/index.php?GLSL%A4%CB%A4%E8%A4%EB%A5%D5%A5%A9%A5%F3%A5%B7%A5%A7%A1%BC%A5%C7%A5%A3%A5%F3%A5%B0]

uniform vec4 u_eyePos  ;
uniform vec4 u_cuboid  ;

varying vec3 v_posW      ; //position in world cd
varying vec3 v_normW     ; //norma in world cd
varying vec3 v_posNormalW; //position in world cd normalized in [0,1]x[0,1]x[0,1]

void main(void)
{
	v_posW       = gl_Vertex.xyz;
	v_normW      = gl_Normal;
	v_posNormalW = gl_Vertex.xyz / u_cuboid.xyz;

    gl_Position = ftransform(); //projectionM * modelView * position (正規化座標系)
	gl_TexCoord[0] = gl_MultiTexCoord0;      

	//以下よく利用するけど今回は不用
    //v_pos  = (gl_ModelViewMatrix*gl_Vertex).xyz;  // modelView * position (カメラ座標系)
    //v_norm = gl_NormalMatrix * gl_Normal;         // modelView * normal   (カメラ座標系)
}