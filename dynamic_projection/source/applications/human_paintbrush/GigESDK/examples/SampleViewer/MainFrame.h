/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        MainFrm.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the main frame class	
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

#ifndef _MAINFRM_INCLUDED_
#define _MAINFRM_INCLUDED_

#pragma warning(disable: 4786)

#include <map>
#include <PvApi.h>

#include "FinderFrame.h"
#include "InfoFrame.h"
#include "HistFrame.h"
#include "CtrlFrame.h"
#include "ListFrame.h"
#include "EventsFrame.h"
#include "SerialFrame.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== TYPE DEFINITIONS ======================================================

// camera related data stored in the cameras map own by the main frame
typedef struct tCameraData
{
    tPvHandle       hCamera;    // camera handle
    tPvCameraInfoEx sInfo;      // camera's info
    bool            bOwner;     // true when we are the master of the camera
    CFinderFrame*   pLive;      // live window
    CInfoFrame*     pInfo;      // info window
    CHistFrame*     pHist;      // hist window
    CCtrlFrame*     pCtrl;      // ctrl window
    CEventsFrame*   pEvnt;      // evnt window
    CSerialFrame*   pSrIO;      // serial window

} tCameraData;

// cameras map
typedef std::map<tPvUint32,tCameraData > tCameraMap;

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CMainFrame
 * Purpose:  Derive the standard MDI frame to implement the main window of the
 *           application
 * Comments: none
 */
class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)

    public:

        /*
         * Method:    CMainFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CMainFrame();

        /*
         * Method:    ~CMainFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        virtual ~CMainFrame();

        /*
         * Method:    Show()
         * Purpose:   show the window
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void Show();

        /*
         * Method:    IsThereAStream()
         * Purpose:   check is there is an ongoing streaming
         * Arguments: none
         * Return:    true or false
         * Comments:  none
         */
        BOOL IsThereAStream() const;

        /*
         * Method:    GrabNextFrame()
         * Purpose:   instruct the current stream to "grab" the next frame
         * Arguments: 
         *
         * [i] bool Setup, false if we are to stop grabing the next frame, true otherwise
         *
         * Return:    none
         * Comments:  none
         */
        void GrabNextFrame(bool Setup);

        /*
         * Method:    IsGrabNextFrame()
         * Purpose:   check if current stream is grabbing the next frame
         * Arguments: none
         * Return:    true or false
         * Comments:  none
         */
        BOOL IsGrabNextFrame();

    public: // from CMDIFrameWnd

        /*
         * Method:    PreCreateWindow()
         * Purpose:   called by the framework prior to the creation of
         *            the window. 
         * Arguments:
         *
         * [b] CREATESTRUCT& cs, creation parameters
         *
         * Return:    FALSE if an error occured
         * Comments:  Can be used to alter to window size and position
         */
	    BOOL PreCreateWindow(CREATESTRUCT& cs);

        /*
         * Method:    OnCreate()
         * Purpose:   called by the framework when the frame is created 
         * Arguments: 
         *
         * [i] LPCREATESTRUCT lpCreateStruct, creation parameters
         *
         * Return:    -1 if the creation is to fail, 0 otherwise
         * Comments:  none
         */
        afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );

        /*
         * Method:    OnClose()
         * Purpose:   called by the framework prior to closing the window
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnClose();

        /*
         * Method:    OnDestroy()
         * Purpose:   called by the framework prior to the destruction of
         *            the window. 
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnDestroy();

        /*
         * Method:    OnQueryEndSession()
         * Purpose:   called by the framework when the user session is ending  (restart or shutdown)
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg BOOL OnQueryEndSession();

    public: // events handling

        /*
         * Method:    OnCameraAction()
         * Purpose:   handle an action on a given camera
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter  (camera UID)      
         * [i] LPARAM lParam, second parameter (action ID)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnCameraAction(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnCameraRemoval()
         * Purpose:   handle the removal of a camera
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter  (camera UID)      
         * [i] LPARAM lParam, second parameter (not used)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnCameraRemoval(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnChildEvent()
         * Purpose:   handle an event from a child window
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter        
         * [i] LPARAM lParam, second parameter
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnChildEvent(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnErrorReport()
         * Purpose:   handle a request to report an error
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter (error id)        
         * [i] LPARAM lParam, second parameter (error string)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnErrorReport(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnWarningReport()
         * Purpose:   handle a request to report a warning
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter (error id)        
         * [i] LPARAM lParam, second parameter (error string)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnWarningReport(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnCommandForwarding()
         * Purpose:   handle a request to forward a command
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter (command id)        
         * [i] LPARAM lParam, second parameter (command struct)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnCommandForwarding(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnQueryHandle()
         * Purpose:   handle a query for the camera
         * Arguments:
         *
         * [i] WPARAM wParam, first parameter (command id)        
         * [i] LPARAM lParam, second parameter (command struct)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnQueryHandle(WPARAM wParam,LPARAM lParam);

    private: // new

        /*
         * Method:    DoShowInfo()
         * Purpose:   display the Info window for a given camera
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowInfo(UINT lUID);

        /*
         * Method:    DoShowCtrl()
         * Purpose:   display the Control window for a given camera
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowCtrl(UINT lUID);

        /*
         * Method:    DoShowLive()
         * Purpose:   display the Live window for a given camera
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowLive(UINT lUID);

        /*
         * Method:    DoShowHist()
         * Purpose:   display the Histogram window for a given camera
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowHist(UINT lUID);

        /*
         * Method:    DoShowExpo()
         * Purpose:   export the camera setup to a text file
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowExpo(UINT lUID);

        /*
         * Method:    DoShowEvnt()
         * Purpose:   display the events window
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowEvnt(UINT lUID);

        /*
         * Method:    DoShowSrIO()
         * Purpose:   display the serial window
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */
        void DoShowSrIO(UINT lUID);

        /*
         * Method:    DoCameraOpen()
         * Purpose:   open a given camera for control&imaging
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    and error code, ePvErrSuccess if none
         * Comments:  none
         */
        tPvErr DoCameraOpen(UINT lUID);

        /*
         * Method:    DoCameraClose()
         * Purpose:   close a given camera (previously open)
         * Arguments:
         *
         * [i] UINT lUID, UID of the camera
         *
         * Return:    and error code, ePvErrSuccess if none
         * Comments:  none
         */
        tPvErr DoCameraClose(UINT lUID);

        /*
         * Method:    DoCameraExport()
         * Purpose:   export the setup of a given camera
         * Arguments:
         *
         * [i] UINT lUID,               UID of the camera
         * [i] const char* Filename,    name and path of the text file to export to
         *
         * Return:    true if succeed
         * Comments:  none
         */
        BOOL DoCameraExport(UINT lUID,const char* Filename);

        /*
         * Method:    ReportError()
         * Purpose:   report an error to the user
         * Arguments:
         *
         * [i] const char* Text, string to be displayed      
         * [i] tPvErr Error,     PvAPI error code (if any)
         *
         * Return:    none
         * Comments:  none
         */
        void ReportError(const char* Text,tPvErr Error = ePvErrSuccess);

        /*
         * Method:    ReportWarning()
         * Purpose:   report a warning to the user
         * Arguments:
         *
         * [i] const char* Text, string to be displayed      
         *
         * Return:    none
         * Comments:  none
         */
        void ReportWarning(const char* Text);

        /*
         * Method:    ReportLost()
         * Purpose:   report a lost of connection
         * Arguments:
         *
         * [i] const char* aSerial, serial string of the camera     
         *
         * Return:    none
         * Comments:  none
         */
        void ReportLost(const char* aSerial);
	   
    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private: // data

        // map of the open camera
        tCameraMap m_Cameras;

        // icons used for the child windows
        HICON      m_IconLive;
        HICON      m_IconInfo;
        HICON      m_IconCtrl;
        HICON      m_IconHist;
        HICON      m_IconEvnt;
        HICON      m_IconSrIO;
        // icon used for the main window
        HICON      m_IconAppl;

        // status bar
        CStatusBar m_StatusBar;

        // list frame
        CListFrame* m_ListFrame;

        // UID of the curretly selected camera
        tPvUint32   m_Current;
};

#endif // _MAINFRM_INCLUDED_
