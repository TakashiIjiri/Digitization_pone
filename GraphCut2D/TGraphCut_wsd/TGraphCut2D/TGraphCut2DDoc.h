/* ------------------------------------------------------------
 CTGraphCut2DDoc.h is written by Takashi Ijiri @ RIKEN

 This codes is distributed with NYSL (Version 0.9982) license. 
 
 �Ɛӎ���   : �{�\�[�X�R�[�h�ɂ���ċN���������Ȃ��Q�ɂ��Ă��A���҂͈�؂̐ӔC�𕉂��܂���B
 disclaimer : The author (Takashi Ijiri) is not to be held responsible for any problems caused by this software.
 ----------------------------------------------------*/

#pragma once

class CTGraphCut2DDoc : public CDocument
{
protected: // create from serialization only
	CTGraphCut2DDoc();
	DECLARE_DYNCREATE(CTGraphCut2DDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CTGraphCut2DDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
