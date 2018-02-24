
// MainFrm.h : CMainFrame クラスのインターフェイス
//

#pragma once

class CMainFrame : public CFrameWnd
{
	static CMainFrame* m_mySelf;
protected: // シリアル化からのみ作成します。
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 属性
public:

	static CMainFrame* getInst() { return m_mySelf;  }


// 操作
public:

// オーバーライド
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 実装
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // コントロール バー用メンバー
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// 生成された、メッセージ割り当て関数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

};


