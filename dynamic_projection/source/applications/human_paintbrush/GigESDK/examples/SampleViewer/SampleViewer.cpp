/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        SampleViewer.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the application class	
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

//===== INCLUDE FILES =========================================================

#include "stdafx.h"
#include "SampleViewer.h"

#include "MainFrame.h"
#include "ListFrame.h"
#include "AboutDlg.h"
#include "SaveNextDlg.h"

#include <PvAPI.h>
#include <iostream>
using namespace std;
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

//===== DEFINES ===============================================================

BEGIN_MESSAGE_MAP(CSampleViewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_TOOLS_SAVENEXTFRAME, OnSaveNextFrame)
END_MESSAGE_MAP()

//===== GLOBAL DATA ===========================================================

// global application object (there's only one for the whole app)
CSampleViewerApp gTheApplication;

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CSampleViewerApp()
 * Purpose:   class constructor
 * Comments:  none
 */
CSampleViewerApp::CSampleViewerApp()
{
    m_MainFrame = NULL;
}

/*
 * Method:    ~CSampleViewerApp()
 * Purpose:   class destructor
 * Comments:  none
 */
CSampleViewerApp::~CSampleViewerApp()
{
}

/*
 * Method:    InitInstance()
 * Purpose:   initialise the class's instance
 * Comments:  none
 */
BOOL CSampleViewerApp::InitInstance()
{
    //_crtBreakAlloc = 47;

    // initialise the Prosilica API
    if(PvInitialize() == ePvErrSuccess)
    {    
        // request 3D look for the controls
	    Enable3dControls();			

	    // change the registry key under which our settings are stored.
	    SetRegistryKey(_T("Prosilica"));

        // load standard INI file options (including MRU)
	    //LoadStdProfileSettings();  

	    // create main MDI Frame window
	    m_MainFrame = new CMainFrame;

        if(m_MainFrame)
        {
	        if (!m_MainFrame->LoadFrame(IDR_MAINFRAME))
		        return FALSE;
	        m_pMainWnd = m_MainFrame;

	        // the main window has been initialized, so show and update it.
            m_MainFrame->Show();
        }
        else
            return FALSE;

	    return TRUE;
    }
    else
    {
        MessageBox(NULL,_T("The Prosilica API failed to be initialised!"),_T("Error!"),MB_OK);
        return FALSE;
    }
}

/*
 * Method:    ExitInstance()
 * Purpose:   called when the application terminate
 * Comments:  none
 */
int CSampleViewerApp::ExitInstance()
{
    if(m_SaveNext)
        delete m_SaveNext;

    // uninit the API
    PvUnInitialize();

    //_CrtDumpMemoryLeaks();

    // and call the parent's method
    return CWinApp::ExitInstance();
}

/*
 * Method:    OnAppAbout()
 * Purpose:   display an about box for the application upon request
 * Comments:  none
 */
void CSampleViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg(m_pMainWnd);
	aboutDlg.DoModal();
}



/*
 * Method:    OnSaveNextFrame()
 * Purpose:   display the save next frame dialog
 * Comments:  none
 */
void CSampleViewerApp::OnSaveNextFrame()
{
    if(m_SaveNext)
        if(!m_SaveNext->IsWindowVisible())
            delete m_SaveNext;  
        else
            return;

    if(m_MainFrame->IsThereAStream())
    {
        m_SaveNext = new CSaveNextDlg(m_MainFrame);

        if(m_SaveNext)
        {
            m_SaveNext->Create(IDD_SAVENEXTFRAME,m_MainFrame);
            m_SaveNext->ShowWindow(SW_SHOW);
        }
    }
    else
    {
        MessageBox(NULL,_T("You need to start streaming from one of the open camera to use this tool."),_T("Error!"),MB_OK);    
    }
}

