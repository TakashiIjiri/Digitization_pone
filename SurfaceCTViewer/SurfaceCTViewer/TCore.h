#pragma once

#include "./COMMON/tmath.h"
#include "./COMMON/OglForCLI.h"
#include "./COMMON/OglImage.h"
#include "./COMMON/ttexmesh.h"

#include "GlslShader.h"

#pragma unmanaged 


class TCore
{
	bool m_bL, m_bR, m_bM;

	//volume info
	EVec3i m_reso  ;
	EVec3f m_pitch ;
	EVec3f m_cuboid;
	short  *m_volume;


	OglImage3D m_volumeOgl; //volume image

	//flg 0 (モデル外            ),
	//flg 1 (モデル内 & Cut除去  ),
	//flg 2 (モデル内 & Cut非除去),// モデル内は 1ring dilationあり
	OglImage3D m_volumeFlg; 

	//texture
	OGLImage2D4 m_texture;

	//surface mesh
	TTexMesh    m_surface;
	
	//cut stroke
	bool           m_bDrawStr;
	vector<EVec3f> m_stroke3D;
	vector<EVec2i> m_stroke2D;
	TTexMesh       m_CutSurface;


	GlslShader m_SurfaceShader;
	GlslShader m_SurfaceShader_trans ;
	GlslShader m_CrsSecShader      ;

	TCore();
public:
	~TCore();

	static TCore* getInst(){
		static TCore p;
		return &p;
	}

	void drawScene(EVec3d camP);
	void BtnDownL (EVec2i p, OglForCLI* ogl);
	void BtnDownR (EVec2i p, OglForCLI* ogl);
	void BtnDownM (EVec2i p, OglForCLI* ogl);
	void BtnUpL   (EVec2i p, OglForCLI* ogl);
	void BtnUpR   (EVec2i p, OglForCLI* ogl);
	void BtnUpM   (EVec2i p, OglForCLI* ogl);
	void MouseMove(EVec2i p, OglForCLI* ogl);

private:
	void loadModels();
	void updateCutSurface(const EVec3f &camP);
	void updateFlgVolByCutStr(OglForCLI* ogl);
};



#pragma managed

