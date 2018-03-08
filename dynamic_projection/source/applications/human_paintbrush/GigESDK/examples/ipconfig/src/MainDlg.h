/*
|==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:			MainDlg.h
|
| Project/lib:	ipconfig
|
| Target:		Win32
|
| Description:	Main dialog of ipconfig example.
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

#if !defined(AFX_MAINDLG_H__FCBC7F7A_70A8_466A_9627_9226A4AEE598__INCLUDED_)
#define AFX_MAINDLG_H__FCBC7F7A_70A8_466A_9627_9226A4AEE598__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//===== INCLUDE FILES =========================================================

//===== #DEFINES ==============================================================

//===== TYPE DEFINITIONS ======================================================

class CMainDlg : public CDialog
{
public:

	CMainDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CMainDlg();

protected:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainDlg)
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_DATA(CMainDlg)
	enum { IDD = IDD_IPCONFIG_DIALOG };
	CListCtrl	m_ctlList;
	CButton	m_ctlChange;
	//}}AFX_DATA

	// Generated message map functions
	//{{AFX_MSG(CMainDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
#ifdef WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#else
	afx_msg void OnTimer(UINT nIDEvent);
#endif
    virtual void OnOK();
    virtual void OnCancel();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	void refreshCameraList();

};

//===== FUNCTION PROTOTYPES ===================================================

//===== DATA ==================================================================


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDLG_H__FCBC7F7A_70A8_466A_9627_9226A4AEE598__INCLUDED_)
