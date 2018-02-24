#include "stdafx.h"
#include "TCore.h"
#include "TexMapperView.h"
#include "MainFrm.h"
#include "ttexmesh.h"

#include <omp.h>


static EVec2d t_CenterOfForeground
(
	const int W,
	const int H,
	const byte *binImg
	)
{
	//gravity center 
	EVec2d gc(0, 0);
	int forePixN = 0;

	for (int y = 0; y < H; ++y)
	{
		for (int x = 0; x < W; ++x)
		{
			if (binImg[x + y*W])
			{
				forePixN++;
				gc[0] += x;
				gc[1] += y;
			}
		}
	}

	gc /= forePixN;
	gc[0] += 0.5;
	gc[1] += 0.5;

	return gc;
}



static void t_genDistImg
	(
	const int W,
	const int H, 
	const byte *binImg,

	float *distImg
	)
{
	typedef Eigen::Vector3i PixInfo; //(x,y,idx)

	const int WH = W*H;
	for( int i=0; i < WH; ++i) distImg[i] = -1;
	

	list< PixInfo > Q;
	for (int y = 1; y < H-1; ++y)
	for (int x = 1; x < W-1; ++x)
	{
		int i = x + y * W;

		if (binImg[i] && (binImg[i - 1] == 0 || binImg[i + 1] == 0 || binImg[i - W] == 0 || binImg[i + W] == 0))
		{
			distImg[i] = 0;
			Q.push_back(PixInfo(x, y, i));
		}
	}

	//growth
	while ( !Q.empty() )
	{
		const int x = Q.front()[0];
		const int y = Q.front()[1];
		const int i = Q.front()[2];
		const float v1 = distImg[i] + 1.0f;
		//const float v2 = distImg[i] + (float)sqrt(2.0);
		Q.pop_front();
		
		//if (x > 0  && y > 0  && distImg[i - 1 - W] < 0) { distImg[i - 1 - W] = v2;  Q.push_back(PixInfo(x - 1, y - 1, i - 1 - W)); }
		if (          y > 0  && distImg[i     - W] < 0) { distImg[i     - W] = v1;  Q.push_back(PixInfo(x    , y - 1, i     - W)); }
		//if (x <W-1 && y > 0  && distImg[i + 1 - W] < 0) { distImg[i + 1 - W] = v2;  Q.push_back(PixInfo(x + 1, y - 1, i + 1 - W)); }

		if (x > 0            && distImg[i - 1    ] < 0) { distImg[i - 1    ] = v1;  Q.push_back(PixInfo(x - 1, y    , i - 1    )); }
		if (x <W-1           && distImg[i + 1    ] < 0) { distImg[i + 1    ] = v1;  Q.push_back(PixInfo(x + 1, y    , i + 1    )); }

		//if (x > 0  && y <H-1 && distImg[i - 1 + W] < 0) { distImg[i - 1 + W] = v2;  Q.push_back(PixInfo(x - 1, y + 1, i - 1 + W)); }
		if (          y <H-1 && distImg[i     + W] < 0) { distImg[i     + W] = v1;  Q.push_back(PixInfo(x    , y + 1, i     + W)); }
		//if (x <W-1 && y <H-1 && distImg[i + 1 + W] < 0) { distImg[i + 1 + W] = v2;  Q.push_back(PixInfo(x + 1, y + 1, i + 1 + W)); }
	}
}




/*------------------------------------------

diff( img1, img2 )
image1 (given binary image ) is given by distance field image (img1_dist) 
image2 (rendered image     ) is given by depth image (img2_depth)

-------------------------------------------*/
static double t_Diff( 
	const int W,
	const int H, 
	const float *img1_dist,
	const float *img2_depth)
{
	double diff = 0;
	int    boundPixNum = 0;

	//note same as t_GetBoundPixFromDepthImg()
	for (int y = 1; y < H-1; ++y)
	for (int x = 1; x < W-1; ++x)
	{
		const int I = x + y*W;

		if (img2_depth[I] >= 0.9999) continue;

		if (img2_depth[I - 1] >= 0.9999 || img2_depth[I + 1] >= 0.9999 ||
			img2_depth[I - W] >= 0.9999 || img2_depth[I + W] >= 0.9999 )
		{
			diff += img1_dist[I];
			boundPixNum++;
		}
	}

	return diff / boundPixNum;
}



/*------------------------------------------
diff( img1, img2 )
img1 (binary   image) is given by distance image (int *img1_dist) field
img2 (rendered image) is given by depth    image (float *img2_depth)

returns (diff, rotX) 
-------------------------------------------*/
static void t_Diff_optimizeRot(
	const int W,
	const int H,
	const float  *img1_dist,
	const EVec2d &img1_gc,

	const int     img2_bPixN,
	const EVec2d *img2_bPix ,
	const EVec2d &img2_gc   ,

	double &res_diff, 
	double &res_angle)
{
	res_diff = DBL_MAX;

	for (int angle = 0; angle < 360; ++angle)
	{
		const double theta = angle / 360.0 * 2 * M_PI;
		const double sinT = sin(theta);
		const double cosT = cos(theta);

		double d = 0;

		int erPix = 0;
		for (int i = 0; i < img2_bPixN; ++i)
		{
			double X = img2_bPix[i][0] + 0.5 - img2_gc[0];
			double Y = img2_bPix[i][1] + 0.5 - img2_gc[1];
			int x1 = (int)(cosT * X - sinT * Y + img1_gc[0]);
			int y1 = (int)(sinT * X + cosT * Y + img1_gc[1]);

			if (0 <= x1 && 0 <= y1  && x1 < W && y1 < H) d += img1_dist[x1 + y1 * W];
			else erPix ++;
		}

		//if( erPix != 0) fprintf(stderr, "erPix(%d)", erPix);

		d /= img2_bPixN;

		if (d < res_diff)
		{
			res_diff  = d;
			res_angle = theta;
		}
	}
}







/*----------------------------------------------------------------------------
bruteforce search for the best matching camera positions

const byte      trgtImg      --> target binary image
const float    *trgtImg_dist --> target distance field image

if flgX ==  1 the system only considers cameraPosiion.x > 0
if flgX == -1 the system only considers cameraPosiion.x < 0

vector<EVec3d> &sphV_Diff --> output (minimum diff values when the cam is placed at sphere.m_verts[i])
------------------------------------------------------------------------------*/
static double t_coarseSearch
(
	const TTexMesh &sphere      ,
	const double    focalLen    ,
	const byte     *trgtImg     , //trgt image (binary)
	const float    *trgtImg_dist, //trgt Image (distance field)
	
	vector<RendImageInfo> &rendImgs ,
	vector<double>        &sphV_diff,
	CameraParam           &camera  

)
{
	const int WH = CAMERA_RES_W * CAMERA_RES_H;
	EVec2d trgtImg_gc = t_CenterOfForeground(CAMERA_RES_W, CAMERA_RES_H, trgtImg);


#pragma omp parallel for
	for (int i = 0; i < (int)rendImgs.size(); ++i)
	{
		RendImageInfo &rendImg = rendImgs[i];

		t_Diff_optimizeRot( CAMERA_RES_W, CAMERA_RES_H, trgtImg_dist, trgtImg_gc, 
			                rendImg.m_FgEdgeN  , rendImg.m_FgEdgePix, rendImg.m_FgCenter, 
							rendImg.m_foundDiff, rendImg.m_foundAngle );
	}
	

	// search best fitting camera position

	double minDiff = DBL_MAX;
	int    minRendImgIdx = 0;

	sphV_diff.clear();
	sphV_diff.resize(sphere.m_vSize, DBL_MAX);
	
	for (int i = 0; i < (int)rendImgs.size(); ++i)
	{
		const RendImageInfo &rendImg = rendImgs[i];

		sphV_diff[ rendImg.m_sphereIdx ] = min(sphV_diff[rendImg.m_sphereIdx], rendImg.m_foundDiff );

		if ( rendImg.m_foundDiff < minDiff)
		{
			minDiff       = rendImg.m_foundDiff ;
			minRendImgIdx = i;
		}
	}

	//seek init camera position ------------------------------------------------------------	
	//これより上では画像左下が原点（画像中心を原点にしたかったら、全てのpixel座標から(cx,cy)を引けばよい）
	//これよりしたでは画像中心が原点

	const double minAngleX = rendImgs[minRendImgIdx].m_foundAngle;
	const double minRadi   = rendImgs[minRendImgIdx].m_sphereRadi;
	const EVec2d rendImgC  = rendImgs[minRendImgIdx].m_FgCenter  ;
	const EVec3d sphV      = sphere.m_verts[ rendImgs[minRendImgIdx].m_sphereIdx ];
	

	//focal length in pixel unig
	const double fx = focalLen * CAMERA_RES_W / CAMERA_SENSOR_W;
	const double fy = focalLen * CAMERA_RES_H / CAMERA_SENSOR_H;
	const EVec2d Orig(CAMERA_RES_W / 2., CAMERA_RES_H / 2.);

	double x0 = cos( minAngleX ) * (rendImgC[0] - Orig[0]) - sin( minAngleX ) * (rendImgC[1] - Orig[1]);
	double y0 = sin( minAngleX ) * (rendImgC[0] - Orig[0]) + cos( minAngleX ) * (rendImgC[1] - Orig[1]);
	double x1 = trgtImg_gc[0] - Orig[0];
	double y1 = trgtImg_gc[1] - Orig[1];

	double theta = atan2(-sphV[2], sphV[0]);
	double phi   = asin(  sphV[1]);
	double tx    = - minAngleX;
	//double ty1    = - atan(x0 / fx) + atan(x1 / fx);
	//double tz1    = - atan(y0 / fy) + atan(y1 / fy);
	double ty    = atan( (-x0+x1) / fx);
	double tz    = atan( (-y0+y1) / fy);

	//fprintf( stderr, "%.10f %.10f  %.10f %.10f\n", ty, ty1, tz, tz1 );
	camera.Set(theta, phi, minRadi, tx, ty, tz, focalLen);

	return rendImgs[minRendImgIdx].m_foundDiff;
}







//前景かつ境界pixelをサンプリング点に
static void t_getBoundPixArray
(
	const int    W,
	const int    H,
	const float *depth,

	EVec2d  &gc,
	int     &boundPixNum,
	EVec2d* &boundPix //will be allocated in the function
)
{

	boundPixNum = 0;
	gc = EVec2d(0, 0);
	int forePixN = 0;
	EVec2d *tmpBPix = new EVec2d[W*H];

	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x)
		{
			if (depth[x + y*W] >= 0.9999) continue;
			forePixN++;
			gc[0] += x;
			gc[1] += y;

			if ((x > 0 && depth[x - 1 + y*W] >= 0.9999) ||
				(x <W - 1 && depth[x + 1 + y*W] >= 0.9999) ||
				(y > 0 && depth[x + (y - 1)*W] >= 0.9999) ||
				(y <H - 1 && depth[x + (y + 1)*W] >= 0.9999))
			{
				tmpBPix[boundPixNum][0] = x;
				tmpBPix[boundPixNum][1] = y;
				boundPixNum++;
			}
		}

	gc /= forePixN;
	gc[0] += 0.5;
	gc[1] += 0.5;

	boundPix = new EVec2d[boundPixNum];
	memcpy(boundPix, tmpBPix, sizeof(EVec2d) * boundPixNum);

	delete[] tmpBPix;
}





// precompute multiple rendering images from bruteforce searach candidates
// 1) render depth image from all coarse combinations (theta, phi, r, 0, 0, 0)
// 2) extract boundary pixels for all depth image 
// 3) store the bundary pixel positions into rendImgs_coarseSearch
static void preRendMultiImgsForCoarseSearch
(
	const TTexMesh  &sphere  ,
	const double    &camRadi0,
	const double    &focalLen,

	vector<RendImageInfo> &rendImgs_coarseSearch

)
{
	const int    RAD_S = -3;
	const int    RAD_E = 3;
	const int    RAD_N = RAD_E - RAD_S + 1;
	const double RAD_STEP = 10.0;

	const int  WH = CAMERA_RES_W * CAMERA_RES_H;
	const int  sphV_N = sphere.m_vSize;

	float *depthBuff_full = new float[WH * REND_NUM];
	float *depths[REND_NUM];
	for (int k = 0; k < REND_NUM; ++k) depths[k] = new float[WH];

	rendImgs_coarseSearch.resize(sphV_N * RAD_N);
	CameraParam cams[REND_NUM];

	clock_t total1 = 0, total2 = 0;

	for (int radiI = 0; radiI < RAD_N; ++radiI)
	{
		const double radi = camRadi0 + (radiI + RAD_S) * RAD_STEP;

		for (int sphI = 0; sphI < sphV_N; sphI += REND_NUM)
		{

			//1) set cameras & get depth images
			clock_t t0 = clock();
			for (int i = 0; i < REND_NUM; ++i) if (sphI + i < sphere.m_vSize)
			{
				const EVec3d &p = sphere.m_verts[sphI + i];
				cams[i] = CameraParam(atan2(-p[2], p[0]), asin(p[1]), radi, 0, 0, 0, focalLen);
			}

			CTexMapperView::getInst()->offscreenRend_genImages(cams, depthBuff_full, depths);

			//2) extract boundary pixel info 
			clock_t t1 = clock();

#pragma omp parallel for
			for (int i = 0; i < REND_NUM; ++i) if (sphI + i < sphere.m_vSize)
			{
				RendImageInfo &info = rendImgs_coarseSearch[radiI * sphV_N + sphI + i];
				info.Set(radi, sphI + i);
				t_getBoundPixArray(CAMERA_RES_W, CAMERA_RES_H, depths[i], info.m_FgCenter, info.m_FgEdgeN, info.m_FgEdgePix);
			}

			clock_t t2 = clock();

			total1 += t1 - t0;
			total2 += t2 - t1;
			if (sphI % 200 == 0) fprintf(stderr, "aa %d/%d %d %d\n", radiI, RAD_N, total1, total2);
		}
	}

	delete[] depthBuff_full;
	for (int k = 0; k < REND_NUM; ++k) delete[] depths[k];
}

















/*----------------------------------------------------------------------------
Steepest descent search to refine the camera position

const byte      trgtImg      --> target binary image
const float    *trgtImg_dist --> target distance field image
------------------------------------------------------------------------------*/
static void t_finSearch
(
	const byte       *trgtImg     , 
	const float      *trgtImg_dist, 
	const CameraParam initCam     ,
	double           &resDiff,
	CameraParam      &resCam  
	)
{

	const int WH = CAMERA_RES_W * CAMERA_RES_H;
	double       diff[REND_NUM];
	CameraParam  cams[REND_NUM];

	float *depthBuff_full = new float [WH * REND_NUM];
	float *depths[REND_NUM];
	for( int k=0; k < REND_NUM; ++k) depths[k] = new float[WH];


	resDiff = DBL_MAX;
	resCam  = initCam;
	for (int kkk = 0; kkk < 1000; ++kkk)
	{
		// set cameras compute gradint
		for (int i = 0; i < REND_NUM; ++i) cams[i] = resCam;

		cams[0 ].m_theta  -= FINSEARCH_OFST_ANGL;  cams[1 ].m_theta  += FINSEARCH_OFST_ANGL;
		cams[2 ].m_phi    -= FINSEARCH_OFST_ANGL;  cams[3 ].m_phi    += FINSEARCH_OFST_ANGL;
		cams[4 ].m_radius -= FINSEARCH_OFST_RADI;  cams[5 ].m_radius += FINSEARCH_OFST_RADI;
		cams[6 ].m_tx     -= FINSEARCH_OFST_ROT ;  cams[7 ].m_tx     += FINSEARCH_OFST_ROT ;
		cams[8 ].m_ty     -= FINSEARCH_OFST_ROT ;  cams[9 ].m_ty     += FINSEARCH_OFST_ROT ;
		cams[10].m_tz     -= FINSEARCH_OFST_ROT ;  cams[11].m_tz     += FINSEARCH_OFST_ROT ;

		//get depth 
		CTexMapperView::getInst()->offscreenRend_genImages( cams, depthBuff_full, depths);

		//compute diff
#pragma omp parallel for
		for (int t = 0; t < 12; ++t)
		{
			diff[t] = t_Diff(CAMERA_RES_W, CAMERA_RES_H, trgtImg_dist, depths[t]);
		}

		//calc gradient --> (smiple) backtracking line search
		double dir[6] = {
			H_ANGLE * (diff[ 1] - diff[0])  / (2 * FINSEARCH_OFST_ANGL),
			H_ANGLE * (diff[ 3] - diff[2])  / (2 * FINSEARCH_OFST_ANGL),
			H_RADI  * (diff[ 5] - diff[4])  / (2 * FINSEARCH_OFST_RADI),
			H_ROT   * (diff[ 7] - diff[6])  / (2 * FINSEARCH_OFST_ROT ),
			H_ROT   * (diff[ 9] - diff[8])  / (2 * FINSEARCH_OFST_ROT ),
			H_ROT   * (diff[11] - diff[10]) / (2 * FINSEARCH_OFST_ROT )
		};

		for (int i = 0; i < REND_NUM; ++i)
		{
			cams[i].m_theta = resCam.m_theta - i * dir[0];
			cams[i].m_phi   = resCam.m_phi   - i * dir[1];
			cams[i].m_radius= resCam.m_radius- i * dir[2];
			cams[i].m_tx    = resCam.m_tx    - i * dir[3];
			cams[i].m_ty    = resCam.m_ty    - i * dir[4];
			cams[i].m_tz    = resCam.m_tz    - i * dir[5];
		}
		
		//get REND_NUM depth images
		CTexMapperView::getInst()->offscreenRend_genImages(cams, depthBuff_full, depths);

#pragma omp parallel for
		for (int t = 0; t < 12; ++t)
		{
			diff[t] = t_Diff(CAMERA_RES_W, CAMERA_RES_H, trgtImg_dist, depths[t]);
		}

		//for (int t = 0; t < 12; ++t) fprintf( stderr, "%d %f\n", kkk, diff[t]); 

		CameraParam TmpCam;
		double TmpMinDiff = DBL_MAX;
		for (int t = 0; t < 12; ++t) if (diff[t] < TmpMinDiff)
		{
			TmpMinDiff = diff[t];
			TmpCam     = cams[t];
		}

		if (TmpMinDiff >= resDiff || fabs(TmpMinDiff - resDiff) < 0.000001) break;
		resDiff = TmpMinDiff;
		resCam  = TmpCam;
	}


	for (int k = 0; k < REND_NUM; ++k) delete[] depths[k];
	delete[] depthBuff_full;
}









inline double t_rand11()
{
	return 2 * (rand() / (double)RAND_MAX) -1;
}

/*
inline double t_rand01()
{
	return rand() / (double)RAND_MAX;
}
*/


//----------------------------------------------------------------------------
//HillCrimbing alg
//------------------------------------------------------------------------------
static void t_fineSearch_HillCrimbing
(
	const byte       *trgtImg     , 
	const float      *trgtImg_dist, 
	const CameraParam initCam     ,
	const double      initDiff    ,

	double           &resDiff     ,
	CameraParam      &resCam  
	)
{

	const int N = 1000;
	const int WH = CAMERA_RES_W * CAMERA_RES_H;

	float *depthBuff_full = new float[WH * REND_NUM];
	float *depths[REND_NUM];
	for (int k = 0; k < REND_NUM; ++k) depths[k] = new float[WH];


	double      minDiff[REND_NUM];
	CameraParam minCams[REND_NUM];
	CameraParam tmpCams[REND_NUM];

	for (int i = 0; i < REND_NUM; ++i) 
	{	
		minDiff[i] = initDiff;
		minCams[i] = tmpCams[i] = initCam ;
	}

	for (int kkk = 0; kkk < N; ++kkk)
	{	
		//random modification
		for (int i = 0; i < REND_NUM; ++i)
		{
			tmpCams[i] = tmpCams[i];
			if( kkk%6 == 0) tmpCams[i].m_theta  += 0.01 * t_rand11();
			if( kkk%6 == 1) tmpCams[i].m_phi    += 0.01 * t_rand11();
			if( kkk%6 == 2) tmpCams[i].m_radius += 0.1  * t_rand11();
			if( kkk%6 == 3) tmpCams[i].m_tx     += 0.01 * t_rand11();
			if( kkk%6 == 4) tmpCams[i].m_ty     += 0.01 * t_rand11();
			if( kkk%6 == 5) tmpCams[i].m_tz     += 0.01 * t_rand11();
		}
		CTexMapperView::getInst()->offscreenRend_genImages(tmpCams, depthBuff_full, depths);

#pragma omp parallel for
		for (int t = 0; t < REND_NUM; ++t)
		{
			//Hill clibming algorithm
			double diff = t_Diff(CAMERA_RES_W, CAMERA_RES_H, trgtImg_dist, depths[t]);
			
			if ( diff < minDiff[t] )
			{
				fprintf(stderr, "*");
				minDiff[t] = diff;
				minCams[t] = tmpCams[t];
			}
		}
	}


	resDiff  = DBL_MAX;
	for (int t = 0; t < REND_NUM; ++t) if ( minDiff[t] < resDiff)
	{
		resDiff = minDiff[t];
		resCam  = minCams[t];
	}

	fprintf( stderr, "up(%f %f)", initDiff, resDiff);

	for (int k = 0; k < REND_NUM; ++k) delete[] depths[k];
	delete[] depthBuff_full;
}



TCore::~TCore()
{

}





TCore::TCore()
{
	//EVec2d angle;
	//t_CalcAnglesAtPointOnScreen(105, EVec2d(100, 100), angle);
	//Trace(angle);

	//load sphere for coarse search
	if (!m_sphere.initialize("sphere5.obj"))
	{
		AfxMessageBox("No sphere is found\n"); 
		exit(0);
	}
	m_sphere.Translate( - m_sphere.getGravityCenter() );
	for (int i = 0; i < m_sphere.m_vSize; ++i) m_sphere.m_verts[i].normalize();

	CFileDialog obj_dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, "surface model(.obj) | *.obj");
	if (obj_dlg.DoModal() != IDOK) exit(1);


	m_mesh.initialize(obj_dlg.GetPathName());
	m_mesh.Translate( - m_mesh.getGravityCenter() );



	CFileDialog imginfo_dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, "config file(.txt) | *.txt");
	if (imginfo_dlg.DoModal() != IDOK) exit(1);

	string path(imginfo_dlg.GetPathName());
	string filename(imginfo_dlg.GetFileName());
	m_dirPath = path.substr(0, path.length() - filename.length());

	
	char buf1[256], buf2[256];
	int  fX,fY,fZ;

	FILE *fp = fopen(path.c_str(), "r");

	fscanf(fp, "%lf %lf %s", &m_focalLen, &m_camRadi0, buf1);

	while ( fscanf(fp, "%s%s%d%d%d", buf1, buf2, &fX, &fY, &fZ) == 5)
	{
		m_imgs.push_back( TargetImageInfo() );
		m_imgs.back().m_imgFname    = string(buf1);
		m_imgs.back().m_imgBinFname = string(buf2);
		m_imgs.back().m_fX          = fX;
		m_imgs.back().m_fY          = fY;
		m_imgs.back().m_fZ          = fZ;
	}

	fprintf(stderr, "%s %s\n", path.c_str(), filename.c_str());
}









static void t_drawTrianglesWithNormal(const TTexMesh &m)
{
	glBegin(GL_TRIANGLES);
	const EVec3d *verts = m.m_verts  ;
	const EVec3d *norms = m.m_v_norms;
	for (int i = 0; i < m.m_pSize; ++i)
	{
		const TPoly &p = m.m_polys[i];
		glNormal3dv(norms[p.vIdx[0]].data()); glVertex3dv( verts[p.vIdx[0]].data());
		glNormal3dv(norms[p.vIdx[1]].data()); glVertex3dv( verts[p.vIdx[1]].data());
		glNormal3dv(norms[p.vIdx[2]].data()); glVertex3dv( verts[p.vIdx[2]].data());
	}
	glEnd();
}


struct VERTEX_FORMAT
{
	float nx, ny, nz, x, y, z;
};


//参考 http://asura.iaigiri.com/OpenGL/gl43.html
//床井先生　http://marina.sys.wakayama-u.ac.jp/~tokoi/?date=20151125　配列を用意しない方法もある
void TCore::drawScene()
{
	static GLuint vertexBuffer, indexBuffer;
	static bool isFirst = true;
	if (isFirst)
	{
		isFirst = false;

		VERTEX_FORMAT *Vs = new VERTEX_FORMAT[m_mesh.m_vSize];
		unsigned int  *Ps = new unsigned int [m_mesh.m_pSize * 3];

		for (int i = 0; i < m_mesh.m_vSize; ++i)
		{
			Vs[i].nx = (float)m_mesh.m_v_norms[i][0];
			Vs[i].ny = (float)m_mesh.m_v_norms[i][1];
			Vs[i].nz = (float)m_mesh.m_v_norms[i][2];
			Vs[i].x  = (float)m_mesh.m_verts[i][0]; 
			Vs[i].y  = (float)m_mesh.m_verts[i][1]; 
			Vs[i].z  = (float)m_mesh.m_verts[i][2];
		}
		for (int i = 0; i < m_mesh.m_pSize; ++i)
		{
			Ps[3 * i + 0] = m_mesh.m_polys[i].vIdx[0];
			Ps[3 * i + 1] = m_mesh.m_polys[i].vIdx[1];
			Ps[3 * i + 2] = m_mesh.m_polys[i].vIdx[2];
		}

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, m_mesh.m_vSize * sizeof(VERTEX_FORMAT), Vs, GL_STATIC_DRAW);

		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int) * m_mesh.m_pSize * 3, Ps, GL_STATIC_DRAW);

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		delete[] Vs;
		delete[] Ps;
	}


	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );
	glEnable( GL_LIGHT2 );

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glInterleavedArrays(GL_N3F_V3F, 0, NULL);
	glDrawElements(GL_TRIANGLES, m_mesh.m_pSize * 3, GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	//t_drawTrianglesWithNormal( m_mesh );

}





//added time computation at 2017/11/26 
static double totalTime1 = 0, totalTime2 = 0, totalTime3 = 0;
static int    counter   = 0;

//search best fitting camera point and export it as text file
static void t_optimizeCameraPosition
(
	const double   &focalLen, 
	const TTexMesh &sphere  , //for coarse search
	const byte     *trgtImg ,
	
	vector<RendImageInfo> &rendImgs,
	double         &optimCameraDiff,
	CameraParam    &optimCamera    ,
	vector<double> &sphV_diff 
)
{
	fprintf( stderr, "CamPosSearch");

	//generate distance image 
	float *trgtImg_dist = new float[CAMERA_RES_W * CAMERA_RES_H];
	t_genDistImg(CAMERA_RES_W, CAMERA_RES_H, trgtImg, trgtImg_dist);

	t_ExportGrayBmp( "dist.bmp", CAMERA_RES_W, CAMERA_RES_H,  3.0f, trgtImg_dist, 0);

	//Coarse Search--------------------------------------------------------------
	time_t t0 = clock(); 
	CameraParam initCam;
	optimCameraDiff = t_coarseSearch( sphere, focalLen, trgtImg, trgtImg_dist, rendImgs, sphV_diff, initCam);
	
	//Steepest dscent ------------------------------------------------------------	
	time_t t1 = clock(); 
	double d = DBL_MAX;
	CameraParam c;
	t_finSearch( trgtImg, trgtImg_dist, initCam, d, c);

	//MCMC ------------------------------------------------------------	
	time_t t2 = clock(); 
	t_fineSearch_HillCrimbing(trgtImg, trgtImg_dist, c, d, optimCameraDiff,  optimCamera );
	time_t t3 = clock(); 

	fprintf( stderr, "found: %f\n", optimCameraDiff);

	//added before submission 
	totalTime1 += (t1-t0)/(double)CLOCKS_PER_SEC;
	totalTime2 += (t2-t1)/(double)CLOCKS_PER_SEC;
	totalTime3 += (t3-t2)/(double)CLOCKS_PER_SEC;
	counter ++;
	fprintf(stderr,  "coarse %f, steep:%f, hill:%f\n", totalTime1/counter, totalTime2/counter, totalTime3/counter);

	delete[] trgtImg_dist;	
}








void TCore::cameraCalibration()
{
	byte *binImg = new byte[CAMERA_RES_W * CAMERA_RES_H];

	clock_t tt0 = clock();
	preRendMultiImgsForCoarseSearch(m_sphere, m_camRadi0, m_focalLen, m_rendImgs_coarseSearch);
	clock_t tt1 = clock();
	fprintf( stderr, "precomp time : %f\n", (tt1-tt0) / (double) CLOCKS_PER_SEC);



	FILE *allResFp = fopen( (m_dirPath + string("allRes.txt")).c_str(), "w"); 

	for( int i = 0; i < m_imgs.size(); ++i)
	{	
		clock_t t1 = clock();

		//load target silhouette image
		TImage2D img;
		bool     invertFlg = 0;
		if (!img.AllocateFromFile((m_dirPath + m_imgs[i].m_imgBinFname).c_str(), invertFlg, 0)) continue;
		if (invertFlg) img.FlipInY();
		
		int IMG_SIZE_RATE = img.m_W / CAMERA_RES_W; // 8 for NIKON D7000 or EOS 
		for( int y = 0; y < CAMERA_RES_H; ++y)
		for( int x = 0; x < CAMERA_RES_W; ++x) 
		{
			const int I  = x + y * CAMERA_RES_W;
			const int Ib = IMG_SIZE_RATE * x + IMG_SIZE_RATE * y * img.m_W;
			binImg[I] = img[ Ib * 4 ] ? 255 : 0; 
		}

		//find best position
		double         optDiff;
		CameraParam    optCam(0,0,0, 0,0,0, m_focalLen);
		vector<double> sphDiff;
		t_optimizeCameraPosition( m_focalLen, m_sphere, binImg, m_rendImgs_coarseSearch, optDiff, optCam, sphDiff);


		//export info ------------------------ 
		string fname = m_dirPath + m_imgs[i].m_imgBinFname + "cameraInfo.txt";
		FILE *fp = fopen( fname.c_str(), "w");//可視化用

		fprintf( fp, "best_fitting_camera\n%f %f %f  %f %f %f  %f  %f\n", 
			optCam.m_theta, optCam.m_phi, optCam.m_radius, 
			optCam.m_tx   , optCam.m_ty , optCam.m_tz    , optCam.m_fLen, optDiff);

		for (int sphI = 0; sphI < m_sphere.m_vSize; ++sphI)
		{
			const EVec3d &p = m_sphere .m_verts[sphI];
			fprintf(fp, "%d %f %f %f\n", sphI, atan2(-p[2], p[0]), asin(p[1]), sphDiff[sphI]);
		}
		fclose( fp );

		EVec3d cP, cF, cD;
		optCam.getCamearaPos(cP, cF, cD);
		
		fprintf( allResFp, "%s %f %f %f  %f %f %f  %f %f %f   %f %f\n", 
			m_imgs[i].m_imgBinFname.c_str(),
			optCam.m_theta, optCam.m_phi, optCam.m_radius, 
			optCam.m_tx   , optCam.m_ty , optCam.m_tz    , 
			cP[0],cP[1],cP[2],
			optCam.m_fLen, optDiff);

		clock_t t2 = clock();

		fprintf(stderr, "total time %.10f\n", (t2-t1)/(double) CLOCKS_PER_SEC );

	}
	fclose( allResFp );

	delete[] binImg;
}


