/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ChildFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define our MDI child class	
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

#ifndef _CHILDFRAME_INCLUDED_
#define _CHILDFRAME_INCLUDED_

#include "stdafx.h"
#include <PvAPI.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== TYPE  DEFINITION ======================================================

// command structure used
typedef struct {

    DWORD Uid;      // camera id
    DWORD Cid;      // command id
    DWORD Arg[4];   // arguments
    void* Ptr;      // pointer

} tCommand;

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CChildFrame
 * Purpose:  Derive the standard MFC child window to create the base class of all
 *           the camera related window
 * Comments: none
 */
class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)

    public: // type

        // Child type
        enum TChildType {
            
            eCtrl,
            eCtrlRO,
            eView,
            eInfo,
            eHist,
            eEvents,
            eSerial

        };

    public:

        /*
         * Method:    CChildFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CChildFrame();

        /*
         * Method:    ~CChildFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CChildFrame();

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments:
         *
         * [i] tPvCameraInfoEx* pInfo,  info to be displayed 
         * [i] TChildType Type,         type of the frame been created
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(tPvCameraInfoEx* pInfo,TChildType Type);

        /*
         * Method:    Show()
         * Purpose:   show the frame on screen
         * Arguments: 
         *
         * [i] HICON hIcon, icon to be used by the frame
         * 
         * Return:    none
         * Comments:  none
         */ 
        void Show(HICON hIcon);

        /*
         * Method:    Close()
         * Purpose:   close the frame
         * Arguments: none
         * Return:    none
         * Comments:  the frame will be destroyed
         */
        void Close();

        /*
         * Method:    SendCommandTo()
         * Purpose:   send a command to another child window
         * Arguments: 
         *
         * [i] TChildType aTarget, target
         * [i] tCommand* rCommand, command to be sent
         *
         * Return:    none
         * Comments:  none
         */
        void SendCommandTo(TChildType aTarget,tCommand* pCommand);

        /*
         * Method:    GetHandleToCamera()
         * Purpose:   get the handle to the camera
         * Arguments: none
         * Return:    the camera handle
         * Comments:  none
         */
        tPvHandle GetHandleToCamera();

    protected: // new

        /*
         * Method:    Block()
         * Purpose:   block the frame
         * Arguments: none
         * Return:    none
         * Comments:  it will not be closed until unblocked
         */
        void Block();

        /*
         * Method:    Unblock()
         * Purpose:   unblock the frame
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void Unblock();

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
         * Method:    DestroyIfRequested()
         * Purpose:   destroy the frame if it was requested
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void DestroyIfRequested();

        /*
         * Method:    OnShouldClose()
         * Purpose:   called when the window should be closed, but
         *            won't be closed right away as it is blocked
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        virtual void OnShouldClose() {};

        /*
         * Method:    OnCommandReceived()
         * Purpose:   called when the window has received a command
         * Arguments: 
         *
         * [i] tCommand* pCommand, received command
         *
         * Return:    none
         * Comments:  none
         */
        virtual void OnCommandReceived(const tCommand* pCommand) {};

        /*
         * Method:    ResizeToFit()
         * Purpose:   request the window to resize it-self to fit
         *            a given size
         * Arguments: 
         *
         * [i] UINT nWidth,     width
         * [i] UINT nHeight,    height
         *
         * Return:    none
         * Comments:  none
         */
        void ResizeToFit(UINT nWidth,UINT nHeight);

    protected: // from CMDIChildWnd

        /*
         * Method:    OnCreateClient()
         * Purpose:   called by the framework during the creation of
         *            the window. 
         * Arguments:
         *
         * [i] LPCREATESTRUCT lpcs ,        creation parameters
         * [i] CCreateContext* pContext,    creation context
         *
         * Return:    FALSE if an error occured
         * Comments:  none
         */
        BOOL OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext );

        /*
         * Method:    OnDestroy()
         * Purpose:   called by the framework when the window is about to
         *            be destroyed
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        virtual afx_msg void OnDestroy();

        /*
         * Method:    OnMDIActivate()
         * Purpose:   called by the framework when the window is been activated
         * Arguments: 
         *
         * [i] BOOL bActivate,      true when the window is been activated 
         * [i] CWnd* pActivateWnd,  window been activated
         * [i] CWnd* pDeactivateWnd window been deactivated
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnMDIActivate( BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd );


    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    protected:

        // pointer to the camera info
        tPvCameraInfoEx* m_Info;
        // type of this frame
        TChildType       m_Type;
        // camera model
        const char*      m_Model;
        // true when blocked (can't be destroyed)
        BOOL             m_Blocked;
        // true when the frame must destroy it-self
        BOOL             m_Destroy;

        friend class CMainFrame;
};

#endif // _CHILDFRAME_INCLUDED_
