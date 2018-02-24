#include "stdafx.h"


#include "GlslShader.h"


//床井先生のページ参照 http://marina.sys.wakayama-u.ac.jp/~tokoi/?date=20051006
// シェーダーのソースプログラムをメモリに読み込む by Tokoi-senssei
inline int t_readShaderSource(GLuint shader, const char *f)
{
	string path = string(__argv[0] );
	path = path.substr( 0, path.length() - 19);
	
	path += string( "\\" ) + string(f);

	//ファイルを開く
	FILE *fp = fopen( path.c_str(), "rb");
	if (fp == NULL) {
		perror(path.c_str());
		return -1;
	}

	//ファイルの末尾に移動し現在位置（つまりファイルサイズ）を得る
	fseek(fp, 0L, SEEK_END);
	GLsizei length = ftell(fp);

	//ファイルサイズのメモリを確保 
	const GLchar *source = (GLchar *)malloc(length);
	if (source == NULL) {
		fprintf(stderr, "Could not allocate read buffer.\n");
		return -1;
	}

	// ファイルを先頭から読み込む 
	fseek(fp, 0L, SEEK_SET);
	int ret = fread((void *)source, 1, length, fp) != (size_t)length;
	fclose(fp);

	// シェーダのソースプログラムのシェーダオブジェクトへの読み込み
	if (ret) fprintf(stderr, "Could not read file: %s.\n", path.c_str());
	else glShaderSource(shader, 1, &source, &length);

	free((void *)source);

	return ret;
}

//by Tokoi Sensei
inline void printShaderInfoLog(GLuint shader)
{
  GLsizei bufSize;
  
  /* シェーダのコンパイル時のログの長さを取得する */
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1) {
    GLchar *infoLog = (GLchar *)malloc(bufSize);
    
    if (infoLog != NULL) {
      GLsizei length;
      
      /* シェーダのコンパイル時のログの内容を取得する */
      glGetShaderInfoLog(shader, bufSize, &length, infoLog);
      fprintf(stderr, "InfoLog:\n%s\n\n", infoLog);
      free(infoLog);
    }
    else
      fprintf(stderr, "Could not allocate InfoLog buffer.\n");
  }
}






inline bool t_initializeShader
(
	const char* vtxFname,
	const char* frgFname,
	GLuint &_gl2Program
)
{
	fprintf( stderr , "initializeShader\n");
	GLuint  vertShaderId;
	GLuint  fragShaderId;

	// gen shader object and read shader programs
	vertShaderId = glCreateShader(GL_VERTEX_SHADER  );
	fragShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	if ( t_readShaderSource(vertShaderId, vtxFname ) ) return false;
	if ( t_readShaderSource(fragShaderId, frgFname ) ) return false;


	// compile vertex shader and fragment shader 
	GLint isCompiled;
	glCompileShader(vertShaderId);
	glGetShaderiv(vertShaderId, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		fprintf(stderr, "Compile error in vertex shader.\n");
		printShaderInfoLog(vertShaderId);
		return false;
	}
	glCompileShader(fragShaderId);
	glGetShaderiv(fragShaderId, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		fprintf(stderr, "Compile error in fragment shader.\n");
		printShaderInfoLog(fragShaderId);
		return false;
	}


	// generate program object 
	_gl2Program = glCreateProgram();
	glAttachShader(_gl2Program, vertShaderId );
	glAttachShader(_gl2Program, fragShaderId );
	glDeleteShader(vertShaderId);
	glDeleteShader(fragShaderId);

	GLint isLined;
	glLinkProgram(_gl2Program);
	glGetProgramiv(_gl2Program, GL_LINK_STATUS, &isLined);
	if (isLined == GL_FALSE) {
		fprintf(stderr, "Link error.\n");
		exit(1);
	}

	fprintf(stderr, "success initialize shader!!\n");

	return true;
}









void GlslShader::bind
(
	int UnitID_vol ,//3D
	int UnitID_flg ,//3D 
	int UnitID_tex ,//2D
	EVec3i reso    ,
	EVec3d camPos  ,	
	EVec3d cuboid  )
{	
	if (!m_bInit)
	{
		t_initializeShader(m_vtxFname.c_str(), m_frgFname.c_str(), m_gl2Program );
		m_bInit = true;
	}
	glUseProgram(m_gl2Program);
	glUniform1i( glGetUniformLocation(m_gl2Program, "u_img3_vol"  ), UnitID_vol );
	glUniform1i( glGetUniformLocation(m_gl2Program, "u_img3_flg"  ), UnitID_flg );
	glUniform1i( glGetUniformLocation(m_gl2Program, "u_img2_tex"  ), UnitID_tex );
	glUniform4f( glGetUniformLocation(m_gl2Program, "u_texCdOfst" ), (GLfloat)1.0/reso[0], (GLfloat)1.0/reso[1], (GLfloat)1.0/reso[2], 0);
	glUniform4f( glGetUniformLocation(m_gl2Program, "u_eyePos"    ), (GLfloat)camPos[0]  , (GLfloat)camPos[1]  , (GLfloat)camPos[2]  , 0);
	glUniform4f( glGetUniformLocation(m_gl2Program, "u_cuboid"    ), (GLfloat)cuboid[0]  , (GLfloat)cuboid[1]  , (GLfloat)cuboid[2]  , 0);
}

