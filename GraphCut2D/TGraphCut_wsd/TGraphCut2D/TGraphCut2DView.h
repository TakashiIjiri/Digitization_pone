/* ------------------------------------------------------------
 CTGraphCut2DView.h is written by Takashi Ijiri @ RIKEN

 This codes is distributed with NYSL (Version 0.9982) license. 
 
 免責事項   : 本ソースコードによって起きたいかなる障害についても、著者は一切の責任を負いません。
 disclaimer : The author (Takashi Ijiri) is not to be held responsible for any problems caused by this software.
 ----------------------------------------------------*/



#pragma once


class CTGraphCut2DView : public CView
{
protected: // create from serialization only
	CTGraphCut2DView();
	DECLARE_DYNCREATE(CTGraphCut2DView)

// Attributes
public:
	CTGraphCut2DDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CTGraphCut2DView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove    (UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown  (UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown  (UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown  (UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp    (UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp    (UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp    (UINT nFlags, CPoint point);

public:
	bool   m_bPlaceCPs, m_bEraseCPs;
	bool   m_bLButton, m_bRButton, m_bMButton; //flag of mouse mode
	bool   m_bTranslating, m_bZooming; //flags of interaction
	double m_transX, m_transY, m_zoom;//view parameter
	CPoint m_prePoint;//mouse point

	void convertCursorPosToWorldPos( const CPoint &point, double &x, double &y);


};




inline bool isCtrKeyOn  (){ return GetKeyState( VK_CONTROL ) < 0 ; }
inline bool isSpaceKeyOn(){ return GetKeyState( VK_SPACE   ) < 0 ; }
inline bool isShiftKeyOn(){ return GetKeyState( VK_SHIFT   ) < 0 ; }


#ifndef _DEBUG  // debug version in TGraphCut2DView.cpp
inline CTGraphCut2DDoc* CTGraphCut2DView::GetDocument() const
   { return reinterpret_cast<CTGraphCut2DDoc*>(m_pDocument); }
#endif

