#pragma once

#include "resource.h"

// TDlgManual �_�C�A���O

class TDlgManual : public CDialogEx
{
	DECLARE_DYNAMIC(TDlgManual)

public:
	TDlgManual(CWnd* pParent = NULL);   // �W���R���X�g���N�^�[
	virtual ~TDlgManual();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_DIALOG_1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
};
