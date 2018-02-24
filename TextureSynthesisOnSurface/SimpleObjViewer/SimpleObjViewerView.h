
// SimpleObjViewerView.h : CSimpleObjViewerView クラスのインターフェイス
//

#pragma once

#include "SimpleObjViewerDoc.h"

#include "./COMMON/TOGL.h"
#include "./COMMON/ttexmesh.h"
#include "./COMMON/timage.h"
#include "expmap.h"


class TDbgInfo
{
	TDbgInfo(){}


public:
	static TDbgInfo *getInst(){static TDbgInfo p; return &p;}

	EVec3d m_vtx;
};




class CSimpleObjViewerView : public CView
{
	TOGL		      m_ogl    ;
	TTexMesh	      m_mesh   ;
	TImage2D          m_texture;

	vector<EVec3d>    m_vFlow;

	bool m_bL, m_bR, m_bM;




protected: // シリアル化からのみ作成します。
	CSimpleObjViewerView();
	DECLARE_DYNCREATE(CSimpleObjViewerView)

// 属性
public:
	CSimpleObjViewerDoc* GetDocument() const;



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
	virtual ~CSimpleObjViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp  (UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp  (UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp  (UINT nFlags, CPoint point);
	afx_msg void OnMouseMove  (UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // SimpleObjViewerView.cpp のデバッグ バージョン
inline CSimpleObjViewerDoc* CSimpleObjViewerView::GetDocument() const
   { return reinterpret_cast<CSimpleObjViewerDoc*>(m_pDocument); }
#endif

