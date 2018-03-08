// ipconfig.h : main header file for the IPCONFIG application
//

#if !defined(AFX_IPCONFIG_H__F41B7AE5_DF55_4949_83DA_02953CB714C0__INCLUDED_)
#define AFX_IPCONFIG_H__F41B7AE5_DF55_4949_83DA_02953CB714C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CApp:
// See ipconfig.cpp for the implementation of this class
//

class CApp : public CWinApp
{
public:
	CApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IPCONFIG_H__F41B7AE5_DF55_4949_83DA_02953CB714C0__INCLUDED_)
