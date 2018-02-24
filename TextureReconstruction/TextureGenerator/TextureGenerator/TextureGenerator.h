
// TextureGenerator.h : TextureGenerator アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル


// CTextureGeneratorApp:
// このクラスの実装については、TextureGenerator.cpp を参照してください。
//

class CTextureGeneratorApp : public CWinApp
{
public:
	CTextureGeneratorApp();


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTextureGeneratorApp theApp;
