/*
|==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:			ConfigEdit.cpp
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
| 06/Jun/2006  DLee		Original.
| 04/Jun/2007  jlv      Reviewed license.
|==============================================================================
*/

//===== INCLUDE FILES =========================================================

#include "stdafx.h"
#include "ipconfig.h"
#include "ConfigEdit.h"

//===== #DEFINES ==============================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===== TYPE DEFINITIONS ======================================================

//===== FUNCTION PROTOTYPES ===================================================

//===== DATA (PUBLIC) =========================================================

//===== DATA (PRIVATE) ========================================================

BEGIN_MESSAGE_MAP(CConfigEdit, CDialog)
	//{{AFX_MSG_MAP(CConfigEdit)
	ON_BN_CLICKED(IDC_CONFIG_DHCP, OnConfig)
	ON_BN_CLICKED(IDC_CONFIG_LLA, OnConfig)
	ON_BN_CLICKED(IDC_CONFIG_PERSIST, OnConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//===== IMPLEMENTATION ========================================================


CConfigEdit::CConfigEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigEdit)
	//}}AFX_DATA_INIT
}


void CConfigEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigEdit)
	DDX_Control(pDX, IDC_CONFIG_DHCP, m_ctlConfigDhcp);
	DDX_Control(pDX, IDC_CONFIG_LLA, m_ctlConfigLla);
	DDX_Control(pDX, IDC_CONFIG_PERSIST, m_ctlConfigPersist);
	DDX_Control(pDX, IDC_ADDRESS, m_ctlAddress);
	DDX_Control(pDX, IDC_SUBNET, m_ctlSubnet);
	DDX_Control(pDX, IDC_GATEWAY, m_ctlGateway);
	//}}AFX_DATA_MAP
}


BOOL CConfigEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	switch (configMode)
	{
	case ePvIpConfigPersistent:
		m_ctlConfigPersist.SetCheck(1);
		break;

	case ePvIpConfigAutoIp:
		m_ctlConfigLla.SetCheck(1);
		break;

	case ePvIpConfigDhcp:
	default:
		m_ctlConfigDhcp.SetCheck(1);
	}

	m_ctlAddress.SetAddress(persistAddress);
	m_ctlSubnet.SetAddress(persistSubnet);
	m_ctlGateway.SetAddress(persistGateway);

	OnConfig();
	
	return TRUE;
}


void CConfigEdit::OnConfig() 
{
	const bool			configPersist = (m_ctlConfigPersist.GetCheck() == 1);

	// Enable/disable the persistent IP fields, depending on the configuration
	// mode.

	m_ctlAddress.EnableWindow(configPersist);
	m_ctlSubnet.EnableWindow(configPersist);
	m_ctlGateway.EnableWindow(configPersist);
}


void CConfigEdit::OnOK()
{
	const int			numAddressFields = m_ctlAddress.GetAddress(persistAddress);
	const int			numSubnetFields = m_ctlSubnet.GetAddress(persistSubnet);
	const int			numGatewayFields = m_ctlGateway.GetAddress(persistGateway);


	if (m_ctlConfigPersist.GetCheck() == 1)
	{

		//
		// Partial validation of persistent IP configuration.
		//

		// Address must be filled in.
		if (numAddressFields != 4)
		{
			AfxMessageBox("\"IP address\" is incomplete", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// Subnet mask must be filled in.
		if (numSubnetFields != 4)
		{
			AfxMessageBox("\"Subnet mask\" is incomplete", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// If the gateway is not empty, it must be completely filled in.
		if (numGatewayFields && (numGatewayFields != 4))
		{
			AfxMessageBox("\"Default gateway\" is incomplete", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// If the gatway is non-zero, check that it is on the subnet.
		if (persistGateway &&
			((persistAddress & persistSubnet) != (persistAddress & persistSubnet)))
		{
			AfxMessageBox("\"Default gateway\" is incorrect, because it is not "
						  "on the same subnet as the camera.\n\nYou can leave the "
						  "gateway blank, or all zeros.",
						  MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		configMode = ePvIpConfigPersistent;
	}
	else if (m_ctlConfigDhcp.GetCheck() == 1)
	{
		configMode = ePvIpConfigDhcp;
	}
	else
		configMode = ePvIpConfigAutoIp;


	CDialog::OnOK();
}


