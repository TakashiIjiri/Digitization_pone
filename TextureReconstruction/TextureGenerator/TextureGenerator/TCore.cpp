#include "stdafx.h"
#include "TCore.h"
#include "TextureGeneratorView.h"
#include "../../Common/CameraInfo.h"
#include "TextureSynthesis.h"
#include "TextureBlender.h"
#include "TextureHoleFill.h"

#include <list>
#include <vector>



const int   OCLUSION_RAD    = 5;
const float OCLUSION_THRESH = 0.05f;




using namespace std;
TCore::~TCore()
{
	for( const auto i: m_texAngle) delete[] i;
}




void t_calcNearFarDist(const int N, const EVec3d *Vs, const EVec3d &P, double &nearD, double &farD)
{
	nearD = DBL_MAX;
	farD  = 0      ;

	for (int i = 0; i < N; ++i)
	{
		double d  = (P - Vs[i]).norm();
		nearD = min( d , nearD);
		farD  = max( d , farD );
	}
}







//check a vertex of a mesh is visible by using id ref image
inline static bool t_checkIdRefImg_existTrgtID
(
	const EVec2i pix,
	const int   idRefW, 
	const int   idRefH,
	const byte *idRefImg,
	const vector<int> &trgtPolyIDs
	)
{
	const int WINDOW_SIZE = 2;

	for (int y = -WINDOW_SIZE; y <= WINDOW_SIZE; ++y)
	for (int x = -WINDOW_SIZE; x <= WINDOW_SIZE; ++x)
	{
		int II = 4 * (x + pix[0] + idRefW * ( y + pix[1]) );
		int ID = (int)RGB(idRefImg[II], idRefImg[II + 1], idRefImg[II + 2]);
		for (const auto pid : trgtPolyIDs) if (ID == pid) return true;
	}
	return false;
}




//set visibility of each triangle 
void t_VisibleTestForTriangles
(
	const TTexMesh &mesh  , //mesh
	const EVec3d   &cPos  , //cam info 
	const EVec3d   &cFoc  , //cam info
	const EVec3d   &cDir  , //cam info
	const double   cFovY  , //cam info
	const double   cAspect, //cam info
	const double   nearD  , //cam info
	const double   farD   , //cam info

	const int    W        , //render image resolution W
	const int    H        , //render image resolution H
	const byte  *idRefImg , //id reference image (x,y) --> poly idx
	const float *imgDepth , //depth image        (x,y) --> rendered depth

	byte  *isPolyVisible  ,
	float *polyDotNormRay
)
{
#pragma omp parallel for
	for (int p = 0; p < mesh.m_pSize; ++p)
	{
		isPolyVisible[p] = false;

		int *vidx = mesh.m_polys[p].vIdx;
		const EVec3d v0 = mesh.m_verts[vidx[0]]; 
		const EVec3d v1 = mesh.m_verts[vidx[1]]; 
		const EVec3d v2 = mesh.m_verts[vidx[2]]; 

		//normal-test
		polyDotNormRay[p] = (float)mesh.m_p_norms[p].dot( (v0 - cPos).normalized() );
		if ( polyDotNormRay[p] > 0 ) continue;


		const EVec3d pV0 = t_Project(cFovY, cAspect, nearD, farD, cPos, cDir, cFoc, v0);
		const EVec3d pV1 = t_Project(cFovY, cAspect, nearD, farD, cPos, cDir, cFoc, v1);
		const EVec3d pV2 = t_Project(cFovY, cAspect, nearD, farD, cPos, cDir, cFoc, v2);
		const EVec2i pP0( (int)((pV0[0]+1)/2.0 * W), (int)((pV0[1]+1)/2.0 * H) );
		const EVec2i pP1( (int)((pV1[0]+1)/2.0 * W), (int)((pV1[1]+1)/2.0 * H) );
		const EVec2i pP2( (int)((pV2[0]+1)/2.0 * W), (int)((pV2[1]+1)/2.0 * H) );

		//occulusion-test (use idRefImage)
		if( !t_checkIdRefImg_existTrgtID( pP0, W, H, idRefImg, mesh.m_v_RingPs[vidx[0]]) ||
			!t_checkIdRefImg_existTrgtID( pP1, W, H, idRefImg, mesh.m_v_RingPs[vidx[1]]) ||
			!t_checkIdRefImg_existTrgtID( pP2, W, H, idRefImg, mesh.m_v_RingPs[vidx[2]]) )
		{
			continue;
		}

		//occulusion-test (use depthImage)
		float d0 = imgDepth[ pP0[0] + W*pP0[1]];
		float d1 = imgDepth[ pP1[0] + W*pP1[1]];
		float d2 = imgDepth[ pP2[0] + W*pP2[1]];
		if (fabs(d0 - d1) > OCLUSION_THRESH ||
			fabs(d1 - d2) > OCLUSION_THRESH ||
			fabs(d2 - d0) > OCLUSION_THRESH )
		{
			continue;
		}

		isPolyVisible[p] = true;
	}

}






static float t_getLowestDepthInWin(
	const int x,
	const int y,
	const int W,
	const int H,
	const int winRad, 
	const float *imgDepth)
{
	float res = imgDepth[x + y * W];

	for( int xx = -winRad; xx <= winRad; ++xx) if( 0 <= x + xx && x + xx < W)
	for (int yy = -winRad; yy <= winRad; ++yy) if (0 <= y + yy && y + yy < H)
	{
		int I = x+ xx + W*(y+yy);
		res = min(res, imgDepth[I]);
	}
	return res;
}



EVec3d t_mapPoint_UVto3D(const TTexMesh &mesh, const int trgtPolyIdx, const EVec2d &uv)
{
	const int    *tidx = mesh.m_polys[trgtPolyIdx].tIdx;
	const EVec3d &p0   = mesh.m_uvs[tidx[0]];
	const EVec3d &p1   = mesh.m_uvs[tidx[1]];
	const EVec3d &p2   = mesh.m_uvs[tidx[2]];

	double s, t;
	t_solve2by2LinearEquation(p1[0] - p0[0], p2[0] - p0[0], 
		                      p1[1] - p0[1], p2[1] - p0[1], 
		                      uv[0] - p0[0], uv[1] - p0[1], s, t);
	
	const int   *vidx = mesh.m_polys[trgtPolyIdx].vIdx;
	const EVec3d &v0 = mesh.m_verts[vidx[0]];
	const EVec3d &v1 = mesh.m_verts[vidx[1]];
	const EVec3d &v2 = mesh.m_verts[vidx[2]];
	return v0 + s * (v1-v0) + t * (v2-v0);
}






//alpha channel : 0:have value, 1:occluded, 2:backgoud
//photo size [3696 x 2448]  * 0.125 = [462x306], * 3/8 = [idRefW x idRefH]

void t_computeTexture
(
	const TTexMesh   &mesh     , // input : 3D mesh model
	const TPhotoInfo &photoInfo, // input : photo (position/binPhoto/RGBAphoto)
	const int        *PolyIdTex, // input : image (u,v) --> poly idx

	TImage2D &texture , //extracted tecture
	float    *texAngle  //camRay.dot( norm )
	)
{
	// image sizes tW/tH (uv texture), iW/iH (photo image)
	const int tW = texture.m_W;
	const int tH = texture.m_H;
	const int iW = photoInfo.m_img.m_W;
	const int iH = photoInfo.m_img.m_H;


	//gen camera info----------------------------------
	EVec3d cPos, cFoc, cDir;
	photoInfo.m_cam.getCamearaPos(cPos, cFoc, cDir, 500);
	const double cFovY   = photoInfo.m_cam.calcFovInY();
	const double cAspect = iW / (double)iH;

	double nearD, farD;
	t_calcNearFarDist( mesh.m_vSize, mesh.m_verts, cPos, nearD, farD);
	nearD -= 10;
	farD  += 10;
	if ( nearD < 0.1 ) nearD = 0.1;


	// depth/idRef image  &  polygon visible test -----------------------------
	float *imgDepth   = new float[ iW * iH    ];
	byte  *imgIdRef   = new byte [ iW * iH * 4];
	byte  *plyVisible = new byte [mesh.m_pSize];
	float *plyDot     = new float[mesh.m_pSize];
	CTextureGeneratorView::getInst()->rendIDrefImage(iW, iH, cPos, cFoc, cDir, cFovY, nearD, farD, imgIdRef, imgDepth);
	t_VisibleTestForTriangles( mesh, cPos, cFoc, cDir, cFovY, cAspect, nearD, farD, iW, iH, imgIdRef, imgDepth, plyVisible, plyDot);
	
	for (int i = 0; i < tH * tW; ++i)
	{
		if ( PolyIdTex[i] < 0 ) texture.setPix(4 * i, BACK_COL[0],BACK_COL[1],BACK_COL[2], BACK_COL[3]);
		else                    texture.setPix(4 * i, FORE_NAN[0],FORE_NAN[1],FORE_NAN[2], FORE_NAN[3]);		
		texAngle[i] = 0;
	}

	
	// texture extraction ---------------------------

#pragma omp parallel for
	for (int v = 0; v < tH; ++v)
	{
		for (int u = 0; u < tW; ++u)
		{
			const int pixUV  = u + v * tW;
			const int polyId = PolyIdTex[pixUV];
			
			if(  polyId < 0 || !plyVisible[polyId]) continue;

			// (u, v) --> pos on surface (pos3D) --> project on photo (projPt) --> (x,y)
			const EVec3d pos3D  = t_mapPoint_UVto3D(mesh, polyId, EVec2d((u+0.5)/tW,(v+0.5)/tH) );
			const EVec3d projPt = t_Project(cFovY, cAspect, nearD, farD, cPos, cDir, cFoc, pos3D);
			const int x = (int)((projPt[0] + 1) * iW / 2.0);
			const int y = (int)((projPt[1] + 1) * iH / 2.0);
			const int pixXY = x + y * iW;

			//check pixel of photo at (x,y)
			if (x < 0 || x > iW - 1 || y < 0 || y > iH - 1 ) continue;
			if (!photoInfo.m_imgBin[4 * pixXY]) continue;

			//近傍にdepth値の小さな物体がある場合は遮蔽の可能性があるため、この画像の画素値を利用しない
			float lowDep = t_getLowestDepthInWin(x,y, iW,iH,OCLUSION_RAD,imgDepth);
			if(  imgDepth[pixXY] - lowDep > OCLUSION_THRESH) continue;

			//set pixel color & ray.dot(norm)
			texture.setPix(4*pixUV, photoInfo.m_img[4*pixXY + 0], photoInfo.m_img[4*pixXY + 1], photoInfo.m_img[4*pixXY + 2], 0);
			texAngle[ pixUV ] = (float) max(0, - plyDot[polyId] );
		}
	}

	if ( true )
	{
		TImage2D img;
		img.AllocateImage(iW, iH, 0);
		memcpy(img.m_RGBA, imgIdRef, sizeof(byte) * iW * iH * 4);
		img.saveAsFile("idRefImg.bmp", 0);
		t_ExportGrayBmp("depthImg.bmp", iW, iH, 255.0f, imgDepth,1 );
		//t_ExportGrayBmp("edgeImg.bmp" , iW, iH, imgIsEdge, 1);
	}

	delete[] plyVisible;
	delete[] imgDepth;
	delete[] imgIdRef;
	//delete[] imgIsEdge;
}


































static int *seamMapImg = 0;
static int *polyIdImg  = 0;
static TImage2D img_polyId;

void TCore::computeTexture()
{
	fprintf(stderr, "computeTexture\n");

	const int W = GEN_TEXTURE_SIZE;
	const int H = GEN_TEXTURE_SIZE;

	polyIdImg  = new int[W*H];
	seamMapImg = new int[W*H];
	t_genPolygonIDtexture(W, H, 3,         m_texMesh, polyIdImg );
	t_genSeamMapTexture  (W, H, polyIdImg, m_texMesh, seamMapImg);

	img_polyId = t_genImage2DforIdImg(W,H,polyIdImg);
	img_polyId.FlipInY();
	img_polyId.saveAsFile( "idMap.bmp", 0);



	if (m_photos.size() == 0) return;
	m_textures.clear();
	m_texAngle.clear();

	for (int i = 0; i < m_photos.size(); ++i)
	{
		m_texAngle.push_back( new float[W*H] );
		m_textures.push_back( TImage2D() );
		m_textures.back().AllocateImage(W, H, 0);
		t_computeTexture(m_texMesh, m_photos[i], polyIdImg, m_textures.back(), m_texAngle.back() );
		m_textures.back().saveAsFile("aaaaaa.bmp", 0);

		fprintf( stderr, "%d/%d done\n", i, (int)m_photos.size());
	}


	// blend all textures
	TextureBlender ::BlendTexture( m_textures, m_texAngle, seamMapImg, m_finTex);

	TImage2D resTex;
	TextureHoleFill::fillHoleByDilation(m_finTex, polyIdImg,seamMapImg, m_texMesh, resTex);
	
	CString fname1, fname2;
	fname1.Format("texture_%.5f.bmp", GRAPH_DATATERM_COEF);
	fname2.Format("texturefill_%.5f.bmp", GRAPH_DATATERM_COEF);

	fprintf( stderr, "%s\n",fname1.GetString() );
	fprintf( stderr, "%s\n",fname2.GetString() );
	m_finTex.FlipInY();
	m_finTex.saveAsFile( fname1, 0 );
	m_finTex.FlipInY();

	resTex.FlipInY();
	resTex.saveAsFile( fname2, 0 );
	resTex.FlipInY();

	m_finTex = resTex;

	delete[] polyIdImg;
}










static bool loadPhotoInfo(  const string fname, TPhotoInfo &photoInfo, double DIFF_THRESH)
{
	//test to gen pull req
	FILE *fp = fopen( fname.c_str(), "r");

	char buf[2000];

	double diff;
	CameraParam &cam = photoInfo.m_cam;
	fscanf( fp, "%s", buf);
	fscanf( fp, "%lf %lf %lf %lf %lf %lf %lf %lf", 
		&cam.m_theta, &cam.m_phi, &cam.m_radius, 
		&cam.m_tx   , &cam.m_ty , &cam.m_tz    , &cam.m_fLen, &diff );
	fclose( fp );

	if( diff >= DIFF_THRESH ) return false;
	bool bInv;
	string binImgPath = fname.substr(0, fname.size() - 14);
	//string ImgPath    = fname.substr(0, fname.size() - 23) + ".bmp";
	//string ImgPath    = fname.substr(0, fname.size() - 21) + ".bmp";
	string ImgPath1   = fname.substr(0, fname.size() - 23) + ".JPG";
	string ImgPath2   = fname.substr(0, fname.size() - 23) + ".bmp";

	if( !photoInfo.m_img.AllocateFromFile( ImgPath1.c_str(), bInv, 0 ) ) 
		if( !photoInfo.m_img.AllocateFromFile( ImgPath2.c_str(), bInv, 0 ) ) 
		{
			fprintf( stderr, "ERROR !!!! %s %s", ImgPath1.c_str(), ImgPath2.c_str());
			AfxMessageBox("cant load photo correctly");
			return false;
		}

	if( bInv ) photoInfo.m_img.FlipInY();

	photoInfo.m_imgBin.AllocateFromFile( binImgPath.c_str(), bInv, 0 );
	if( bInv ) photoInfo.m_imgBin.FlipInY();

	return true;
}





class LessString {
public:
    bool operator()(const string& rsLeft, const string& rsRight) const 
	{
		if(rsLeft.length() == rsRight.length()) return rsLeft < rsRight;
		else                                    return rsLeft.length() < rsRight.length();
    }
};



static bool t_getOpenFilePaths(const string &filter, vector<string> &fpath, string &fext)
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY, filter.c_str());
	char szFileNameBuffer[20000] = {0};      // ファイル名を保存させる為のバッファ
	dlg.m_ofn.lpstrFile = szFileNameBuffer;  // バッファの割り当て
	dlg.m_ofn.nMaxFile  = 20000;              // 最大文字数の設定

	if(dlg.DoModal() == IDOK)
	{
		POSITION pos = dlg.GetStartPosition();
		while(pos)
		{
			fpath.push_back( (LPCTSTR) dlg.GetNextPathName(pos) );

			if( fpath.size() == 1 ) fext = fpath.back().substr( fpath.back().length() - 3, 3);
			else {
				string ext = fpath.back().substr( fpath.back().length() - 3, 3);//拡張子チェック
				if( strcmp( ext.c_str(), fext.c_str() ) != 0 ){
					AfxMessageBox( "複数のファイルが混ざっています");
					return false;
				}
			}
		}
		sort(fpath.begin(), fpath.end(), LessString() );

		return true;
	}
	else return false;
}


const double DIFF_THRESHOLD = 0.7;
//const double DIFF_THRESHOLD = 1.0;


TCore::TCore()
{
	/*
	//test
	bool invert;
	TImage2D sample, result;
	sample.AllocateFromFile( "texSample.bmp",invert, 0 );
	if( invert ) sample.FlipInY();
	result.AllocateImage( 600, 600, 0);
	NonparametricTextureSynthesis( sample.m_W, sample.m_H, sample.m_RGBA, result.m_W, result.m_H, result.m_RGBA);

	result.saveAsFile("result.bmp", 0);
	*/


	//1. load uv mesh 
	CFileDialog obj_dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, "surface model(.obj) | *.obj");
	if (obj_dlg.DoModal() != IDOK) exit(1);
	m_texMesh.initialize( obj_dlg.GetPathName() );

	EVec3d gc = m_texMesh.getGravityCenter();
	Trace(gc);
	m_texMesh.Translate( -1 * gc );
	//m_texMesh.Scale( OBJ_SCALLING );

	//2. load photos 
	int deleteCounter = 0;
	vector< string > camConfigPaths;
	string file_ext;
	t_getOpenFilePaths( string("photo/camera config(.txt) | *.txt"), camConfigPaths, file_ext );
	if( camConfigPaths.size() )
	{
		for( int i=0; i < camConfigPaths.size(); ++i)
		{
			m_photos.push_back( TPhotoInfo () );
			if (!loadPhotoInfo(camConfigPaths[i], m_photos.back(), DIFF_THRESHOLD))
			{
				m_photos.pop_back();
				deleteCounter++;
			}
		}

		fprintf( stderr, "-------------------------------------------\n");
		fprintf( stderr, "-------------------------------------------\n");
		fprintf( stderr, "-------------------------------------------\n");
		fprintf( stderr, "%d photos are removed because of bad fitting\n" ,deleteCounter );
		//CString str;
		//str.Format( "%d photos are removed because of bad fitting\n" ,deleteCounter );
		//AfxMessageBox(str);
	}
	else
	{

		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, "bitmap (.bmp) | *.bmp");
		if (dlg.DoModal() != IDOK) exit(1);
		bool tf;
		m_finTex.AllocateFromFile( dlg.GetPathName(), tf, 0 );
		if ( tf ) m_finTex.FlipInY();
	}
}






static void debugVisMapImg(const TTexMesh &m)
{


	const double r = 50;
	img_polyId.Bind(0);

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glColor3d(1,1,1);

	glDisable( GL_CULL_FACE);
	glBegin ( GL_QUADS );
		glTexCoord2d( 0,0 ); glVertex3d( 0,r,0 );
		glTexCoord2d( 1,0 ); glVertex3d( r,r,0 );
		glTexCoord2d( 1,1 ); glVertex3d( r,0,0 );
		glTexCoord2d( 0,1 ); glVertex3d( 0,0,0 );
	glEnd();
	
	glDisable( GL_TEXTURE_2D );

	const int W = GEN_TEXTURE_SIZE;
	const int H = GEN_TEXTURE_SIZE;

	glDisable( GL_LIGHTING );
	glBegin( GL_LINES );
	for( int y= 2000; y < 2300; ++y)
	{
		for (int x = 0; x < 2000; ++x)
		{
			const int I = x + y * W;
			if( seamMapImg[I] < 0) continue;

			int x1 = seamMapImg[I] % W;
			int y1 = seamMapImg[I] / W;

			glColor3d(1,1,1);
			glVertex3d( (x+0.5) /(double)W * r, (y+0.5)  /(double)H * r, 0.01 );
			glVertex3d((x1+0.5) /(double)W * r, (y1+0.5) /(double)H * r, 0.01 );

		}
	}
	glEnd();



	//draw triangles
	const TPoly  *polys = m.m_polys;
	const TEdge  *edges = m.m_edges;
	const EVec3d *uvs   = m.m_uvs  ;

	glLineWidth( 1 );
	glBegin( GL_LINES );

	for (int i = 0; i < m.m_pSize; ++i)
	{
		const int *t = polys[i].tIdx;
		const EVec3d &uv0 = uvs[t[0]];
		const EVec3d &uv1 = uvs[t[1]];
		const EVec3d &uv2 = uvs[t[2]];

		if( edges[polys[i].edge[0]].bAtlsSeam) glColor3d(1,0,0);
		else glColor3d(1,1,1);
		glVertex3d( uv0[0] * r, uv0[1] * r, 0.02);
		glVertex3d( uv1[0] * r, uv1[1] * r, 0.02);
		if( edges[polys[i].edge[1]].bAtlsSeam) glColor3d(1,0,0);
		else glColor3d(1,1,1);
		glVertex3d( uv1[0] * r, uv1[1] * r, 0.02);
		glVertex3d( uv2[0] * r, uv2[1] * r, 0.02);
		if( edges[polys[i].edge[2]].bAtlsSeam) glColor3d(1,0,0);
		else glColor3d(1,1,1);
		glVertex3d( uv2[0] * r, uv2[1] * r, 0.02);
		glVertex3d( uv0[0] * r, uv0[1] * r, 0.02);
	}

	glEnd();




	glEnable( GL_LIGHTING );

}





static GLfloat spec    [] = { 1,1,1,1 } ;
static GLfloat diff    [] = { 0.4f,0.4f,0.4f,1.0f };
static GLfloat amb     [] = { 1.0f,1.0f,1.0f,1.0f };
static GLfloat shin    [] = { 64.0f};

void TCore::drawScene()
{
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR , spec );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE  , diff );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT  , amb  );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, shin);
	m_texMesh.drawWithTex( m_finTex );

	glDisable( GL_LIGHTING );
	glColor3d( 1,0,0);
	glLineWidth(5);
	//m_texMesh.drawEdge   ( 1);
	//if( img_polyId.isAllocated() ) debugVisMapImg(m_texMesh);
}

void TCore::drawPolygonIDs()
{
	glBegin(GL_TRIANGLES);

	const EVec3d *verts = m_texMesh.m_verts;
	const EVec3d *norms = m_texMesh.m_v_norms;
	const EVec3d *uvs   = m_texMesh.m_uvs;

	for (int i = 0; i < m_texMesh.m_pSize; ++i)
	{
		const int *idx   = m_texMesh.m_polys[i].vIdx;
		COLORREF a = i;
		byte r = GetRValue(a);
		byte g = GetGValue(a);
		byte b = GetBValue(a);
		glColor3ub(r, g, b);
		glVertex3dv(verts[idx[0]].data());
		glVertex3dv(verts[idx[1]].data());
		glVertex3dv(verts[idx[2]].data());
	}
	glEnd();

}





