/* ------------------------------------------------------------
 MainFrm.h is written by Takashi Ijiri @ RIKEN

 This codes is distributed with NYSL (Version 0.9982) license. 
 
 免責事項   : 本ソースコードによって起きたいかなる障害についても、著者は一切の責任を負いません。
 disclaimer : The author (Takashi Ijiri) is not to be held responsible for any problems caused by this software.
 ----------------------------------------------------*/

#pragma once

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar        m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnFile32771();
};


