// TDlgManual.cpp : 実装ファイル
//

#include "stdafx.h"
#include "TGraphCut2D.h"
#include "TDlgManual.h"
#include "afxdialogex.h"


// TDlgManual ダイアログ

IMPLEMENT_DYNAMIC(TDlgManual, CDialogEx)

TDlgManual::TDlgManual(CWnd* pParent /*=NULL*/)
	: CDialogEx(TDlgManual::IDD, pParent)
{

}

TDlgManual::~TDlgManual()
{
}

void TDlgManual::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TDlgManual, CDialogEx)
END_MESSAGE_MAP()


// TDlgManual メッセージ ハンドラー
