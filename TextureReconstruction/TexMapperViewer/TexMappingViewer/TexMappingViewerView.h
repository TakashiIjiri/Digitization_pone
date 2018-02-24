
// TexMappingViewerView.h : CTexMappingViewerView クラスのインターフェイス
//

#pragma once

#include "TOGL.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "../../Common/CameraInfo.h"
#include "../../Common/ttexmesh.h"
#include "../../Common/timage.h"

#include <vector>
using namespace std;


#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")


class ImgInfo
{
public:
	ImgInfo() {}
	CameraParam    m_cam ;
	vector<EVec3d> m_sphV_diffAngle;
	TImage2D       m_img ;
	double         m_diff;
};



class CTexMappingViewerView : public CView
{
	//Mouse Cursor
	bool  m_bRendSphere;
	bool  m_isL, m_isM, m_isR;
	TOGL  m_ogl;
	TTexMesh m_sph;

	int   m_activeImgInfoIdx;

	//mesh info
	TTexMesh m_mesh;

	//2D images
	vector< ImgInfo > m_imgData;
	TImage2D m_mask ;

	void export_rendImg_fromRandomCamera();

protected: // シリアル化からのみ作成します。
	CTexMappingViewerView();
	DECLARE_DYNCREATE(CTexMappingViewerView)

// 属性
public:
	CTexMappingViewerDoc* GetDocument() const;

// 操作
public:

// オーバーライド
public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画するためにオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 実装
public:
	virtual ~CTexMappingViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd( CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(  UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(  UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(  UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(  UINT nFlags, CPoint point);
	afx_msg void OnSize(       UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(    UINT nChar, UINT nRepCnt, UINT nFlags);

	void backBufDraw( CameraParam c, int W, int H, float* depthImg, byte *colImg);

};

#ifndef _DEBUG  // TexMappingViewerView.cpp のデバッグ バージョン
inline CTexMappingViewerDoc* CTexMappingViewerView::GetDocument() const
   { return reinterpret_cast<CTexMappingViewerDoc*>(m_pDocument); }
#endif

