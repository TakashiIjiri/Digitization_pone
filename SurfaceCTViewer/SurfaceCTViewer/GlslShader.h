#pragma once


/* ------------------------------------
class GlslShader*

date 2017/07/07 
written by Takashi Ijiri, Tomofumi Naritra

This file contains multiple classes managing glsl shaders.
-------------------------------------*/


#include "./COMMON/OglForCLI.h"

#include <string>
using namespace std;







class GlslShader
{
	const string m_vtxFname;
	const string m_frgFname;
	GLuint       m_gl2Program;
	bool         m_bInit;

public:

	GlslShader( string vtxFname, string frgFname) : 
		m_vtxFname(vtxFname) , 
		m_frgFname(frgFname)
	{
		m_bInit = false;
	}

	~GlslShader(){}

	void bind
	(
		int UnitID_vol ,//3D
		int UnitID_flg ,//3D 
		int UnitID_tex ,//2D
		EVec3i reso    ,
		EVec3d camPos  ,
		EVec3d cuboid  );

	void unbind()
	{
		glUseProgram(0);
	}
};
