
// SimpleObjViewer.h : SimpleObjViewer アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル


// CSimpleObjViewerApp:
// このクラスの実装については、SimpleObjViewer.cpp を参照してください。
//

class CSimpleObjViewerApp : public CWinApp
{
public:
	CSimpleObjViewerApp();


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSimpleObjViewerApp theApp;
