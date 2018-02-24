
// TextureGeneratorView.h : CTextureGeneratorView クラスのインターフェイス
//

#pragma once

#include "TextureGeneratorDoc.h"
#include "../../Common/TOGL.h"


class CTextureGeneratorView : public CView
{


	static CTextureGeneratorView *m_self;
protected: // シリアル化からのみ作成します。
	CTextureGeneratorView();
	DECLARE_DYNCREATE(CTextureGeneratorView)


	TOGL m_ogl;
	bool m_bL, m_bR, m_bM;

// 属性
public:
	CTextureGeneratorDoc* GetDocument() const;

	void rendIDrefImage(
		const int idRefW, const int idRefH, 
		const EVec3d camP, const EVec3d camC, const EVec3d camU, 
		const double fovy, const double nearD, const double farD,
		byte* idRefImg, float* depthImg);

	static CTextureGeneratorView* getInst() { return m_self; }

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
	virtual ~CTextureGeneratorView();
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
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType,int cx,int cy);

	afx_msg void OnLButtonUp(  UINT nFlags,CPoint point);
	afx_msg void OnMButtonUp(  UINT nFlags,CPoint point);
	afx_msg void OnRButtonUp(  UINT nFlags,CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnMouseMove(  UINT nFlags,CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // TextureGeneratorView.cpp のデバッグ バージョン
inline CTextureGeneratorDoc* CTextureGeneratorView::GetDocument() const
   { return reinterpret_cast<CTextureGeneratorDoc*>(m_pDocument); }
#endif

