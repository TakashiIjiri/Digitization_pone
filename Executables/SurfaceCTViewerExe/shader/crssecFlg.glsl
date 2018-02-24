//based on [http://www.slis.tsukuba.ac.jp/~fujisawa.makoto.fu/cgi-bin/wiki/index.php?GLSL%A4%CB%A4%E8%A4%EB%A5%D5%A5%A9%A5%F3%A5%B7%A5%A7%A1%BC%A5%C7%A5%A3%A5%F3%A5%B0]

varying vec3 v_posNorm ; //position nomarized in  [0,1]x[0,1]x[0,1]

uniform sampler3D u_img3_vol  ;
uniform sampler3D u_img3_flg  ;
 
const float COEF_1_255     = 0.00392;
const float COEF_1_255_2   = 0.00196;

void main(void)
{
	float flg = texture3D(u_img3_flg, v_posNorm.xyz).x;
	if(  flg < 1 * COEF_1_255_2 ) discard;
	
	float c   = texture3D(u_img3_vol, v_posNorm.xyz).x;
	gl_FragColor = vec4(c,c,c,1);
}