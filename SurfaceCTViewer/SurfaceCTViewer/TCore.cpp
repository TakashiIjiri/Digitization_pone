#include "stdafx.h"
#include "TCore.h"
#include "MainForm.h"

#include "time.h"
#include <map>


using namespace SurfaceCTViewer;







bool t_open_traw3d
( 
	const string fname  ,
	EVec3i  &reso  ,
	EVec3f  &pitch ,
	EVec3f  &cuboid,
	short*  &volume
)
{
	printf( "traw3d\n") ;

	FILE *fp = fopen( fname.c_str(), "rb" );

	int  W, H, D; // resoulution (Width, Height, Depth)
	double px, py, pz;
	fread( &W , sizeof(int   ), 1, fp ); 
	fread( &H , sizeof(int   ), 1, fp ); 
	fread( &D , sizeof(int   ), 1, fp );
	fread( &px, sizeof(double), 1, fp ); 
	fread( &py, sizeof(double), 1, fp ); 
	fread( &pz, sizeof(double), 1, fp );
	reso   <<    W,    H,    D;
	pitch  <<  (float)px, (float)py, (float)pz;
	cuboid <<  (float)(W*px), (float)(H*py), (float)(D*pz);

	volume  = new short[W*H*D];
	fread( volume, sizeof(short), W*H*D, fp );

	fclose(fp);
	return true;
}





static void t_calcBoundBox2D(const vector<EVec2i> &verts, EVec2i &BBmin, EVec2i &BBmax)
{
	BBmin << INT_MAX, INT_MAX;
	BBmax << INT_MIN, INT_MIN;

	for(int i=0; i < (int)verts.size(); ++i)
	{
		BBmin[0] = min(BBmin[0], verts[i][0]);  
		BBmin[1] = min(BBmin[1], verts[i][1]);
		BBmax[0] = max(BBmax[0], verts[i][0]); 
		BBmax[1] = max(BBmax[1], verts[i][1]);
	}
}



static void t_calcBoundBox3D( const int N,  const EVec3d* verts, EVec3f &BBmin, EVec3f &BBmax)
{
	BBmin << FLT_MAX, FLT_MAX, FLT_MAX;
	BBmax <<-FLT_MAX,-FLT_MAX,-FLT_MAX;

	for(int i=0; i < N; ++i)
	{
		BBmin[0] = min(BBmin[0], (float)verts[i][0]);
		BBmin[1] = min(BBmin[1], (float)verts[i][1]);
		BBmin[2] = min(BBmin[2], (float)verts[i][2]);

		BBmax[0] = max(BBmax[0], (float)verts[i][0]);
		BBmax[1] = max(BBmax[1], (float)verts[i][1]);
		BBmax[2] = max(BBmax[2], (float)verts[i][2]);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//Mesh filling ////////////////////////////////////////////////////////////////////////////


inline void calcBoundBox(
	const EVec3f &v0, 
	const EVec3f &v1,
	const EVec3f &v2,
	EVec3f &bbMin,
	EVec3f &bbMax)
{
	bbMin << min3(v0[0], v1[0], v2[0]), 
		     min3(v0[1], v1[1], v2[1]), 
		     min3(v0[2], v1[2], v2[2]);
	bbMax << max3(v0[0], v1[0], v2[0]), 
		     max3(v0[1], v1[1], v2[1]),
		     max3(v0[2], v1[2], v2[2]);
}


// 
// intersection h = v0 + s(v1-v0) + t(v2-v0) = (x,y,z)    // two of x,y,z are known , v0,v1,v2 are known 

inline bool intersectTriangleToRayX(
	const EVec3f &v0, 
	const EVec3f &v1, 
	const EVec3f &v2, 
	const double y, 
	const double z, 
	
	double &x //output 
)
{
	//pre-check
	if( (y<v0[1] && y<v1[1] && y<v2[1]) || (y>v0[1] && y>v1[1] && y>v2[1]) ) return false;
	if( (z<v0[2] && z<v1[2] && z<v2[2]) || (z>v0[2] && z>v1[2] && z>v2[2]) ) return false;
				
	double s,t;
	if( !t_solve2by2LinearEquation( v1[1]-v0[1], v2[1]-v0[1], 
									v1[2]-v0[2], v2[2]-v0[2], y-v0[1], z-v0[2], s, t) ) return false; 
	if (s < 0 || t < 0 || s+t > 1) return false;

	x = (1-s-t)*v0[0] + s*v1[0] + t*v2[0];
	return true;
}
inline bool intersectTriangleToRayY(
	const EVec3f &v0, 
	const EVec3f &v1, 
	const EVec3f &v2, 
	const double x, 
	const double z, 
	
	double &y //output 
)
{
	//pre-check
	if( (x<v0[0] && x<v1[0] && x<v2[0]) || (x>v0[0] && x>v1[0] && x>v2[0]) ) return false;
	if( (z<v0[2] && z<v1[2] && z<v2[2]) || (z>v0[2] && z>v1[2] && z>v2[2]) ) return false;
				
	double s,t;
	if( !t_solve2by2LinearEquation( v1[0]-v0[0], v2[0]-v0[0], 
									v1[2]-v0[2], v2[2]-v0[2], x-v0[0], z-v0[2], s, t) ) return false; 
	if (s < 0 || t < 0 || s+t > 1) return false;

	y = (1-s-t)*v0[1] + s*v1[1] + t*v2[1];
	return true;
}



// cast ray in X axis 
static void genBinaryVolumeInTriangleMeshX
(
	const int W,
	const int H,
	const int D,
	const double px,
	const double py,
	const double pz,

	const int vSize,
	const int pSize,
	const EVec3d *verts,
	const EVec3d *vNorm, 
	const TPoly  *polys,
	const EVec3d *pNorm,

	byte *binVol //allocated[WxHxD], 0:out, 1:in
)
{
	clock_t t0 = clock();
	const int WH = W*H, WHD = W*H*D;
	const EVec3f cuboid( (float)(W*px), (float)(H*py), (float)(D*pz));

	EVec3f BBmin, BBmax; 
	t_calcBoundBox3D( vSize, verts, BBmin, BBmax );

	memset( binVol, 0, sizeof( byte ) * WHD );


	// insert triangles in BINs -- divide yz space into (BIN_SIZE x BIN_SIZE)	
	const int BIN_SIZE = 20;
	vector< vector<int> > polyID_Bins( BIN_SIZE * BIN_SIZE, vector<int>() );

	for( int p=0; p<pSize; ++p)
	{
		EVec3f bbMin, bbMax;
		calcBoundBox( verts[ polys[p].vIdx[0] ].cast<float>(), verts[ polys[p].vIdx[1] ].cast<float>(), verts[ polys[p].vIdx[2] ].cast<float>(), bbMin, bbMax );
		int yS = min( (int) (bbMin[1]/cuboid[1]*BIN_SIZE), BIN_SIZE-1 );
		int zS = min( (int) (bbMin[2]/cuboid[2]*BIN_SIZE), BIN_SIZE-1 );
		int yE = min( (int) (bbMax[1]/cuboid[1]*BIN_SIZE), BIN_SIZE-1 );
		int zE = min( (int) (bbMax[2]/cuboid[2]*BIN_SIZE), BIN_SIZE-1 );
		for( int z = zS; z <= zE; ++z) for( int y = yS; y <= yE; ++y) polyID_Bins[ z*BIN_SIZE + y ].push_back(p);
	}

	clock_t t1 = clock();

	// ray casting along x axis to fill inside the mesh 
#pragma omp parallel for
	for (int zI = 0;  zI < D;  ++zI) if( BBmin[2] <= (0.5 + zI) * pz && (0.5 + zI) * pz <= BBmax[2] )
	for (int yI = 0;  yI < H;  ++yI) if( BBmin[1] <= (0.5 + yI) * py && (0.5 + yI) * py <= BBmax[1] )
	{
		double z = (0.5 + zI) * pz;
		double y = (0.5 + yI) * py;
		int bin_yi = min( (int) (y/cuboid[1]*BIN_SIZE), BIN_SIZE-1 );
		int bin_zi = min( (int) (z/cuboid[2]*BIN_SIZE), BIN_SIZE-1 );
		vector<int> &trgtBin = polyID_Bins[ bin_zi * BIN_SIZE + bin_yi ];

		multimap<double, double> blist;// (xPos, normInXdir);

		for( const auto pi : trgtBin ) if( pNorm[pi][1] != 0 ) 
		{
			const EVec3f &V0 = verts[ polys[ pi ].vIdx[0] ].cast<float>();
			const EVec3f &V1 = verts[ polys[ pi ].vIdx[1] ].cast<float>();
			const EVec3f &V2 = verts[ polys[ pi ].vIdx[2] ].cast<float>();
			double x;
			if( intersectTriangleToRayX( V0, V1, V2, y,z, x) ) blist.insert( make_pair( x, pNorm[pi][0]) ); //(x 座標, normal[0])
		}

		//clean blist (edge上で起こった交差重複を削除)
		while( blist.size() != 0 )
		{
			if( blist.size() == 1 ){ blist.clear(); break;}

			bool found = false;
			auto it0 = blist.begin();
			auto it1 = blist.begin(); it1++;
				
			for(; it1 != blist.end(); ++it0, ++it1) if( it0->second * it1->second > 0)
			{
				blist.erase( it1 );
				found = true;
				break;
			}
			if( !found ) break;
		}				
		
		bool flag = false;
		int xI = 0;
		
		//int pivIdx = xI ;
		for ( auto it = blist.begin(); it != blist.end(); ++it) 
		{
			int pivXi= (int)(it->first / py);
			for( ; xI <= pivXi && xI < W; ++xI) binVol[ xI + yI * W + zI*WH ] = flag; 
			flag = !flag;
		}
		if( flag == true) fprintf( stderr, "error double check here!");
	}

	clock_t t2 = clock();
	printf("compute time : %f %f\n", (t1-t0)/ (double) CLOCKS_PER_SEC, (t2-t1)/ (double) CLOCKS_PER_SEC);
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


// cast ray in Y axis ( divide ZX plane )  
static void genBinaryVolumeInTriangleMeshY
(
	const int W,
	const int H,
	const int D,
	const double px,
	const double py,
	const double pz,

	const int vSize,
	const int pSize,
	const EVec3d *verts,
	const EVec3d *vNorm, 
	const TPoly  *polys,
	const EVec3d *pNorm,

	byte *binVol //allocated[WxHxD], 0:out, 1:in
)
{
	clock_t t0 = clock();
	const int WH = W*H, WHD = W*H*D;
	const EVec3f cuboid( (float)(W*px), (float)(H*py), (float)(D*pz));

	EVec3f BBmin, BBmax; 
	t_calcBoundBox3D( vSize, verts, BBmin, BBmax );

	memset( binVol, 0, sizeof( byte ) * WHD );


	// insert triangles in BINs -- divide yz space into (BIN_SIZE x BIN_SIZE)	
	const int BIN_SIZE = 20;
	vector< vector<int> > polyID_Bins( BIN_SIZE * BIN_SIZE, vector<int>() );

	for( int p=0; p<pSize; ++p)
	{
		EVec3f bbMin, bbMax;
		calcBoundBox( verts[ polys[p].vIdx[0] ].cast<float>(), verts[ polys[p].vIdx[1] ].cast<float>(), verts[ polys[p].vIdx[2] ].cast<float>(), bbMin, bbMax );
		int xS = min( (int) (bbMin[0]/cuboid[0]*BIN_SIZE), BIN_SIZE-1 );
		int zS = min( (int) (bbMin[2]/cuboid[2]*BIN_SIZE), BIN_SIZE-1 );
		int xE = min( (int) (bbMax[0]/cuboid[0]*BIN_SIZE), BIN_SIZE-1 );
		int zE = min( (int) (bbMax[2]/cuboid[2]*BIN_SIZE), BIN_SIZE-1 );
		for( int z = zS; z <= zE; ++z) for( int x = xS; x <= xE; ++x) polyID_Bins[ z*BIN_SIZE + x ].push_back(p);
	}

	clock_t t1 = clock();

	// ray casting along x axis to fill inside the mesh 
#pragma omp parallel for
	for (int zI = 0;  zI < D;  ++zI) if( BBmin[2] <= (0.5 + zI) * pz && (0.5 + zI) * pz <= BBmax[2] )
	for (int xI = 0;  xI < W;  ++xI) if( BBmin[0] <= (0.5 + xI) * px && (0.5 + xI) * px <= BBmax[0] )
	{
		double x = (0.5 + xI) * px;
		double z = (0.5 + zI) * pz;
		int bin_xi = min( (int) (x/cuboid[0]*BIN_SIZE), BIN_SIZE-1 );
		int bin_zi = min( (int) (z/cuboid[2]*BIN_SIZE), BIN_SIZE-1 );
		vector<int> &trgtBin = polyID_Bins[ bin_zi * BIN_SIZE + bin_xi ];

		multimap<double, double> blist;// (xPos, normInXdir);

		for( const auto pi : trgtBin ) if( pNorm[pi][1] != 0 ) 
		{
			const EVec3f &V0 = verts[ polys[ pi ].vIdx[0] ].cast<float>();
			const EVec3f &V1 = verts[ polys[ pi ].vIdx[1] ].cast<float>();
			const EVec3f &V2 = verts[ polys[ pi ].vIdx[2] ].cast<float>();
			double y;
			if( intersectTriangleToRayY(V0,V1,V2, x,z,y) )
				blist.insert( make_pair( y, pNorm[pi][1]) ); //(y 座標, normal)
		}

		if( blist.size() == 0 ) continue;

		//clean blist (edge上で起こった交差重複を削除)
		while( blist.size() != 0 )
		{
			if( blist.size() == 1 ){ blist.clear(); break;}

			bool found = false;
			auto it0 = blist.begin();
			auto it1 = blist.begin(); it1++;
				
			for(; it1 != blist.end(); ++it0, ++it1) if( it0->second * it1->second > 0)
			{
				blist.erase( it1 );
				found = true;
				break;
			}
			if( !found ) break;
		}				

		bool flag = false;
		int yI = 0;
		
		//int pivIdx = xI ;
		for ( auto it = blist.begin(); it != blist.end(); ++it) 
		{
			int pivYi= (int)(it->first / py);
			for( ; yI <= pivYi && yI < H; ++yI) binVol[ xI + yI * W + zI*WH ] = flag; 
			flag = !flag;
		}
		if( flag == true) fprintf( stderr, "error double check here!");
	}

	clock_t t2 = clock();
	printf("compute time : %f %f\n", (t1-t0)/ (double) CLOCKS_PER_SEC, (t2-t1)/ (double) CLOCKS_PER_SEC);
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////





TCore::~TCore()
{
	delete[] m_volume;
	m_volume = 0;
}



void TCore::loadModels()
{

	//load volume / surfacemodel / texture
	std::string fname;

	if( !myFileDialog("3d traw files(*.traw3D_ss)|*.traw3D_ss", fname) ) exit(0);
	t_open_traw3d( fname, m_reso, m_pitch, m_cuboid, m_volume);

	if( !myFileDialog("3d surface model (*.obj)|*.obj", fname) ) exit(0);
	m_surface.initialize(fname.c_str());

	if( !myFileDialog("texture (*.jpg;*.bmp)|*.jpg;*.bmp", fname) ) exit(0);
	m_texture.Allocate(fname.c_str());
	m_texture.FlipInY();


	//flip stac direction (only for this data )
	fprintf( stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf( stderr, "!!!FLIP VOLUME IN STACK DIRECTION !!!!!!!!!!!!!!!!!!!!\n");
	const int W = m_reso[0];
	const int H = m_reso[1];
	const int D = m_reso[2], WH = W*H, WHD = W*H*D;
	short *tmp = new short[WH];
	for( int z = 0; z < D/2; ++z)
	{
		memcpy( tmp                    , &m_volume[z       * WH], sizeof( short )*WH);
		memcpy( &m_volume[z * WH]      , &m_volume[(D-1-z) * WH], sizeof( short )*WH);
		memcpy( &m_volume[(D-1-z) * WH], tmp                    , sizeof( short )*WH);
	}
	delete[] tmp;


	//allocate volume for ogl
	m_volumeOgl.Allocate( m_reso );
	short minV = SHRT_MAX, maxV = SHRT_MIN;
	for (int i = 0; i < WHD; ++i){
		minV = min(minV, m_volume[i]); 
		maxV = max(maxV, m_volume[i]);
	}

	for (int i = 0; i < WHD; ++i) 
	{
		//可視化用伝達関数
		maxV = 2000;
		minV = -100;

		double v = (m_volume[i] - minV) / (double)(maxV - minV) * 255;
		m_volumeOgl[i] = (byte) max( 0, min(v,255) ) ;
	}
	//allocate flag volume (0:モデル外, 1:モデル内:cut strokeで削除, 2:モデル内&非削除)
	m_volumeFlg.Allocate( m_reso );
	
	genBinaryVolumeInTriangleMeshY( 
		m_reso[0], m_reso[1], m_reso[2], 
		m_pitch[0], m_pitch[1], m_pitch[2], 
		m_surface.m_vSize, m_surface.m_pSize, m_surface.m_verts, m_surface.m_v_norms, m_surface.m_polys, m_surface.m_p_norms, m_volumeFlg.getVol());


	for (int i = 0; i < WHD; ++i) m_volumeFlg[i] = (m_volumeFlg[i]==0) ? 1 : 255;
	t_morpho3D_dilate(m_volumeFlg);
	t_morpho3D_dilate(m_volumeFlg);
	for (int i = 0; i < WHD; ++i) m_volumeFlg[i] = ( m_volumeFlg [i] == 255 ) ? 2 : 0;
}





TCore::TCore() : 
	m_SurfaceShader("shader\\surfVtx.glsl"  , "shader\\surfFlg.glsl"  ), 
	m_CrsSecShader ("shader\\crssecVtx.glsl", "shader\\crssecFlg.glsl"),
	m_SurfaceShader_trans("shader\\surfVtx.glsl", "shader\\surfFlg_trans.glsl")

{
	m_bL = m_bR = m_bM = false;
	m_bDrawStr  = false;
	vector<EVec3d> m_CutStroke;
}




void TCore::BtnDownL (EVec2i p, OglForCLI* ogl)
{
	m_bL = true;

	if( isCtrKeyOn() )
	{
		m_stroke2D.clear();
		m_stroke3D.clear();
		m_bDrawStr = true;
	}
	else 
		ogl->BtnDown_Trans(p);
}
void TCore::BtnDownR (EVec2i p, OglForCLI* ogl)
{
	m_bR = true;
	ogl->BtnDown_Rot(p);
}
void TCore::BtnDownM (EVec2i p, OglForCLI* ogl)
{
	m_bM = true;
	ogl->BtnDown_Zoom(p);
}

void TCore::BtnUpL   (EVec2i p, OglForCLI* ogl)
{
	if( m_bDrawStr )
	{
		//stroke reduction 
		const int STEP = 3;
		const int minN = 5;
		if( m_stroke2D.size() * STEP > minN )
		{
			vector<EVec2i> tmp2D;
			vector<EVec3f> tmp3D;
			for( int i=0; i < (int)m_stroke2D.size(); i+= STEP)
			{
				tmp2D.push_back( m_stroke2D[i] );
				tmp3D.push_back( m_stroke3D[i] );
			}
			m_stroke2D = tmp2D;
			m_stroke3D = tmp3D;
		}

		updateCutSurface(ogl->GetCamPos());
		updateFlgVolByCutStr(ogl);
		m_stroke2D.clear();
		m_stroke3D.clear();
	}

	m_bL = m_bDrawStr = false;
	ogl->BtnUp();
	MainForm_redrawMainPanel();

}
void TCore::BtnUpR   (EVec2i p, OglForCLI* ogl)
{
	m_bR = false;
	ogl->BtnUp();
	MainForm_redrawMainPanel();
}
void TCore::BtnUpM   (EVec2i p, OglForCLI* ogl)
{
	m_bM = false;
	ogl->BtnUp();
	MainForm_redrawMainPanel();
}




void TCore::MouseMove(EVec2i p, OglForCLI* ogl)
{
	if( !m_bL && !m_bR && !m_bM ) return;

	if( m_bDrawStr )
	{
		EVec3f rayP, rayD, pos;
		ogl->GetCursorRay( p, rayP, rayD);
		m_stroke2D.push_back( p );
		m_stroke3D.push_back( rayP + 0.1f * rayD );
	}
	else
		ogl->MouseMove(p);
	MainForm_redrawMainPanel();
}








void t_drawFrame(const EVec3f &cuboid)
{
	float x = cuboid[0];
	float y = cuboid[1];
	float z = cuboid[2];

	float vtx[8][3] =
	{
		{0,0,0},{x,0,0}, {x,y,0}, {0,y,0},
		{0,0,z},{x,0,z}, {x,y,z}, {0,y,z}
	};

	static const int idx[12][2]=
	{
		{ 0,1 },{ 1,2 },{ 2,3 },{ 3,0 },
		{ 0,4 },{ 1,5 },{ 2,6 },{ 3,7 },
		{ 4,5 },{ 5,6 },{ 6,7 },{ 7,4 },
	};

	glDisable(GL_LIGHTING);
	glColor3d(0, 0, 0.5);
	glLineWidth(4);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vtx);
	glDrawElements(GL_LINES, 12 * 2, GL_UNSIGNED_INT, idx);
	glDisableClientState(GL_VERTEX_ARRAY);
}





void TCore::drawScene(EVec3d camP)
{


	static bool isFirst = true;
	if( isFirst )
	{
		isFirst = false;
		loadModels();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	glActiveTextureARB(GL_TEXTURE1);
	m_volumeOgl.bindOgl(true);
	glActiveTextureARB(GL_TEXTURE2);
	m_volumeFlg.bindOgl(false);
	glActiveTextureARB(GL_TEXTURE3);
	m_texture  .bindOgl(true);
	
	if( !isShiftKeyOn() ) t_drawFrame(m_cuboid);
	
	//draw crssec 
	glDisable(GL_CULL_FACE);
	m_CrsSecShader.bind(1,2,3, m_reso, camP, m_cuboid.cast<double>() );
	m_CutSurface.draw();
	m_CrsSecShader.unbind();
	glEnable(GL_CULL_FACE);


	//draw surface
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_SurfaceShader.bind(1,2,3, m_reso, camP, m_cuboid.cast<double>() );
	m_surface.draw();
	m_SurfaceShader.unbind();

	if( !isSpaceKeyOn() )
	{
		glDepthMask(false);
		glEnable( GL_BLEND );
		m_SurfaceShader_trans.bind(1,2,3, m_reso, camP, m_cuboid.cast<double>() );
		m_surface.draw();
		m_SurfaceShader_trans.unbind();
		glDisable( GL_BLEND );
		glDepthMask(true);
	}

	//draw Cut stroke
	if (m_bDrawStr )
	{
		glColor3d(1,0,0);
		glLineWidth(3);
		glBegin(GL_LINE_STRIP);
			for (const auto &p : m_stroke3D) glVertex3fv( p.data() );
		glEnd();
	}

}








/*-------------------------------------------------------------------------------------------------
bool t_intersectRayToCuboid
const EVec3f &rayP　: input ray point
const EVec3f &rayD  : input ray direction
EVec3f &nearP : output intersection point (close to rayP)
EVec3f &farP  : output intersection point (far from rayP)
--------------------------------------------------------------------------------------------------*/
bool t_intersectRayToCuboid(const EVec3f &cube,
							const EVec3f &rayP,
							const EVec3f &rayD,
							EVec3f &nearP,
							EVec3f &farP)
{
	bool hasNear = false;
	bool hasFar = false;

	for (int xyz = 0; xyz < 3; ++xyz) 
	{
		if (rayD[xyz] == 0 ) continue;

		float t1 = (cube[xyz] - rayP[xyz]) / rayD[xyz];
		float t2 = (0         - rayP[xyz]) / rayD[xyz];
		EVec3f tmpP1 = rayP + t1 * rayD;
		EVec3f tmpP2 = rayP + t2 * rayD;

		if ( t_bInWindow3D(EVec3f(0,0,0), cube, tmpP1, 0.001f) )
		{
			if ( rayD[xyz] < 0 )
			{ 
				nearP = tmpP1; 
				hasNear = true; 
			}
			else 
			{ 
				farP = tmpP1; 
				hasFar = true;
			}
		}

		if (t_bInWindow3D( EVec3f(0,0,0), cube, tmpP2, 0.001f))
		{
			if (rayD[xyz] > 0) 
			{ 
				nearP = tmpP2; 
				hasNear = true;
			}
			else 
			{ 
				farP = tmpP2;
				hasFar = true;
			}
		}
	}

	return hasNear && hasFar;
}






//update surface and volume by CutStroke
void TCore::updateCutSurface(const EVec3f &camP)
{
	m_CutSurface.clear();

	if( m_stroke3D.size() <= 5 ) return;

	// smoothing and resampling stroke
	vector<EVec3f> tmp = m_stroke3D, stroke3D;
	t_verts_Smoothing( 2, tmp );

	int strN = min(100, max(30, (int)tmp.size()));
	t_verts_ResampleEqualInterval( strN, tmp, stroke3D );
	

	//gen surface
	vector< EVec3d > Vs ;
	vector< TPoly  > Ps ;
	
	if( t_bInWindow3D( EVec3f(0,0,0), m_cuboid, camP) )
	{
		Vs.push_back( camP.cast<double>() );

		bool isTherePreVerts = false;

		for( const auto &p : stroke3D )
		{
			EVec3f p0, p1, dir = (p - camP).normalized();

			if( t_intersectRayToCuboid( m_cuboid, camP, dir, p0, p1 ) )
			{
				Vs.push_back( p1.cast<double>() );
				if( isTherePreVerts ) 
				{
					const int piv = (int)Vs.size() - 1;
					Ps.push_back( TPoly( piv, 0, piv-1,  piv, 0, piv-1) );
				}
				isTherePreVerts = true;
			}
			else
				isTherePreVerts = false;
		}
		//if (!Ps.empty())
		//{
		//	m_curveCrsSecEdge.push_back( EVec2i(0, 1) );
		//	m_curveCrsSecEdge.push_back( EVec2i(0, (int)Vs.size()-1) );
		//}

	}
	else
	{
		bool isTherePreVerts = false;
		for (const auto &p : stroke3D)
		{
			EVec3f p0, p1, dir = (p - camP).normalized();

			if(t_intersectRayToCuboid(m_cuboid, camP, dir, p0, p1) )
			{
				Vs.push_back( p0.cast<double>() );
				Vs.push_back( p1.cast<double>() );
				
				if( isTherePreVerts )
				{
					int piv = (int)Vs.size() - 1;
					Ps.push_back( TPoly( piv  , piv-1, piv-2,  piv  , piv-1, piv-2) );
					Ps.push_back( TPoly( piv-1, piv-3, piv-2,  piv-1, piv-3, piv-2) );
					//m_curveCrsSecEdge.push_back( EVec2i(piv  , piv-2) );
					//m_curveCrsSecEdge.push_back( EVec2i(piv-1, piv-3) );
				}
				isTherePreVerts = true;
			}
			else
				isTherePreVerts = false;
		}
	}

	vector< EVec3d > UVs = Vs;
	for( auto &it : UVs )
	{
		UVs[0] /= m_cuboid[0];
		UVs[1] /= m_cuboid[1];
		UVs[2] /= m_cuboid[2];
	}

	m_CutSurface.initialize(Vs, UVs, Ps);

}








static double calcAngle(const EVec2d &d1, const EVec2d &d2)
{
	double l = d1.norm() * d2.norm();
	if (l == 0) return 0;

	double cosT = t_crop(-1.0, 1.0, (d1.dot(d2)) / l);

	if (d1[0] * d2[1] - d1[1] * d2[0] >= 0) return  acos(cosT);
	else					                return -acos(cosT);
}


static bool isInClosedStroke(double x, double y, const vector<EVec2i> &stroke)
{
	//stroke.size() > 3 
	EVec2d d1, d2;
	double sum = 0;

	d1 << stroke.back().x() - x, stroke.back().y() - y;
	d2 << stroke[0].x() - x, stroke[0].y() - y;
	sum = calcAngle(d1, d2);

	for (int i = 1; i < (int)stroke.size(); ++i)
	{
		d1 << stroke[i - 1].x() - x, stroke[i - 1].y() - y;
		d2 << stroke[i].x() - x, stroke[i].y() - y;
		sum += calcAngle(d1, d2);
	}

	return fabs(sum) > M_PI * 1.5;
}






void TCore::updateFlgVolByCutStr(OglForCLI* ogl)
{
	fprintf(stderr, "----updateFlgVolByCutStr %d----\n", (int)m_stroke2D.size());

	const int WH  = m_reso[0] * m_reso[1];
	const int WHD = m_reso[0] * m_reso[1] * m_reso[2];

	m_volumeFlg.setUpdated();
	for( int i=0; i < WHD ; ++i) if( m_volumeFlg[i] >= 1) m_volumeFlg[i] = 2;
	if(m_stroke2D.size() < 5) return;


	//projection matrix 
	double model[16], proj[16];
	int vp[4];
	MainForm_GetProjModelViewMat( model, proj, vp );


	// add a point to 2D stroke
	EVec2i tmpDir = m_stroke2D.front() - m_stroke2D.back();
	EVec2i dir(-tmpDir[1], tmpDir[0]); //rot 90 deg
	m_stroke2D.push_back( (m_stroke2D.front() + m_stroke2D.back()) / 2 + 10 * dir );
	for( auto &p : m_stroke2D) p[1] = vp[3] - p[1];

	EVec2i strBBmin, strBBmax;
	t_calcBoundBox2D(m_stroke2D, strBBmin, strBBmax);

	//2d image (0:yet, 1:in, 2:out)
	byte *binImg2D = new byte[ vp[2] * vp[3] ];
	memset( binImg2D, 0, sizeof(byte) * vp[2] * vp[3] );

	if( !ogl->isDrawing() ) ogl->oglMakeCurrent();
	fprintf( stderr, "project all pixels\n");
	fprintf( stderr, "%d\n", (int)m_stroke2D.size());

	int c = 0;
	for( int I = 0; I < WHD; ++I )if( m_volumeFlg[I] != 0 )
	{
		const int z = I / WH;
		const int y = (I % WH)/m_reso[0];
		const int x = (I % WH)%m_reso[0];

		double px, py, pz;
		gluProject(	(x+0.5)*m_pitch[0],  (y+0.5)*m_pitch[1],  (z+0.5)*m_pitch[2],  model, proj, vp, &px,&py,&pz);

		if( px <= strBBmin[0] ||  py <= strBBmin[1] || px > strBBmax[0] || py > strBBmax[1]) continue;
		if( px < 0 || vp[2]-1 < px || py < 0 || vp[3]-1 < py) continue;

		const int idxOnVp = (int)px + (int)py * vp[2];
		m_volumeFlg[I] =  ( binImg2D[idxOnVp] == 1) ? 1 : 
						  ( binImg2D[idxOnVp] == 2) ? 2 : 
						  ( isInClosedStroke(px,py, m_stroke2D ) ) ? 1 : 2;

		binImg2D[idxOnVp] = (m_volumeFlg[I] == 1) ? 1: 2;

		if( (++c) %10000 == 0 ) fprintf( stderr, "-");
	}


	delete[] binImg2D;
	fprintf( stderr, "DONE\n");

	if( !ogl->isDrawing() )wglMakeCurrent(NULL, NULL);
}





