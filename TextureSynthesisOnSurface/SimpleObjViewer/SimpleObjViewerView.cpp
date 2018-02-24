
// SimpleObjViewerView.cpp : CSimpleObjViewerView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "SimpleObjViewer.h"
#endif

#include "SimpleObjViewerDoc.h"
#include "SimpleObjViewerView.h"

#include "texsynthesissurface.h"

#include <map>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSimpleObjViewerView

IMPLEMENT_DYNCREATE(CSimpleObjViewerView, CView)

BEGIN_MESSAGE_MAP(CSimpleObjViewerView, CView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()




static void t_getBB(
	const int     N, 
	const EVec3d *verts, 
	EVec3d &bbMin, 
	EVec3d &bbMax)
{
	bbMin << DBL_MAX, DBL_MAX, DBL_MAX;
	bbMax <<-DBL_MAX,-DBL_MAX,-DBL_MAX;

	for( int i=0; i < N; ++i)
	{
		bbMin[0] = min(bbMin[0], verts[i][0]);
		bbMin[1] = min(bbMin[1], verts[i][1]);
		bbMin[2] = min(bbMin[2], verts[i][2]);
		bbMax[0] = max(bbMax[0], verts[i][0]);
		bbMax[1] = max(bbMax[1], verts[i][1]);
		bbMax[2] = max(bbMax[2], verts[i][2]);
	}


}




// CSimpleObjViewerView コンストラクション/デストラクション

CSimpleObjViewerView::CSimpleObjViewerView()
{
	m_bL = m_bR = m_bM = false;



	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, "wavefront obj (*.obj)|*.obj||");
	if (dlg.DoModal() != IDOK) exit(0);
	
	m_mesh.initialize(dlg.GetPathName());

	EVec3d gc = m_mesh.getGravityCenter();
	m_mesh.Translate( -gc );
	m_mesh.updateNormal();


	CFileDialog dlg1(TRUE, NULL, NULL, OFN_HIDEREADONLY, "texture (*.bmp;*.jpg)|*.bmp;*.jpg||");
	if (dlg1.DoModal() != IDOK) exit(0);
	bool flip;
	m_texture.AllocateFromFile( dlg1.GetPathName(), flip, 0);
	if( flip) m_texture.FlipInY();


	
	//compute flow field
	const EVec3d Xdir(1,0,0);
	m_vFlow.resize( m_mesh.m_vSize );
	for( int i=0 ; i < m_mesh.m_vSize; ++ i )
	{	
		const EVec3d &N = m_mesh.m_v_norms[i];
		m_vFlow[i] = (Xdir - Xdir.dot(N)*N).normalized();
	}

	for( int kk=0; kk < 3; ++kk)
	{
		vector<EVec3d> dirs(m_mesh.m_vSize );
		for( int i=0 ; i < m_mesh.m_vSize; ++ i )
		{
			for( auto it : m_mesh.m_v_RingVs[i]) dirs[i] += m_vFlow[it];
			const EVec3d &N = m_mesh.m_v_norms[i];
			dirs[i] = (dirs[i] - dirs[i].dot(N)*N ).normalized();
		}
		m_vFlow = dirs;
	}	

	EVec3d bbMin, bbMax;
	t_getBB(m_mesh.m_vSize, m_mesh.m_verts, bbMin, bbMax);
	EVec3d bb = bbMax - bbMin;
	fprintf( stderr, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	Trace(bb);

	const int SMOOTHING_NUM = 100;
	const double PATCH_PITCH = bb.norm() / 2000;

	//TImage2D tex0 = HoleRetrieval( m_mesh, m_vFlow, m_texture, SMOOTHING_NUM, PATCH_PITCH/6);
	//tex0.FlipInY();
	//tex0.saveAsFile("filledResult_fill0.bmp", 0);
	//tex0.FlipInY();

	TImage2D tex1 = HoleRetrieval( m_mesh, m_vFlow, m_texture, SMOOTHING_NUM, PATCH_PITCH/4);
	tex1.FlipInY();
	tex1.saveAsFile("filledResult_fill1.bmp", 0);
	tex1.FlipInY();

	//TImage2D tex2 = HoleRetrieval( m_mesh, m_vFlow, m_texture, SMOOTHING_NUM, PATCH_PITCH/2);
	//tex2.FlipInY();
	//tex2.saveAsFile("filledResult_fill2.bmp", 0);
	//tex2.FlipInY();
	//
	//TImage2D tex3 = HoleRetrieval( m_mesh, m_vFlow, m_texture, SMOOTHING_NUM, PATCH_PITCH  );
	//tex3.FlipInY();
	//tex3.saveAsFile("filledResult_fill3.bmp", 0);
	//tex3.FlipInY();

	m_texture = tex1;
}



CSimpleObjViewerView::~CSimpleObjViewerView()
{
}

BOOL CSimpleObjViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}

// CSimpleObjViewerView 描画

void CSimpleObjViewerView::OnDraw(CDC* /*pDC*/)
{
	CSimpleObjViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: この場所にネイティブ データ用の描画コードを追加します。
}


// CSimpleObjViewerView 印刷

BOOL CSimpleObjViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CSimpleObjViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CSimpleObjViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}


// CSimpleObjViewerView 診断

#ifdef _DEBUG
void CSimpleObjViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CSimpleObjViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimpleObjViewerDoc* CSimpleObjViewerView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleObjViewerDoc)));
	return (CSimpleObjViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CSimpleObjViewerView メッセージ ハンドラー


int CSimpleObjViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_ogl.OnCreate(this);
	m_ogl.SetCam( EVec3d(0,0,10), EVec3d(0,0,0), EVec3d(0,1,0));

	return 0;
}


void CSimpleObjViewerView::OnDestroy()
{
	CView::OnDestroy();
	m_ogl.OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}


BOOL CSimpleObjViewerView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}



void CSimpleObjViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_ogl.OnSize(cx,cy);

}


EVec3d visPoint;


static float diff[4] = {1.0f, 1.0f, 1.0f, 0.5f};
static float ambi[4] = {0.6f, 0.5f, .5f, 0.5f};
static float spec[4] = {1.0f, 1.0f, 1.0f, 0.5f};
static float shin[1] = {10.0f};

void CSimpleObjViewerView::OnPaint()
{
	CPaintDC dc(this); 


	
	m_ogl.OnDrawBegin();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glDisable( GL_LIGHTING );
	glLineWidth(5);
	glBegin( GL_LINES );
		glColor3d(1,0,0); glVertex3d(0,0,0); glVertex3d(10,0,0);
		glColor3d(0,1,0); glVertex3d(0,0,0); glVertex3d(0,10,0);
		glColor3d(0,0,1); glVertex3d(0,0,0); glVertex3d(0,0,10);
	glEnd  ();


	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0);
	glEnable( GL_LIGHT1);
	glEnable( GL_LIGHT2);
	glEnable( GL_TEXTURE_2D );

	m_texture.Bind(0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE  , diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT  , ambi);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shin);
	EVec3d *Vs = m_mesh.m_verts  ;
	EVec3d *Ns = m_mesh.m_v_norms;
	TPoly  *Ps = m_mesh.m_polys  ;

	EVec2d *Ts = m_mesh.m_uvs    ;

	glBegin( GL_TRIANGLES );
	for(int p=0; p < m_mesh.m_pSize; ++p)
	{
		int *vidx = Ps[p].vIdx;
		int *tidx = Ps[p].tIdx;
		glTexCoord2dv( Ts[tidx [0]].data()); glNormal3dv(Ns[vidx[0]].data()); glVertex3dv(Vs[vidx[0]].data());
		glTexCoord2dv( Ts[tidx [1]].data()); glNormal3dv(Ns[vidx[1]].data()); glVertex3dv(Vs[vidx[1]].data());
		glTexCoord2dv( Ts[tidx [2]].data()); glNormal3dv(Ns[vidx[2]].data()); glVertex3dv(Vs[vidx[2]].data());
	}
	glEnd();

	/*
	glDisable( GL_TEXTURE_2D );
	glDisable(GL_LIGHTING );
	glBegin( GL_LINES );
	for(int ei=0; ei < m_mesh.m_eSize; ++ei)
	{
		int* v=m_mesh.m_edges[ei].v;

		if( m_mesh.m_edges[ei].bAtlsSeam ) glColor3d(1,0,0);
		else                               continue;//glColor3d(1,1,1);
		glVertex3dv(Vs[v[0]].data());
		glVertex3dv(Vs[v[1]].data());
	}
	glEnd();

	glPointSize(15);
	glBegin(GL_POINTS);
	glVertex3dv(visPoint.data());
	glColor3d(1,0,0);
	glVertex3dv( TDbgInfo::getInst()->m_vtx.data() );
	glEnd();
	*/
	

	m_ogl.OnDrawEnd();

}












void CSimpleObjViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bL = true;
	//m_ogl.BtnDown_Trans(point);

	
	int polyIdx;
	EVec3d rayP,rayD, pos;
	m_ogl.GetCursorRay( point , rayP, rayD);

	if (m_mesh.pickByRay(rayP, rayD, pos, polyIdx))
	{

		visPoint = pos;

		const int W = m_texture.m_W;
		const int H = m_texture.m_H;
		int *polyIdTex = new int[W*H];
		t_genPolygonIDtexture( m_mesh, W,H,polyIdTex);

		int *ti = m_mesh.m_polys[polyIdx].tIdx;
		int *vi = m_mesh.m_polys[polyIdx].vIdx;
		EVec2d uv = (m_mesh.m_uvs[ti[0]] + m_mesh.m_uvs[ti[1]] + m_mesh.m_uvs[ti[2]] ) / 3.0;
		visPoint  = (m_mesh.m_verts[vi[0]] + m_mesh.m_verts[vi[1]] + m_mesh.m_verts[vi[2]] ) / 3.0;


		EVec3i pix( (int)(uv[0] * W), (int)(uv[1] * H), (int)(uv[0] * W) + W*(int)(uv[1] * H));


		int  patchUvId[PATCH_WW ];
		byte patchRGB [PATCH_WW3];
		t_getNeighboringPatch(W,H,pix, polyIdTex[pix[2]], m_mesh, m_vFlow, PATCH_R, 0.1, patchUvId);
		t_getPatchByIdx(m_texture, patchUvId, patchRGB);

		fprintf( stderr, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa %d %d\n", polyIdTex[pix[2]], polyIdx);
		t_exportPatch(51,patchRGB,"aaaaaa.bmp");

	}
	m_ogl.RedrawWindow();

	
}


void CSimpleObjViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bL = false;
}



void CSimpleObjViewerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_bR = true;
	m_ogl.ButtonDownForRotate(point);
}


void CSimpleObjViewerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bR = false;
	m_ogl.ButtonUp();
}


void CSimpleObjViewerView::OnMButtonDown(UINT nFlags, CPoint point)
{
	m_bM = true;
	m_ogl.ButtonDownForZoom(point);
}

void CSimpleObjViewerView::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_bM = false;
	m_ogl.ButtonUp();
}



void CSimpleObjViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bL)
	{
		int polyIdx;
		EVec3d rayP,rayD, pos;
		m_ogl.GetCursorRay( point , rayP, rayD);

		if (m_mesh.pickByRay(rayP, rayD, pos, polyIdx))
		{

		}
		m_ogl.RedrawWindow();
	}


	if (m_bR || m_bM)
	{
		m_ogl.MouseMove( point );
		m_ogl.RedrawWindow();
	}

	CView::OnMouseMove(nFlags, point);
}







