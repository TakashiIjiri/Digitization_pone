
/* ------------------------------------------------------------
 CTGraphCut2DApp.h is written by Takashi Ijiri @ RIKEN

 This codes is distributed with NYSL (Version 0.9982) license. 
 
 免責事項   : 本ソースコードによって起きたいかなる障害についても、著者は一切の責任を負いません。
 disclaimer : The author (Takashi Ijiri) is not to be held responsible for any problems caused by this software.
 ----------------------------------------------------*/



#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CTGraphCut2DApp:
// See TGraphCut2D.cpp for the implementation of this class
//

class CTGraphCut2DApp : public CWinApp
{
public:
	CTGraphCut2DApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTGraphCut2DApp theApp;
