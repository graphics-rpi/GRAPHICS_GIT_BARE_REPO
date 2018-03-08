/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Prosilica is prohibited.
|
|==============================================================================
|
| File:	        IPInput.cpp	
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the camera IP address input window
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
#include "IPInput.h"

#include <Winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIPInput dialog


CIPInput::CIPInput(CWnd* pParent /*=NULL*/)
	: CDialog(CIPInput::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIPInput)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_Value = 0;
}

void CIPInput::SetAddress( DWORD dwAddress )
{
    m_Address.SetAddress(dwAddress);
}

void CIPInput::GetAddress( DWORD& dwAddress )
{
    dwAddress = m_Value;
}


void CIPInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIPInput)
	DDX_Control(pDX, IDC_IPADDRESS1, m_Address);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIPInput, CDialog)
	//{{AFX_MSG_MAP(CIPInput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIPInput message handlers

BOOL CIPInput::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_Address.SetFieldFocus(0);
    m_Address.SetFocus();
	
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIPInput::OnOK() 
{
	// TODO: Add extra validation here

    if(m_Address.GetAddress(m_Value) != 4)
        m_Value = 0;
    else
        m_Value = htonl(m_Value);
	
	CDialog::OnOK();
}
