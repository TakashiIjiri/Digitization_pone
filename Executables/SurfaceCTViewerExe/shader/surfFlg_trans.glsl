//based on [http://www.slis.tsukuba.ac.jp/~fujisawa.makoto.fu/cgi-bin/wiki/index.php?GLSL%A4%CB%A4%E8%A4%EB%A5%D5%A5%A9%A5%F3%A5%B7%A5%A7%A1%BC%A5%C7%A5%A3%A5%F3%A5%B0]


uniform vec4 u_eyePos  ;
uniform vec4 u_cuboid  ;
uniform sampler3D u_img3_vol ;
uniform sampler2D u_img2_tex ;
uniform sampler3D u_img3_flg ;


varying vec3 v_posW      ; //position in world cd
varying vec3 v_normW     ; //norma in world cd
varying vec3 v_posNormalW; //position in world cd normalized in [0,1]x[0,1]x[0,1]


const vec3 La  = vec3(1,1,1);
const vec3 Ld  = vec3(0.5,0.5,0.5);   
const vec3 Ls  = vec3(1,1,1); 
const vec3 Lp1 = vec3(1000, 1000, 1000); 
const vec3 Lp2 = vec3(1000,-1000,-1000); 

const vec3  Ks = vec3(1,1,1); 
const float Kshine = 128;

const float COEF_1_255     = 0.00392;
const float COEF_1_255_2   = 0.00196;

 
void main(void)
{

	float flg = texture3D(u_img3_flg, v_posNormalW.xyz).x;

	if( flg >= 3 * COEF_1_255_2 ) 
	{
		discard;
	}

	
	vec3 albedo = texture2D(u_img2_tex, gl_TexCoord[0].st).xyz;

	// Ž‹ü & –@ü
    vec3 V = normalize(u_eyePos.xyz-v_posW.xyz); 
    vec3 N = normalize(v_normW);                 
	
	//ŒõŒ¹•ûŒü2ŒÂ
	vec3 L1 = normalize( Lp1 - v_posW);    
	vec3 L2 = normalize( Lp2 - v_posW);    
  
    float diff1 = max(dot(L1, N), 0.0);
    float diff2 = max(dot(L2, N), 0.0);
    vec3 diffuse = albedo * Ld * (diff1 + diff2);
 
    vec3 specular = vec3(0.0);
	/*no spec
    if(diff1 > 0.0)
	{
        vec3 H = normalize(L1+V);
        float specularLight = pow(max(dot(H, N), 0.0), Kshine);
        specular += Ks*Ls*specularLight;
    }
    if(diff2 > 0.0)
	{
        vec3 H = normalize(L2+V);
        float specularLight = pow(max(dot(H, N), 0.0), Kshine);
        specular += Ks*Ls*specularLight;
    }
	*/
    vec3 ambient = albedo*La;     
    gl_FragColor.xyz = ambient + diffuse + specular;
	gl_FragColor.w = 0.2;

}