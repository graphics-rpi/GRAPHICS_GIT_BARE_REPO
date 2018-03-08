/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        SaveNextDlg.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implements the File->Save next frame, menu item.
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

#include "stdafx.h"
#include "sampleviewer.h"
#include "SaveNextDlg.h"
#include "MainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SaveNextDlg dialog


CSaveNextDlg::CSaveNextDlg(CMainFrame* pParent /*=NULL*/)
	: CDialog(CSaveNextDlg::IDD, pParent)
{
    m_MainFrame = pParent;


	//{{AFX_DATA_INIT(SaveNextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSaveNextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SaveNextDlg)
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDCANCEL, m_Cancel);
	DDX_Control(pDX, IDOK, m_Save);
	//}}AFX_DATA_MAP

    m_Status.SetWindowText("Click on 'Save' when ready ...");
}


void CSaveNextDlg::OnOK()
{
    if(!m_MainFrame->IsGrabNextFrame())
    {
        m_Status.SetWindowText("Waiting for a frame ...");

        //m_Cancel.EnableWindow(true);
        m_MainFrame->GrabNextFrame(true);
        //m_Save.EnableWindow(false);

        SetTimer(1,100,NULL);

        m_Save.SetWindowText("Cancel");
    }
    else
    {
        KillTimer(1);
        m_Status.SetWindowText("Click on 'Save' when ready ...");
        m_Save.SetWindowText("Save frame");
    }

}

void CSaveNextDlg::OnCancel()
{
    /*
    KillTimer(1);

    m_MainFrame->GrabNextFrame(false);
    */

    CDialog::OnCancel();
}


BEGIN_MESSAGE_MAP(CSaveNextDlg, CDialog)
	//{{AFX_MSG_MAP(SaveNextDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SaveNextDlg message handlers

#ifdef WIN64
void CSaveNextDlg::OnTimer(UINT_PTR nIDEvent) 
#else
void CSaveNextDlg::OnTimer(UINT nIDEvent) 
#endif
{
	// TODO: Add your message handler code here and/or call default
	
    if(!m_MainFrame->IsGrabNextFrame())
    {
        KillTimer(1);
        m_Status.SetWindowText("Click on 'Save' when ready ...");
        //m_Save.EnableWindow(true);
        //m_Cancel.EnableWindow(false);
        m_Save.SetWindowText("Save frame");
    }
}
