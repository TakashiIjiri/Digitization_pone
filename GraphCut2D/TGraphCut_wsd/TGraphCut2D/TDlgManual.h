#pragma once

#include "resource.h"

// TDlgManual ダイアログ

class TDlgManual : public CDialogEx
{
	DECLARE_DYNAMIC(TDlgManual)

public:
	TDlgManual(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~TDlgManual();

// ダイアログ データ
	enum { IDD = IDD_DIALOG_1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
};
