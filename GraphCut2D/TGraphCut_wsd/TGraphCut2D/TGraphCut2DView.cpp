
// TGraphCut2DView.cpp : implementation of the CTGraphCut2DView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "TGraphCut2D.h"
#endif

#include "TGraphCut2DDoc.h"
#include "TGraphCut2DView.h"
#include "TCore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTGraphCut2DView

IMPLEMENT_DYNCREATE(CTGraphCut2DView, CView)

BEGIN_MESSAGE_MAP(CTGraphCut2DView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT        , &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT , &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CTGraphCut2DView construction/destruction

CTGraphCut2DView::CTGraphCut2DView()
{
	m_bPlaceCPs    = m_bEraseCPs = false;
	m_bLButton     = m_bRButton = m_bMButton    = false;
	m_bTranslating = m_bZooming = false;

	m_transX = -5;
	m_transY = -5;
	m_zoom = 3.5;
}

CTGraphCut2DView::~CTGraphCut2DView()
{
}

BOOL CTGraphCut2DView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CTGraphCut2DView drawing


// CTGraphCut2DView printing

BOOL CTGraphCut2DView::OnPreparePrinting(CPrintInfo* pInfo){ return DoPreparePrinting(pInfo); }
void CTGraphCut2DView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/){}
void CTGraphCut2DView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/){}


// CTGraphCut2DView diagnostics

#ifdef _DEBUG
void CTGraphCut2DView::AssertValid() const
{
	CView::AssertValid();
}
void CTGraphCut2DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
CTGraphCut2DDoc* CTGraphCut2DView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTGraphCut2DDoc)));
	return (CTGraphCut2DDoc*)m_pDocument;
}
#endif //_DEBUG


// CTGraphCut2DView message handlers


/*-------------------------------------------------
MFC のviewに OpenGLを貼り付けるため、
 - OnCreate
 - OnDestroy
 - OnSide
 - OnEraseBkgnd
 - OnDraw
を書き換える
---------------------------------------------------*/
int CTGraphCut2DView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1) return -1;
	TCore::getInst()->m_ogl.OnCreate( this );
	return 0;
}

void CTGraphCut2DView::OnDestroy()
{
	CView::OnDestroy();
	TCore::getInst()->m_ogl.OnDestroy();
}

void CTGraphCut2DView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	TCore::getInst()->m_ogl.OnSize(cx,cy);
}

BOOL CTGraphCut2DView::OnEraseBkgnd(CDC* pDC)
{
	return true;//CView::OnEraseBkgnd(pDC);
}

void CTGraphCut2DView::OnDraw(CDC* /*pDC*/)
{
	CTGraphCut2DDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	TCore::getInst()->m_ogl.OnDraw_Begin();

	glPushMatrix();
	glTranslated( m_transX, m_transY, 0);
	glScaled( m_zoom,m_zoom,m_zoom );

	TCore::getInst()->drawScene();

	glPopMatrix();
	TCore::getInst()->m_ogl.OnDraw_End  ();
};







void CTGraphCut2DView::convertCursorPosToWorldPos( const CPoint &point, double &x, double &y)
{
	TCore::getInst()->m_ogl.GetCursorPos( point.x, point.y, x,y);
	x -= m_transX; x /= m_zoom;
	y -= m_transY; y /= m_zoom;
}




/*-------------------------------------------------
Mouse Listenerを実装する(短いコードだし、classを作らずここで実装する)
右ドラッグ: 視点移動
左ドラッグ: 視点移動
中ドラッグ: zoom

shift + 左ドラッグ : foreground constraints
shift + 右ドラッグ : background constraints
shift + 中ドラッグ : erase constraints

---------------------------------------------------*/

void CTGraphCut2DView::OnMouseMove(UINT nFlags, CPoint point)
{
	if( !m_bLButton && !m_bRButton && !m_bMButton ) return;

	double x,y; 
	convertCursorPosToWorldPos( point, x,y );

	if(      m_bPlaceCPs    ) TCore::getInst()->addCP( x, y, m_bLButton, 0.05 );
	else if( m_bEraseCPs    ) TCore::getInst()->pickCPtoRemove(x,y); 
	else if( m_bTranslating ) 
	{
		double x1,y1, x2, y2;  
		TCore::getInst()->m_ogl.GetCursorPos( point.x, point.y, x2, y2);
		TCore::getInst()->m_ogl.GetCursorPos( m_prePoint.x, m_prePoint.y, x1, y1);
		m_transX += x2 - x1;
		m_transY += y2 - y1;
	}
	else if( m_bZooming )
	{
		m_transX /= m_zoom;
		m_transY /= m_zoom; 

		m_zoom *= 1 + 0.001*( point.y - m_prePoint.y);

		m_transX *= m_zoom;
		m_transY *= m_zoom; 

	}

	m_prePoint = point;
	RedrawWindow();
}





void CTGraphCut2DView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLButton = true;
	SetCapture();
	
	double x,y; 
	convertCursorPosToWorldPos( point, x,y );

	m_prePoint = point;

	if (isShiftKeyOn())
	{
		m_bPlaceCPs = true;
		TCore::getInst()->addCP( x, y, true );
	}
	else
	{
		m_bTranslating = true;
	}
}


void CTGraphCut2DView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( m_bPlaceCPs) TCore::getInst()->runGraphCutSegmentation();

	m_bLButton = m_bTranslating = m_bPlaceCPs = false;

	ReleaseCapture();
	this->RedrawWindow();
}


void CTGraphCut2DView::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_bRButton = true;

	double x,y;
	convertCursorPosToWorldPos( point, x,y );
	m_prePoint = point;

	if (isShiftKeyOn())
	{
		m_bPlaceCPs = true;
		TCore::getInst()->addCP( x, y, false );
	}
	else 
	{
		m_bTranslating = true;
	}
}



void CTGraphCut2DView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if( m_bPlaceCPs) TCore::getInst()->runGraphCutSegmentation();

	m_bRButton = m_bTranslating = m_bPlaceCPs = false;

	ReleaseCapture();
	this->RedrawWindow();
}


void CTGraphCut2DView::OnMButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_bMButton = true;
	
	if( isShiftKeyOn() ) m_bEraseCPs = true;
	else                 m_bZooming  = true;
	m_prePoint = point;
}

void CTGraphCut2DView::OnMButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_bMButton = m_bZooming = m_bEraseCPs = false;
	RedrawWindow();
}



void CTGraphCut2DView::OnLButtonDblClk(UINT nFlags, CPoint point){}
void CTGraphCut2DView::OnRButtonDblClk(UINT nFlags, CPoint point){}
void CTGraphCut2DView::OnMButtonDblClk(UINT nFlags, CPoint point){}
