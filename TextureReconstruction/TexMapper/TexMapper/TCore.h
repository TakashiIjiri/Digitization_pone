#pragma once


#include "TOGL.h"
#include <string>
#include <vector>
#include "../Common/CameraInfo.h"


using namespace std;


class TargetImageInfo
{
public:
	int      m_fX, m_fY, m_fZ;
	string   m_imgFname      ;
	string   m_imgBinFname   ;
};


class RendImageInfo
{
public:

	//vertex index / radius of search sphere 
	int     m_sphereIdx ;
	double  m_sphereRadi;

	//edge pixel num and pos
	int     m_FgEdgeN  ;
	EVec2d *m_FgEdgePix;
	EVec2d  m_FgCenter ;

	//values after coarse search
	double  m_foundDiff, m_foundAngle;

	RendImageInfo() {
		m_sphereIdx   = 0;
		m_sphereRadi  = 0;
		m_FgEdgeN     = 0;
		m_FgEdgePix   = 0;
		m_foundDiff   = DBL_MAX;
		m_foundAngle  = 0;
	}

	RendImageInfo(const RendImageInfo &src) {
		Set(src);
	}
	RendImageInfo &operator=(const RendImageInfo &src) {
		Set(src); return *this;
	}
	~RendImageInfo() {
		if (m_FgEdgePix) delete[] m_FgEdgePix;
	}

	void Set(const RendImageInfo &src) {
		m_sphereIdx   = src.m_sphereIdx;
		m_sphereRadi  = src.m_sphereRadi;
		m_FgEdgeN     = src.m_FgEdgeN;
		m_FgEdgePix   = new EVec2d[ m_FgEdgeN ];
		memcpy(m_FgEdgePix, src.m_FgEdgePix, sizeof(EVec2d) * m_FgEdgeN);
		m_FgCenter   = src.m_FgCenter;

		m_foundDiff   = src.m_foundDiff ;
		m_foundAngle  = src.m_foundAngle;

	}

	void Set(double radi, int sphI) {
		m_sphereRadi = radi;
		m_sphereIdx  = sphI;
	}
};








class TCore
{
	TTexMesh  m_sphere;
	TTexMesh  m_mesh   ;

	//target image info 
	string   m_dirPath;
	double   m_focalLen;
	double   m_camRadi0; //r0 in the paper
	vector< TargetImageInfo > m_imgs;

public:
	vector<RendImageInfo> m_rendImgs_coarseSearch;

private:
	TCore();

public:
	~TCore();
	static TCore* getInst() { static TCore p; return &p; }

	void drawScene();
	void cameraCalibration();

	double getCamradi0() { return m_camRadi0; }
	double getfocalLen() { return m_focalLen; }


};







