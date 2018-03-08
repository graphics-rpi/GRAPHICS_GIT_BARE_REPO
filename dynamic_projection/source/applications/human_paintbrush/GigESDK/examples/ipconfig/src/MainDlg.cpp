/*
|==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:			MainDlg.cpp
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

//===== INCLUDE FILES =========================================================

#include "stdafx.h"
#include "ipconfig.h"
#include "MainDlg.h"
#include "ConfigEdit.h"
#include "Winsock2.h"

#include <PvApi.h>

//===== #DEFINES ==============================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Maximum cameras in our list
#define MAX_CAMERA_LIST		100

// Listbox columns:
#define COLUMN_CAMERA		0
#define COLUMN_CONFIG		1
#define COLUMN_ADDRESS		2
#define COLUMN_SUBNET		3
#define COLUMN_GATEWAY		4
#define COLUMN_STATUS		5


//===== TYPE DEFINITIONS ======================================================

//===== FUNCTION PROTOTYPES ===================================================

//===== DATA (PUBLIC) =========================================================

//===== DATA (PRIVATE) ========================================================

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	//{{AFX_MSG_MAP(CMainDlg)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//===== IMPLEMENTATION ========================================================


CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDlg)
	//}}AFX_DATA_INIT
}


CMainDlg::~CMainDlg()
{
}


void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDlg)
	DDX_Control(pDX, IDC_LIST, m_ctlList);
	DDX_Control(pDX, IDC_CHANGE, m_ctlChange);
	//}}AFX_DATA_MAP
}


BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // Initialize the PV API
    if(PvInitialize() == ePvErrSuccess)
    {
	    m_ctlList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	    m_ctlList.InsertColumn(COLUMN_CAMERA, "Camera");
	    m_ctlList.InsertColumn(COLUMN_CONFIG, "Configuration Mode");
	    m_ctlList.InsertColumn(COLUMN_ADDRESS, "Address");
	    m_ctlList.InsertColumn(COLUMN_SUBNET, "Subnet Mask");
	    m_ctlList.InsertColumn(COLUMN_GATEWAY, "Gateway");
	    m_ctlList.InsertColumn(COLUMN_STATUS, "Status");

	    m_ctlList.SetColumnWidth(COLUMN_CAMERA, m_ctlList.GetStringWidth("02-XXXXA-XXXXX") * 125 / 100);
	    m_ctlList.SetColumnWidth(COLUMN_CONFIG, LVSCW_AUTOSIZE_USEHEADER);

	    const int columnWidth = m_ctlList.GetColumnWidth(COLUMN_CONFIG);

	    m_ctlList.SetColumnWidth(COLUMN_ADDRESS, columnWidth);
	    m_ctlList.SetColumnWidth(COLUMN_SUBNET, columnWidth);
	    m_ctlList.SetColumnWidth(COLUMN_GATEWAY, columnWidth);
	    m_ctlList.SetColumnWidth(COLUMN_STATUS, LVSCW_AUTOSIZE_USEHEADER);

	    
	    SetTimer(0, 2000, NULL);

	    refreshCameraList();

	    return TRUE;
    }
    else
        return FALSE;
}

void CMainDlg::OnOK()
{
    // let's not forget to uninitialize the API
    PvUnInitialize();

    CDialog::OnOK();
}

void CMainDlg::OnCancel()
{
    // let's not forget to uninitialize the API
    PvUnInitialize();

    CDialog::OnCancel();
}

void CMainDlg::OnChange() 
{
	CConfigEdit			editDlg;
	UINT32		        selectedUniqueId;
	tPvIpSettings		ipSettings;


	// Get the unique ID of the selected camera.
	{
		const int		item = m_ctlList.GetNextItem(-1, LVNI_SELECTED);

		selectedUniqueId = (item >= 0) ? (int)m_ctlList.GetItemData(item) : 0;
	}


	if (!selectedUniqueId)
		return;

	// Get the current configuration.

	if (PvCameraIpSettingsGet(selectedUniqueId, &ipSettings) == ePvErrSuccess)
	{
		// Run the dialog.

		editDlg.configMode		= ipSettings.ConfigMode;
		editDlg.persistAddress	= ntohl(ipSettings.PersistentIpAddr);
		editDlg.persistSubnet	= ntohl(ipSettings.PersistentIpSubnet);
		editDlg.persistGateway	= ntohl(ipSettings.PersistentIpGateway);

		if (editDlg.DoModal() == IDOK)
		{
			// The user wants to change the settings.

			ipSettings.ConfigMode			= editDlg.configMode;
			ipSettings.PersistentIpAddr		= htonl(editDlg.persistAddress);
			ipSettings.PersistentIpSubnet	= htonl(editDlg.persistSubnet);
			ipSettings.PersistentIpGateway	= htonl(editDlg.persistGateway);

			if (PvCameraIpSettingsChange(selectedUniqueId, &ipSettings) != ePvErrSuccess)
			{
				AfxMessageBox("Could not change the IP address", MB_OK | MB_ICONEXCLAMATION);
			}

		}

	}
	else
		AfxMessageBox("Could not communicate with the camera", MB_OK | MB_ICONEXCLAMATION);
	

	refreshCameraList();		
}


void CMainDlg::refreshCameraList()
{
	UINT32		selectedUniqueId;


	//
	// Get the unique ID of the selected camera.  We use this to restore the
	// selection after the list update.
	//

	{
		const int		item = m_ctlList.GetNextItem(-1, LVNI_SELECTED);

		selectedUniqueId = (item >= 0) ? (int)m_ctlList.GetItemData(item) : 0;
	}


	//
	// Update the camera list.
	//

	m_ctlList.DeleteAllItems();

	{
		tPvCameraInfo	cameraList[MAX_CAMERA_LIST];
		unsigned long	cameraNum;


		// First, get list of reachable cameras.

		cameraNum = PvCameraList(cameraList, MAX_CAMERA_LIST, NULL);

		// Append the list of unreachable cameras.

		if (cameraNum < MAX_CAMERA_LIST)
		{
			cameraNum += PvCameraListUnreachable(&cameraList[cameraNum],
												 MAX_CAMERA_LIST-cameraNum,
												 NULL);
		}

		// For each camera, get the IP configuration.

		for (unsigned long i = 0; i < cameraNum; i++)
		{
			tPvIpSettings	ipSettings;
			tPvErr			errcode;
			struct in_addr	address;
			CString			s;
			const int		item = m_ctlList.InsertItem(0, cameraList[i].SerialString);


			m_ctlList.SetItemData(item, cameraList[i].UniqueId);

			errcode =  PvCameraIpSettingsGet(cameraList[i].UniqueId, &ipSettings);

			if (errcode == ePvErrSuccess)
			{

                if (ipSettings.ConfigMode & ePvIpConfigPersistent)
                    s = "Fixed";
                else if (ipSettings.ConfigMode & ePvIpConfigDhcp)
	                s = "DHCP";
                else
                    s = "AutoIP";

				m_ctlList.SetItemText(item, COLUMN_CONFIG, s);

				address.S_un.S_addr = ipSettings.CurrentIpAddress;
				m_ctlList.SetItemText(item, COLUMN_ADDRESS, inet_ntoa(address));

				address.S_un.S_addr = ipSettings.CurrentIpSubnet;
				m_ctlList.SetItemText(item, COLUMN_SUBNET, inet_ntoa(address));

				address.S_un.S_addr = ipSettings.CurrentIpGateway;
				m_ctlList.SetItemText(item, COLUMN_GATEWAY, inet_ntoa(address));

			}
			else if (errcode == ePvErrUnavailable)
			{
				m_ctlList.SetItemText(item, COLUMN_STATUS, "Feature not available on this camera.");
			}
			else
				m_ctlList.SetItemText(item, COLUMN_STATUS, "Camera unavailable.");

		}

	}


	//
	// Re-select the previously selected camera.
	//

	if (selectedUniqueId)
	{
		for (int item = 0; item < m_ctlList.GetItemCount(); item++)
		{
			if (m_ctlList.GetItemData(item) == selectedUniqueId)
			{
				m_ctlList.SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
				break;
			}
		}
	}

}

#ifdef WIN64
void CMainDlg::OnTimer(UINT_PTR nIDEvent) 
#else
void CMainDlg::OnTimer(UINT nIDEvent) 
#endif
{
	refreshCameraList();
}
