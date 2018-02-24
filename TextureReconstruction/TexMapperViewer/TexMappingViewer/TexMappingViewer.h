
// TexMappingViewer.h : TexMappingViewer アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#include <stdarg.h>

#define LINE_SIZE  1024


#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル


// CTexMappingViewerApp:
// このクラスの実装については、TexMappingViewer.cpp を参照してください。
//

class CTexMappingViewerApp : public CWinApp
{
public:
	CTexMappingViewerApp();


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTexMappingViewerApp theApp;
