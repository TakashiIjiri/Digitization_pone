
// TexMappingViewerView.cpp : CTexMappingViewerView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "TexMappingViewer.h"
#endif

#include "TexMappingViewerDoc.h"
#include "TexMappingViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTexMappingViewerView

IMPLEMENT_DYNCREATE(CTexMappingViewerView, CView)

BEGIN_MESSAGE_MAP(CTexMappingViewerView, CView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CTexMappingViewerView コンストラクション/デストラクション

CTexMappingViewerView::CTexMappingViewerView()
{

	if (!m_sph.initialize("sphere5.obj") )
	{
		AfxMessageBox("sphere is not found\n");
		exit(0);
	}

	m_activeImgInfoIdx = 0;

	m_isR = m_isM = m_isL = 0;
	m_ogl.SetCam( EVec3d(0, 0, 60), EVec3d(0, 0, 0), EVec3d(0, 1, 0));

	CFileDialog obj_dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, "surface model(.obj) | *.obj");
	if (obj_dlg.DoModal() != IDOK) exit(1);
	m_mesh.initialize(obj_dlg.GetPathName() );
	m_mesh.Translate( - m_mesh.getGravityCenter() );
	m_bRendSphere = 0;
}





CTexMappingViewerView::~CTexMappingViewerView()
{
}

BOOL CTexMappingViewerView::PreCreateWindow(CREATESTRUCT& cs)
{

	return CView::PreCreateWindow(cs);
}

// CTexMappingViewerView 印刷

BOOL CTexMappingViewerView::OnPreparePrinting(CPrintInfo* pInfo){
	return DoPreparePrinting(pInfo);
}
void CTexMappingViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/){}
void CTexMappingViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/){}


// CTexMappingViewerView 診断

#ifdef _DEBUG
void CTexMappingViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CTexMappingViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTexMappingViewerDoc* CTexMappingViewerView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTexMappingViewerDoc)));
	return (CTexMappingViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CTexMappingViewerView メッセージ ハンドラー



// CTexMappingViewerView 描画

void t_drawSphereWithDiff(const TTexMesh &sph, const vector<EVec3d> &sphV_diffAngle);



inline bool isCtrKeyOn  (){ return GetKeyState( VK_CONTROL ) < 0 ; }
inline bool isSpaceKeyOn(){ return GetKeyState( VK_SPACE   ) < 0 ; }
inline bool isShiftKeyOn(){ return GetKeyState( VK_SHIFT   ) < 0 ; }





void CTexMappingViewerView::OnDraw(CDC* /*pDC*/)
{
	CTexMappingViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	
	const double defoultFlen = 75;

	double fovY = ( !m_imgData.empty() ) ? 
		m_imgData[m_activeImgInfoIdx].m_cam.calcFovInY() : 
		( 2 * atan( CAMERA_SENSOR_H / 2 / defoultFlen) ) * 360.0 / (2 * M_PI);




	fprintf(stderr, "a%f\n", fovY);
	m_ogl.SetClearColor(1, 1, 1, 0);
	m_ogl.OnDrawBegin( fovY, 1.0, 9000, false);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	float Lposition[] = { 0, 10000, 0, 0 };
	float Ldiffuse[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_POSITION, Lposition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE , Ldiffuse);

	glLineWidth(3);
	

	// axies -------------------------------------
	if (!isShiftKeyOn())
	{
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
			glColor3d(1, 0, 0); glVertex3d(-100, 0, 0);  glVertex3d(100, 0, 0);
			glColor3d(0, 1, 0); glVertex3d(0,-100, 0);  glVertex3d(0, 100, 0);
			glColor3d(0, 0, 1); glVertex3d(0, 0, -100);  glVertex3d(0, 0, 100);
		glEnd();
	}


	// 3D model ------------------------------------
	t_drawMesh( m_mesh );

	// found Eye Points  
	if (!isShiftKeyOn())
	{
		for( int i=0; i < (int)m_imgData.size(); ++i)
		{
			m_imgData[i].m_cam.drawCameraRect( m_imgData[i].m_img );
		}
	}


	if( m_bRendSphere && m_activeImgInfoIdx < m_imgData.size())
	{
		t_drawSphereWithDiff( m_sph, m_imgData[m_activeImgInfoIdx].m_sphV_diffAngle );
	}


	m_ogl.OnDrawEnd();



	// trace camera info 
	fprintf( stderr, "\n\n ---- camera info --- \n");
	EVec3d p = m_ogl.GetCamPos();
	fprintf( stderr, "cameara distance from Origin :  %f\n", p.norm());

	p.normalize();
	const double theta = atan2(-p[2], p[0]);
	const double phi   = asin(p[1]);
	fprintf( stderr, "theta %f , phi %f\n", theta, phi );
	fprintf( stderr, "position (xyz)");	

}



int CTexMappingViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_ogl.OnCreate(this);

	return 0;
}


void CTexMappingViewerView::OnDestroy()
{
	CView::OnDestroy();

	m_ogl.OnDestroy();
}


BOOL CTexMappingViewerView::OnEraseBkgnd(CDC* pDC)
{
	return true;
	//return CView::OnEraseBkgnd(pDC);
}


void CTexMappingViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_isL = 1;
	m_ogl.ButtonDownForTranslate(point);

	CView::OnLButtonDown(nFlags, point);
}


void CTexMappingViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_isL = 0;
	m_ogl.ButtonUp();
	CView::OnLButtonUp(nFlags, point);
}


void CTexMappingViewerView::OnMButtonDown(UINT nFlags, CPoint point)
{
	m_isM = 1;
	m_ogl.ButtonDownForZoom(point);
	CView::OnMButtonDown(nFlags, point);
}


void CTexMappingViewerView::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_isM = 0;
	m_ogl.ButtonUp();
	CView::OnMButtonUp(nFlags, point);
}


void CTexMappingViewerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_isR = 1;
	m_ogl.ButtonDownForRotate(point);
	CView::OnRButtonDown(nFlags, point);
}


void CTexMappingViewerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_isR = 0;
	m_ogl.ButtonUp();
	CView::OnRButtonUp(nFlags, point);
}


void CTexMappingViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_isL || m_isM || m_isR) {
		m_ogl.MouseMove(point);
		RedrawWindow();
	}
	CView::OnMouseMove(nFlags, point);
}


void CTexMappingViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_ogl.OnSize(cx, cy);
}




class LessString {
public:
	bool operator()(const string& rsLeft, const string& rsRight) const
	{
		if (rsLeft.length() == rsRight.length()) return rsLeft < rsRight;
		else                                    return rsLeft.length() < rsRight.length();
	}
};


static bool t_getOpenFilePaths(const string &filter, vector<string> &fpath, string &fext)
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY, filter.c_str());
	char szFileNameBuffer[20000] = { 0 };      // ファイル名を保存させる為のバッファ
	dlg.m_ofn.lpstrFile = szFileNameBuffer;  // バッファの割り当て
	dlg.m_ofn.nMaxFile = 20000;              // 最大文字数の設定

	if (dlg.DoModal() == IDOK)
	{
		POSITION pos = dlg.GetStartPosition();
		while (pos)
		{
			fpath.push_back((LPCTSTR)dlg.GetNextPathName(pos));

			if (fpath.size() == 1) fext = fpath.back().substr(fpath.back().length() - 3, 3);
			else {
				string ext = fpath.back().substr(fpath.back().length() - 3, 3);//拡張子チェック
				if (strcmp(ext.c_str(), fext.c_str()) != 0) {
					AfxMessageBox("複数のファイルが混ざっています");
					return false;
				}
			}
		}
		sort(fpath.begin(), fpath.end(), LessString());

		return true;
	}
	else return false;
}







static void t_HighlightBoundary(TImage2D &img)
{
	for (int y = 1; y < img.m_H-1; ++y)
	for (int x = 1; x < img.m_W-1; ++x)
	{
		const int I = 4 * (x + y*img.m_W);
		if (img.m_RGBA[I] )
		{
			img.m_RGBA[I]   = 255;
			img.m_RGBA[I+1] = 0;
			img.m_RGBA[I+2] = 0;
		}
	}
}





void CTexMappingViewerView::backBufDraw(CameraParam c, int W, int H, float* depthImg, byte *colImg)
{	
	EVec3d cP, cF, cD;
	c.getCamearaPos(cP, cF, cD);

	const double fovy = c.calcFovInY( );

	double nearD = DBL_MAX;
	double farD  = 0      ;

	for (int i = 0; i < m_mesh.m_vSize; ++i)
	{
		double d  = (cP - m_mesh.m_verts[i]).norm();
		nearD = min( d , nearD);
		farD  = max( d , farD );
	}
	nearD -= 10;
	farD  += 10;
	if ( nearD < 0.1 ) nearD = 0.1;

	fprintf( stderr, "nearD, farD :%f %f\n", nearD, farD );



	m_ogl.MakeOpenGLCurrent();

	GLuint canvasFrameBuffer ;
	GLuint canvasRenderBuffer;
	GLuint canvalDepthBuffer;
	glGenFramebuffers(1, &canvasFrameBuffer);
	glBindFramebuffer( GL_FRAMEBUFFER, canvasFrameBuffer);

	glGenRenderbuffers(1, &canvasRenderBuffer);
	glBindRenderbuffer(       GL_RENDERBUFFER, canvasRenderBuffer);
	glRenderbufferStorage(    GL_RENDERBUFFER, GL_RGBA4, W, H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, canvasRenderBuffer);

	glGenRenderbuffers(1, &canvalDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, canvalDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, W, H);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, canvalDepthBuffer);

	//clear color
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	glDisable(GL_POLYGON_SMOOTH);



	// projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, W  / (double)H, nearD, farD);
	glViewport( 0,0, W, H);

	//model view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cP[0], cP[1], cP[2], cF[0], cF[1], cF[2],  cD[0], cD[1], cD[2]);

	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);


	t_drawMesh( m_mesh );


	//get depth frame buffer 
	glReadPixels(0, 0, W, H, GL_DEPTH_COMPONENT, GL_FLOAT        , depthImg);
	glReadPixels(0, 0, W, H, GL_RGBA           , GL_UNSIGNED_BYTE, colImg  );


	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteFramebuffers(1, &canvasFrameBuffer);
	glDeleteFramebuffers(1, &canvasRenderBuffer);
	glDeleteFramebuffers(1, &canvalDepthBuffer );

	glDrawBuffer(GL_BACK);
	glFinish();
}





static double rand_01(){ return  rand() / (double) RAND_MAX ;}
static double rand_11(){ return  2 * (rand_01() - 0.5) ;}


void CTexMappingViewerView::export_rendImg_fromRandomCamera()
{
	FILE *fp = fopen( "groundTruth.txt", "w");
	fprintf( fp, "fname theta phi radi dx dy dz focalLen \n");

	for (int counter = 0; counter < 100; ++counter)
	{
		CString fname;
		fname.Format( "%d.bmp", counter);
		
		double RAND_COEF_PHI   = 0.7 ;//極を避ける
		double RAND_COEF_R     = 10.0; 
		double RAND_COEF_delta = 0.003; 

		// trace camera info 
		const double R = 550 + rand_11() * RAND_COEF_R;
		
		//theta [0,2pi]  phi [-pi/2,pi/2]
		const double theta =   2 * M_PI * rand_01();
		const double phi   = 0.5 * M_PI * rand_11() * RAND_COEF_PHI;
		const double tx    =   2 * M_PI * rand_01();
		const double ty    =   2 * M_PI * rand_11() * RAND_COEF_delta;
		const double tz    =   2 * M_PI * rand_11() * RAND_COEF_delta;
		const double fLen  = 105;
		CameraParam c(theta, phi, R, tx, ty, tz, fLen);

		EVec3d cP, cF, cD;
		c.getCamearaPos(cP, cF, cD);


		float *depthI = new float[  CAMERA_RES_W*CAMERA_RES_H];
		byte  *colI   = new byte [4*CAMERA_RES_W*CAMERA_RES_H];
		backBufDraw(c, CAMERA_RES_W, CAMERA_RES_H, depthI, colI);

		for (int i = 0; i < CAMERA_RES_W*CAMERA_RES_H; ++i)
		{
			if( colI[4*i] == 0 ) colI[4*i] = colI[4*i+1] = colI[4*i+2] = colI[4*i+3] = 0  ;
			else                 colI[4*i] = colI[4*i+1] = colI[4*i+2] = colI[4*i+3] = 255;
		}

		t_exportBmp( fname, CAMERA_RES_W, CAMERA_RES_H, colI, 1);

		delete[] depthI;
		delete[] colI;


		fprintf( fp, "%s camPos  %f %f %f  %f %f %f   %f %f %f  %f\n", 
			fname.GetString(),
			c.m_theta, c.m_phi, c.m_radius, c.m_tx, c.m_ty, c.m_tz, 
			cP[0], cP[1], cP[2],
			c.m_fLen);
	}
	
	fclose( fp );
}




void CTexMappingViewerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'L')
	{
		string filter("cam info (.txt) | *.txt"), fext("txt");
		vector<string> fpaths;
		t_getOpenFilePaths(filter, fpaths, fext);

		for (int i = 0; i < (int)fpaths.size(); ++i)
		{
			FILE *fp = fopen(fpaths[i].c_str(), "r");

			m_imgData.push_back(ImgInfo());
			char buf[2000];

			CameraParam &cam = m_imgData.back().m_cam;
			fscanf(fp, "%s", buf);
			fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf %lf",
				&cam.m_theta, &cam.m_phi, &cam.m_radius,
				&cam.m_tx   , &cam.m_ty , &cam.m_tz    , &cam.m_fLen, &m_imgData.back().m_diff);

			int idx;
			double the, phi, rad, diff;
			for (int i = 0; i < m_sph.m_vSize; ++i)
			{
				fscanf(fp, "%d %lf %lf %lf", &idx, &the, &phi, &diff);
				m_imgData.back().m_sphV_diffAngle.push_back(EVec3d(diff, 0, 0));
			}

			fclose(fp);

			string bmpName = fpaths[i].substr(0, fpaths[i].length() - 14);
			bool bInv;
			m_imgData.back().m_img.AllocateFromFile(bmpName.c_str(), bInv, 0);
			if (bInv)m_imgData.back().m_img.FlipInY();

			t_HighlightBoundary(m_imgData.back().m_img);
			fprintf(stderr, "%s %f\n", bmpName.c_str(), m_imgData.back().m_diff);
		}
	}

	if (nChar == 'F')
	{
		if( m_imgData.size() == 0 ) return;
		m_activeImgInfoIdx++;

		if( m_activeImgInfoIdx > m_imgData.size() -1) m_activeImgInfoIdx = 0;

		EVec3d P,F,Y;
		m_imgData[m_activeImgInfoIdx].m_cam.getCamearaPos(P,F,Y);

		EVec3d ray = (F-P).normalized();
		m_ogl.SetCam(P, P + P.norm()* ray,Y);
		m_ogl.RedrawWindow();

		fprintf( stderr, "diff value : %f\n", m_imgData[m_activeImgInfoIdx].m_diff);

	}	

	if (nChar == 'V') m_bRendSphere = !m_bRendSphere;


	if (nChar == 'S')
	{
		export_rendImg_fromRandomCamera();
	}


	if (m_imgData.size() != 0)
	{
		if (nChar == 49) m_imgData.front().m_cam.m_tx += 0.1;
		if (nChar == 50) m_imgData.front().m_cam.m_tx -= 0.1;
		if (nChar == 51) m_imgData.front().m_cam.m_ty += 0.01;
		if (nChar == 52) m_imgData.front().m_cam.m_ty -= 0.01;
		if (nChar == 53) m_imgData.front().m_cam.m_tz += 0.01;
		if (nChar == 54) m_imgData.front().m_cam.m_tz -= 0.01;
	}

		RedrawWindow();

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}









void t_drawSphereWithDiff(const TTexMesh &sph, const vector<EVec3d> &sphV_diffAngle)
{

	//draw sphere 
	double diffMin = INT_MAX;
	double diffMax = 0;
	for (int i = 0; i < (int)sphV_diffAngle.size(); ++i)
	{
		if (sphV_diffAngle[i][0] < 0) continue;
		diffMin = min( diffMin, sphV_diffAngle[i][0] );	
		diffMax = max( diffMax, sphV_diffAngle[i][0] );
	}
	double coef = 1.0 / (diffMax - diffMin);
	fprintf(stderr, "min max %f %f\n", diffMin, diffMax);

	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);


	for (int k = 0; k < (int)sph.m_pSize; ++k)
	{
		const double r = 400;
		
		const TPoly &p = sph.m_polys[k];
		double c1 = (sphV_diffAngle[ p.vIdx[0] ][0]-diffMin) * coef;
		double c2 = (sphV_diffAngle[ p.vIdx[1] ][0]-diffMin) * coef;
		double c3 = (sphV_diffAngle[ p.vIdx[2] ][0]-diffMin) * coef;
		EVec3d v1 = sph.m_verts[p.vIdx[0]] * r;
		EVec3d v2 = sph.m_verts[p.vIdx[1]] * r;
		EVec3d v3 = sph.m_verts[p.vIdx[2]] * r;

		if (sphV_diffAngle[ p.vIdx[0] ][0] == -1 || 
			sphV_diffAngle[ p.vIdx[1] ][0] == -1 || 
			sphV_diffAngle[ p.vIdx[2] ][0] == -1 )
		{
			glColor3d( 0, 0.1, 0);
			glVertex3dv(v1.data());
			glVertex3dv(v2.data());
			glVertex3dv(v3.data());
		}
		else
		{
			glColor3d( c1, 0, 0);  glVertex3dv(v1.data());
			glColor3d( c2, 0, 0);  glVertex3dv(v2.data());
			glColor3d( c3, 0, 0);  glVertex3dv(v3.data());
		}
	}
	glEnd();
}


