// SurfaceCTViewer.cpp : ���C�� �v���W�F�N�g �t�@�C���ł��B

#include "stdafx.h"
#include "./MainForm.h"

using namespace System;
using namespace SurfaceCTViewer;


#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )
#pragma comment( lib, "gdi32.lib" )
#pragma comment( lib, "User32.lib" )

[STAThread]
int main()
{

    Console::WriteLine("Start Viewer\n");
	MainForm::getInst()->ShowDialog();

    return 0;
}
