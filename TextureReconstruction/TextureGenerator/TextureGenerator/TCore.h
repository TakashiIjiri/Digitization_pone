#pragma once



#include "../../Common/CameraInfo.h"
#include "../../Common/timage.h"


class TPhotoInfo
{
public:
	TImage2D    m_img   ;
	TImage2D    m_imgBin;
	CameraParam m_cam   ;
	
	TPhotoInfo(){}
    TPhotoInfo(const TPhotoInfo &p ){ 
		m_cam    = p.m_cam;
		m_img    = p.m_img;
		m_imgBin = p.m_imgBin;
	}

	TPhotoInfo& operator= (const TPhotoInfo  &p){ 
		m_cam    = p.m_cam;
		m_img    = p.m_img;
		m_imgBin = p.m_imgBin;
		return *this; 
	}


};






class TCore
{
	TCore();

	TTexMesh m_texMesh;
	TImage2D m_finTex;

	vector< TPhotoInfo > m_photos;
	vector< TImage2D   > m_textures;
	vector< float*     > m_texAngle;

public:
	~TCore();
	static TCore* getInst(){ static TCore p; return &p;}

	void drawScene();

	void computeTexture();
	void drawPolygonIDs();
};




