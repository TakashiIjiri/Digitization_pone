
// TextureGeneratorView.cpp : CTextureGeneratorView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "TextureGenerator.h"
#endif

#include "TextureGeneratorDoc.h"
#include "TextureGeneratorView.h"
#include "TCore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTextureGeneratorView

IMPLEMENT_DYNCREATE(CTextureGeneratorView, CView)

BEGIN_MESSAGE_MAP(CTextureGeneratorView, CView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_WM_ERASEBKGND()
ON_WM_SIZE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MBUTTONDOWN()
ON_WM_MBUTTONUP()
ON_WM_RBUTTONDOWN()
ON_WM_RBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CTextureGeneratorView コンストラクション/デストラクション

CTextureGeneratorView *CTextureGeneratorView::m_self = 0;

CTextureGeneratorView::CTextureGeneratorView()
{
	m_self = this;
	m_bL = m_bR = m_bM = false;


}

CTextureGeneratorView::~CTextureGeneratorView()
{
}


BOOL CTextureGeneratorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}

// CTextureGeneratorView 描画


void CTextureGeneratorView::OnDraw(CDC* /*pDC*/)
{
	CTextureGeneratorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;


	m_ogl.SetClearColor( 1,1,1,0);
	glColor3d(1, 1, 1);

	m_ogl.OnDrawBegin(40, 0.1, 1000);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	float pos1 [] = {   0, 10000,   0, 0 };
	float pos2 [] = { 100, 10000,   0, 0 };
	float pos3 [] = {   0, 10000, 100, 0 };
	float diff1[] = { 1, 1, 1, 1 };
	float ambi1[] = { 1, 1, 1, 1 };
	float ambi2[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_POSITION, pos1 );
	glLightfv(GL_LIGHT0, GL_DIFFUSE , diff1);
	glLightfv(GL_LIGHT0, GL_AMBIENT , ambi1);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2 );
	glLightfv(GL_LIGHT1, GL_DIFFUSE , diff1);
	glLightfv(GL_LIGHT1, GL_AMBIENT , ambi2);
	glLightfv(GL_LIGHT2, GL_POSITION, pos3 );
	glLightfv(GL_LIGHT2, GL_DIFFUSE , diff1);
	glLightfv(GL_LIGHT2, GL_AMBIENT , ambi2);


	// axies -------------------------------------
	/*
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
		glColor3d(1, 0, 0); glVertex3d(0, 0, 0);  glVertex3d(35, 0, 0);
		glColor3d(0, 1, 0); glVertex3d(0, 0, 0);  glVertex3d(0, 35, 0);
		glColor3d(0, 0, 1); glVertex3d(0, 0, 0);  glVertex3d(0, 0, 35);
	glEnd();
	*/
	TCore::getInst()->drawScene();
	m_ogl.OnDrawEnd  ();
}


// CTextureGeneratorView 印刷

BOOL CTextureGeneratorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CTextureGeneratorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CTextureGeneratorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}


// CTextureGeneratorView 診断

#ifdef _DEBUG
void CTextureGeneratorView::AssertValid() const
{
	CView::AssertValid();
}

void CTextureGeneratorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTextureGeneratorDoc* CTextureGeneratorView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTextureGeneratorDoc)));
	return (CTextureGeneratorDoc*)m_pDocument;
}
#endif //_DEBUG


// CTextureGeneratorView メッセージ ハンドラー


int CTextureGeneratorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_ogl.OnCreate( this );

	return 0;
}


void CTextureGeneratorView::OnDestroy()
{
	CView::OnDestroy();
	m_ogl.OnDestroy( );

}


BOOL CTextureGeneratorView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}


void CTextureGeneratorView::OnSize(UINT nType,int cx,int cy)
{
	m_ogl.OnSize(cx,cy );
}








void CTextureGeneratorView::OnLButtonDown(UINT nFlags,CPoint point)
{
	m_bL = true;
	m_ogl.ButtonDownForTranslate(point);
}

void CTextureGeneratorView::OnMButtonDown(UINT nFlags,CPoint point)
{
	m_bM = true;
	m_ogl.ButtonDownForZoom(point);
}

void CTextureGeneratorView::OnRButtonDown(UINT nFlags,CPoint point)
{
	m_bR = true;
	m_ogl.ButtonDownForRotate(point);
}


void CTextureGeneratorView::OnLButtonUp(UINT nFlags,CPoint point)
{
	m_bL = false;
	m_ogl.ButtonUp();
}

void CTextureGeneratorView::OnMButtonUp(UINT nFlags,CPoint point)
{
	m_bM = false;
	m_ogl.ButtonUp();
}

void CTextureGeneratorView::OnRButtonUp(UINT nFlags,CPoint point)
{
	m_bR = false;
	m_ogl.ButtonUp();
}


void CTextureGeneratorView::OnMouseMove(UINT nFlags,CPoint point)
{
	if( m_bR || m_bL || m_bM)
	{
		m_ogl.MouseMove( point );
		RedrawWindow();
		fprintf( stderr, "(%d %d %d)", m_bL, m_bR, m_bM);
	}
	
	CView::OnMouseMove(nFlags,point);
}


void CTextureGeneratorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	if (nChar == 'F')
	{
		TCore::getInst()->computeTexture();
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}



void CTextureGeneratorView::rendIDrefImage(
	const int idRefW, 
	const int idRefH,
	const EVec3d camP, 
	const EVec3d camC, 
	const EVec3d camY,
	const double fovy, 
	const double nearD, 
	const double farD,

	byte*   idRefImg, 
	float*  depthImg)
{
	m_ogl.MakeOpenGLCurrent();

	GLuint canvasFrameBuffer ;
	GLuint canvasRenderBuffer;
	GLuint canvalDepthBuffer;
	glGenFramebuffers(1, &canvasFrameBuffer);
	glBindFramebuffer( GL_FRAMEBUFFER, canvasFrameBuffer);

	glGenRenderbuffers(1, &canvasRenderBuffer);
	glBindRenderbuffer(       GL_RENDERBUFFER, canvasRenderBuffer);
	glRenderbufferStorage(    GL_RENDERBUFFER, GL_RGBA4, idRefW, idRefH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, canvasRenderBuffer);

	glGenRenderbuffers(1, &canvalDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, canvalDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, idRefW, idRefH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, canvalDepthBuffer);

	//clear color
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	glDisable(GL_POLYGON_SMOOTH);

	// projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, idRefW  / (double)idRefH, nearD, farD);
	glViewport( 0,0, idRefW, idRefH);

	//model view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camP[0], camP[1], camP[2],
		      camC[0], camC[1], camC[2],
	 	      camY[0], camY[1], camY[2]);

	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	TCore::getInst()->drawPolygonIDs();

	//get depth frame buffer 
	glReadPixels(0, 0, idRefW, idRefH, GL_DEPTH_COMPONENT, GL_FLOAT        , depthImg);
	glReadPixels(0, 0, idRefW, idRefH, GL_RGBA           , GL_UNSIGNED_BYTE, idRefImg);


	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteFramebuffers(1, &canvasFrameBuffer);
	glDeleteFramebuffers(1, &canvasRenderBuffer);
	glDeleteFramebuffers(1, &canvalDepthBuffer );

	glDrawBuffer(GL_BACK);
	glFinish();
}