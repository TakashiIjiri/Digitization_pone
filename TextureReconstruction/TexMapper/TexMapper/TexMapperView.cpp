
// TexMapperView.cpp : CTexMapperView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "TexMapper.h"
#endif

#include "TexMapperDoc.h"
#include "TexMapperView.h"

#include "TCore.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
// TODO 
// 1. レンダリング
	// 視点のモデル化                          OK
	// 6パラメータで視点の位置を指定する関数   OK 
	// 図を作成 --> 完璧に説明できるように   
	// depth buffer 取得                       OK
	// view portサイズをセンササイズに合わせる OK (15.6mm x 23.6mm) 1.51 
	// 画角をカメラに合わせる                  OK (70mm) (resolution 3696x2448 = 1.51) (462x306)

// 2. 画像読み込み
	// 二値か                                  OK
	// 画像の読み込み   

// 3. 比較して視点推定
	// レンダリング画像を保存する
      // 視点モデルの空間(6自由度)をサンプリングし，視点情報を取得する
      // サンプリング間隔:θ[0,360]を360分割，φ[9-0,90]を180分割，Rad[500,1500]を10分割，θx[0,360]360分割，θy[-5,5]を10分割，θz[-5,5]を10分割で行う
	// レンダリング画像と読み込んだ画像の比較
　　　//  dist1(I1,I2) : histogram利用
      //  dist2(I1,I2) : 全画素同士の比較
*/


// CTexMapperView

IMPLEMENT_DYNCREATE(CTexMapperView, CView)

BEGIN_MESSAGE_MAP(CTexMapperView, CView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT         , &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT  , &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW , &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CTexMapperView コンストラクション/デストラクション

// 3D volume  (.traw3D_ss)
// wave front (.obj      )



CTexMapperView* CTexMapperView::m_p = 0;

CTexMapperView::CTexMapperView()
{
	m_p = this;

	m_isR = m_isM = m_isL = 0;

	m_ogl.SetCam(EVec3d(0, 0, 50), EVec3d(0, 0, 0), EVec3d(0, 1, 0));
}

CTexMapperView::~CTexMapperView()
{

}

BOOL CTexMapperView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView::PreCreateWindow(cs)) return false;
	cs.x  = 0;        // x 座標 表示位置
	cs.y  = 0;        // y 座標 表示位置
	cs.cx = CAMERA_RES_W * REND_X_NUM; // 幅
	cs.cy = CAMERA_RES_H * REND_Y_NUM; // 高さ
	return TRUE;
}



	/*	
	for( int k=0; k < REND_NUM; ++k){
		m_depth[k] = new float[CAMERA_RES_W * CAMERA_RES_H];
		for( int i=0; i < CAMERA_RES_W * CAMERA_RES_H; ++i) m_depth[k][i] = 1.0;
	}

	m_depthBuffer = new float [CAMERA_RES_W * CAMERA_RES_H * REND_NUM];
	for( int i=0; i < CAMERA_RES_W * CAMERA_RES_H * REND_NUM; ++i) m_depthBuffer[i] = 1.0;



	for (int k = 0; k < REND_NUM; ++k) delete[] m_depth[k];
	delete[] m_depthBuffer;
	*/

	/*
	t_ExportGrayBmp("01.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[0],1 );
	t_ExportGrayBmp("02.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[1],1 );
	t_ExportGrayBmp("03.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[2],1 );
	t_ExportGrayBmp("04.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[3],1 );
	t_ExportGrayBmp("05.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[4],1 );
	t_ExportGrayBmp("06.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[5],1 );
	t_ExportGrayBmp("07.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[6],1 );
	t_ExportGrayBmp("08.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[7],1 );
	t_ExportGrayBmp("09.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[8],1 );
	t_ExportGrayBmp("10.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[9],1 );
	t_ExportGrayBmp("11.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[10],1 );
	t_ExportGrayBmp("12.bmp", CAMERA_RES_W, CAMERA_RES_H, 255.0f, m_depth[11],1 );
	*/




// depthImg_full and depthImg_subs should be allocated
void CTexMapperView::offscreenRend_genImages
(
	CameraParam cameras[REND_NUM] ,
	float* depthImg_full          , //single   depth buffer image  [REND_X_NUM*CAMERA_RES_W X REND_Y_NUM*CAMERA_RES_H ] 
	float* depthImg_subs[REND_NUM]  //multiple depth buffer images [CAMERA_RES_W X CAMERA_RES_H ] X [REND_NUM]
)
{
	int buffW = REND_X_NUM*CAMERA_RES_W; 
	int buffH = REND_Y_NUM*CAMERA_RES_H;

	m_ogl.MakeOpenGLCurrent();

	//prepare back buffers 
	GLuint canvasFrameBuffer ;
	GLuint canvasRenderBuffer;
	GLuint canvalDepthBuffer;
	glGenFramebuffers(1, &canvasFrameBuffer);
	glBindFramebuffer( GL_FRAMEBUFFER, canvasFrameBuffer);

	glGenRenderbuffers(1, &canvasRenderBuffer);
	glBindRenderbuffer(       GL_RENDERBUFFER, canvasRenderBuffer);
	glRenderbufferStorage(    GL_RENDERBUFFER, GL_RGBA4, buffW, buffH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, canvasRenderBuffer);

	glGenRenderbuffers(1, &canvalDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, canvalDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffW, buffH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, canvalDepthBuffer);

	//clear color
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);


	//render (REND_X_NUM x REND_Y_NUM) images
	for( int ry = 0; ry < REND_Y_NUM; ++ry)
	{
		for( int rx = 0; rx < REND_X_NUM; ++rx)
		{
			CameraParam &cam = cameras[ rx + ry * REND_X_NUM ];
			const double fovy = cam.calcFovInY();

			// projection
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( fovy, CAMERA_RES_W / (double)CAMERA_RES_H , cam.m_fLen, 1000);
			glViewport(CAMERA_RES_W * rx, CAMERA_RES_H * ry, CAMERA_RES_W, CAMERA_RES_H);

			//model view
			glMatrixMode( GL_MODELVIEW ) ;
			glLoadIdentity();
			
			EVec3d cp, cf, cy;
			cam.getCamearaPos( cp,cf,cy);
			gluLookAt(  cp[0], cp[1], cp[2],  
						cf[0], cf[1], cf[2], 
						cy[0], cy[1], cy[2]);

			//light
			float Lposition[] = { 0, 10000, 0, 0 };
			float Ldiffuse [] = { 1, 1, 1, 1 };

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glLightfv(GL_LIGHT0, GL_POSITION, Lposition);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, Ldiffuse); // 「光源０」の「拡散光」

			TCore::getInst()->drawScene();
		}
	}

	//get depth frame buffer 
	glReadPixels(0, 0, buffW, buffH, GL_DEPTH_COMPONENT, GL_FLOAT, depthImg_full);

	//get color buffer (do not need this)
	//glReadPixels(0, 0, buffW, buffH, GL_RGBA, GL_UNSIGNED_BYTE, idRefImg);


	// get depthImg_depth
	for( int y = 0; y < REND_Y_NUM * CAMERA_RES_H; ++y)
	{
		int y_odd = y % CAMERA_RES_H;
		int y_idx = y / CAMERA_RES_H;

		for( int x_idx = 0; x_idx < REND_X_NUM; ++x_idx)
		{
			float *d   =  depthImg_subs[x_idx + y_idx * REND_X_NUM ];
			float *src = &depthImg_full[ x_idx * CAMERA_RES_W + y * CAMERA_RES_W * REND_X_NUM];
			memcpy( &d[ y_odd * CAMERA_RES_W], src, sizeof( float ) * CAMERA_RES_W );
		}
	}


	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteFramebuffers (1, &canvasFrameBuffer);
	glDeleteRenderbuffers(1, &canvasRenderBuffer);
	glDeleteRenderbuffers(1, &canvalDepthBuffer );

	glDrawBuffer(GL_BACK);
	glFinish();
}























// CTexMapperView 描画

void CTexMapperView::OnDraw(CDC* /*pDC*/)
{
	CTexMapperDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;


	m_ogl.MakeOpenGLCurrent() ;
	//clear color
	glClearColor( 0,0,0,0 ) ;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT );


	EVec3d cp, cf, cy;

	for( int ry = 0; ry < REND_Y_NUM; ++ry)
	{
		for( int rx = 0; rx < REND_X_NUM; ++rx)
		{

			CameraParam &cam = CameraParam(0, 0, TCore::getInst()->getCamradi0(), 0, 0, 0, TCore::getInst()->getfocalLen() );
				
			const double fovy = cam.calcFovInY();

			// projection
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( fovy, CAMERA_RES_W / (double)CAMERA_RES_H , cam.m_fLen, 1000);
			glViewport(CAMERA_RES_W * rx, CAMERA_RES_H * ry, CAMERA_RES_W, CAMERA_RES_H);


			//model view
			glMatrixMode( GL_MODELVIEW ) ;
			glLoadIdentity();
			cam.getCamearaPos( cp,cf,cy);
			gluLookAt(  cp[0], cp[1], cp[2],  
						cf[0], cf[1], cf[2], 
						cy[0], cy[1], cy[2]);

			//light
			float Lposition[] = { 0, 10000, 0, 0 };
			float Ldiffuse [] = { 1, 1, 1, 1 };

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glLightfv(GL_LIGHT0, GL_POSITION, Lposition);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, Ldiffuse); // 「光源０」の「拡散光」


			TCore::getInst()->drawScene();
		}
	}


	m_ogl.OnDrawEnd  ();
}









// CTexMapperView 印刷

BOOL CTexMapperView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CTexMapperView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CTexMapperView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}


// CTexMapperView 診断

#ifdef _DEBUG
void CTexMapperView::AssertValid() const
{
	CView::AssertValid();
}

void CTexMapperView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTexMapperDoc* CTexMapperView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTexMapperDoc)));
	return (CTexMapperDoc*)m_pDocument;
}
#endif //_DEBUG


// CTexMapperView メッセージ ハンドラー


int CTexMapperView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_ogl.OnCreate(this);

	return 0;
}


void CTexMapperView::OnDestroy()
{
	CView::OnDestroy();

	m_ogl.OnDestroy();
}


BOOL CTexMapperView::OnEraseBkgnd(CDC* pDC)
{
	return true;

	//return CView::OnEraseBkgnd(pDC);
}


void CTexMapperView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_isL = 1;
	m_ogl.ButtonDownForTranslate(point);
	CView::OnLButtonDown(nFlags, point);
}


void CTexMapperView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_isL = 0;
	m_ogl.ButtonUp();
	CView::OnLButtonUp(nFlags, point);
}


void CTexMapperView::OnMButtonDown(UINT nFlags, CPoint point)
{
	m_isM = 1;
	m_ogl.ButtonDownForZoom(point);
	CView::OnMButtonDown(nFlags, point);
}


void CTexMapperView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	m_isM = 0;
	m_ogl.ButtonUp();
	CView::OnMButtonUp(nFlags, point);
}


void CTexMapperView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_isL || m_isM || m_isR) {
		m_ogl.MouseMove(point);
		RedrawWindow();
	}
	CView::OnMouseMove(nFlags, point);
}


void CTexMapperView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_isR = 1;
	m_ogl.ButtonDownForRotate(point);
	CView::OnRButtonDown(nFlags, point);
}


void CTexMapperView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_isR = 0;
	m_ogl.ButtonUp();
	CView::OnRButtonUp(nFlags, point);
}


void CTexMapperView::OnSize(UINT nType, int cx, int cy)
{
	CRect wrect;
	this->GetWindowRect(&wrect);
	this->MoveWindow(0, 0, CAMERA_RES_W * REND_X_NUM, CAMERA_RES_H* REND_Y_NUM, TRUE);
	m_ogl.OnSize(CAMERA_RES_W * REND_X_NUM, CAMERA_RES_H* REND_Y_NUM);
	//CView::OnSize(nType, cx, cy);
}










void CTexMapperView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( nChar=='F' )
	{
		TCore::getInst()->cameraCalibration();
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
