/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        SampleViewer.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the Application class	
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

#ifndef _SAMPLEVIEWERAPP_INCLUDED_
#define _SAMPLEVIEWERAPP_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== INCLUDE FILES =========================================================

#include "resource.h"       // main symbols

//===== CLASS DEFINITION ======================================================

class CMainFrame;   // forward def
class CSaveNextDlg; // forward def 

/*
 * Class:    CSampleViewerApp
 * Purpose:  Derive the standard MFC application class to add our own specific
 *           code/functionnalities
 * Comments: none
 */
class CSampleViewerApp : public CWinApp
{
    public:

        /*
         * Method:    CSampleViewerApp()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CSampleViewerApp();

        /*
         * Method:    ~CSampleViewerApp()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        ~CSampleViewerApp();        

    public: // new

        /*
         * Method:    OnAppAbout()
         * Purpose:   display an about box for the application upon request
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnAppAbout();

        /*
         * Method:    OnSaveNextFrame()
         * Purpose:   display the save next frame dialog
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnSaveNextFrame();

	public: // from CWinApp

        /*
         * Method:    InitInstance()
         * Purpose:   initialise the class's instance
         * Arguments: none
         * Return:    TRUE if success, FALSE otherwise
         * Comments:  none
         */
	    BOOL InitInstance();

        /*
         * Method:    ExitInstance()
         * Purpose:   called when the application terminate
         * Arguments: none
         * Return:    0 if no error
         * Comments:  none
         */
        int ExitInstance();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:

        CMainFrame*   m_MainFrame;
        CSaveNextDlg* m_SaveNext;
};

#endif // _SAMPLEVIEWERAPP_INCLUDED_
