/*
|==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies Inc.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies Inc. is prohibited.
|
|==============================================================================
|
| File:			ConfigEdit.h
|
| Project/lib:	ipconfig
|
| Target:		Win32
|
| Description:	Configuration-edit dialog of ipconfig example.
|
| Notes:		
|
|==============================================================================
|
| THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
| WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
| NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
| DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
| INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
| LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
| OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF
| LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
| NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
| EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
|
|==============================================================================
| dd/mon/yy  Author		Notes
|------------------------------------------------------------------------------
| 06/Jun/06  DLee		Original.
| 04/Jun/07  jlv        Reviewed license.
|==============================================================================
*/

#if !defined(AFX_CONFIGEDIT_H__0FEDF58B_B2D7_4915_9128_5CB7AB3E8374__INCLUDED_)
#define AFX_CONFIGEDIT_H__0FEDF58B_B2D7_4915_9128_5CB7AB3E8374__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== INCLUDE FILES =========================================================

#include <PvApi.h>

//===== #DEFINES ==============================================================

//===== TYPE DEFINITIONS ======================================================



class CConfigEdit : public CDialog
{
public:

	CConfigEdit(CWnd* pParent = NULL);   // standard constructor

	// Parameters IN and OUT: 
	tPvIpConfig			configMode;
	DWORD				persistAddress;
	DWORD				persistSubnet;
	DWORD				persistGateway;


protected:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigEdit)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_DATA(CConfigEdit)
	enum { IDD = IDD_EDIT };
	CButton	m_ctlConfigDhcp;
	CButton	m_ctlConfigLla;
	CButton	m_ctlConfigPersist;
	CIPAddressCtrl	m_ctlAddress;
	CIPAddressCtrl	m_ctlSubnet;
	CIPAddressCtrl	m_ctlGateway;
	//}}AFX_DATA

	//{{AFX_MSG(CConfigEdit)
	afx_msg void OnConfig();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//===== FUNCTION PROTOTYPES ===================================================

//===== DATA ==================================================================



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGEDIT_H__0FEDF58B_B2D7_4915_9128_5CB7AB3E8374__INCLUDED_)
