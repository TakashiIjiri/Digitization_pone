
// TexMappingViewer.h : TexMappingViewer �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C��
//
#pragma once

#include <stdarg.h>

#define LINE_SIZE  1024


#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"       // ���C�� �V���{��


// CTexMappingViewerApp:
// ���̃N���X�̎����ɂ��ẮATexMappingViewer.cpp ���Q�Ƃ��Ă��������B
//

class CTexMappingViewerApp : public CWinApp
{
public:
	CTexMappingViewerApp();


// �I�[�o�[���C�h
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ����
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTexMappingViewerApp theApp;
