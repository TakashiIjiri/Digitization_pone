
// TexMapper.h : TexMapper �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C��
//
#pragma once

#include <stdarg.h>

#define LINE_SIZE  1024

#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"       // ���C�� �V���{��




class CTexMapperApp : public CWinApp
{
public:
	CTexMapperApp();


// �I�[�o�[���C�h
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ����
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTexMapperApp theApp;
