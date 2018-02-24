
// TexMapperView.h : CTexMapperView �N���X�̃C���^�[�t�F�C�X
//

#pragma once
#include "TOGL.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "TexMapperDoc.h"
#include "TCore.h"


class CTexMapperView : public CView
{
	TOGL   m_ogl   ;
	bool m_isL, m_isM, m_isR;
	static CTexMapperView *m_p;


protected: // �V���A��������̂ݍ쐬���܂��B
	CTexMapperView();
	DECLARE_DYNCREATE(CTexMapperView)

// ����
public:
	

	void offscreenRend_genImages
	( CameraParam cameras[REND_NUM] ,
	  float* depthImg_full,
	  float* depthImg_subs[REND_NUM] );

	CTexMapperDoc* GetDocument() const;

	static CTexMapperView* getInst() { return m_p;  }
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
	virtual ~CTexMapperView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove  (UINT nFlags, CPoint point);
	afx_msg void OnSize       (UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp  (UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp  (UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp  (UINT nFlags, CPoint point);
	afx_msg void OnKeyDown    (UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // TexMapperView.cpp �̃f�o�b�O �o�[�W����
inline CTexMapperDoc* CTexMapperView::GetDocument() const
   { return reinterpret_cast<CTexMapperDoc*>(m_pDocument); }
#endif

