/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        SaveNextDlg.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Defines the File->Save next frame, menu item.
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
*/

#if !defined(AFX_SAVENEXTDLG_H__ECEDEC5D_00AA_4511_95AE_C29B6AA94BED__INCLUDED_)
#define AFX_SAVENEXTDLG_H__ECEDEC5D_00AA_4511_95AE_C29B6AA94BED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveNextDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SaveNextDlg dialog

class CMainFrame; // forward def

class CSaveNextDlg : public CDialog
{
// Construction
public:
	CSaveNextDlg(CMainFrame* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SaveNextDlg)
	enum { IDD = IDD_SAVENEXTFRAME };
	CStatic	m_Status;
	CButton	m_Cancel;
	CButton	m_Save;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SaveNextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SaveNextDlg)
#ifdef WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#else
	afx_msg void OnTimer(UINT nIDEvent);
#endif
	//}}AFX_MSG

    void OnOK( );
    void OnCancel();

	DECLARE_MESSAGE_MAP()

private: // data

    CMainFrame* m_MainFrame;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVENEXTDLG_H__ECEDEC5D_00AA_4511_95AE_C29B6AA94BED__INCLUDED_)
