
// TextureGenerator.h : TextureGenerator �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C��
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"       // ���C�� �V���{��


// CTextureGeneratorApp:
// ���̃N���X�̎����ɂ��ẮATextureGenerator.cpp ���Q�Ƃ��Ă��������B
//

class CTextureGeneratorApp : public CWinApp
{
public:
	CTextureGeneratorApp();


// �I�[�o�[���C�h
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ����
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTextureGeneratorApp theApp;
