
// TexMapper.h : TexMapper アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#include <stdarg.h>

#define LINE_SIZE  1024

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル




class CTexMapperApp : public CWinApp
{
public:
	CTexMapperApp();


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTexMapperApp theApp;
