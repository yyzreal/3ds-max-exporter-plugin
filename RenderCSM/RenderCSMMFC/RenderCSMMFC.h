// RenderCSMMFC.h : main header file for the RenderCSMMFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CRenderCSMMFCApp:
// See RenderCSMMFC.cpp for the implementation of this class
//

class CRenderCSMMFCApp : public CWinApp
{
public:
	CRenderCSMMFCApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle( LONG lCount );
};

extern CRenderCSMMFCApp theApp;