
// SimpleObjViewerView.h : CSimpleObjViewerView �N���X�̃C���^�[�t�F�C�X
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




protected: // �V���A��������̂ݍ쐬���܂��B
	CSimpleObjViewerView();
	DECLARE_DYNCREATE(CSimpleObjViewerView)

// ����
public:
	CSimpleObjViewerDoc* GetDocument() const;



// ����
public:

// �I�[�o�[���C�h
public:
	virtual void OnDraw(CDC* pDC);  // ���̃r���[��`�悷�邽�߂ɃI�[�o�[���C�h����܂��B
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ����
public:
	virtual ~CSimpleObjViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
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

#ifndef _DEBUG  // SimpleObjViewerView.cpp �̃f�o�b�O �o�[�W����
inline CSimpleObjViewerDoc* CSimpleObjViewerView::GetDocument() const
   { return reinterpret_cast<CSimpleObjViewerDoc*>(m_pDocument); }
#endif

