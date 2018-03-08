/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        FinderFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the Live view window	
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

#ifndef _FINDERFRAME_INCLUDED_
#define _FINDERFRAME_INCLUDED_

#include "ChildFrame.h"
#include <stack>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

class CHistFrame;
class CImageView;

/*
 * Class:    CFinderFrame
 * Purpose:  Derive our MDI child class for the Live view window
 * Comments: none
 */
class CFinderFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CFinderFrame)

    public:

        /*
         * Method:    CFinderFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CFinderFrame();

        /*
         * Method:    ~CFinderFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CFinderFrame();

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments:
         *
         * [i] tPvHandle Camera,         camera to stream from
         * [i] tPvCameraInfoEx* pInfo,   pointer to the camera info 
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(tPvHandle hCamera,tPvCameraInfoEx* pInfo,int Width,int Height);

        /*
         * Method:    Start()
         * Purpose:   start streaming from the camera
         * Arguments: 
         *
         * [i] BOOL bNow, when true, the command will be processed right away
         *                and the call block until the streaming is started
         *
         * Return:    none
         * Comments:  none
         */ 
        void Start(BOOL bNow = false);

        /*
         * Method:    Stop()
         * Purpose:   stop streaming from the camera
         * Arguments:
         *
         * [i] BOOL bNow, when true, the command will be processed right away
         *                and the call block until the streaming is stoped
         *
         * Return:    none
         * Comments:  none
         */ 
        void Stop(BOOL bNow = false);

        /*
         * Method:    DoHistogram()
         * Purpose:   update (or not) the histogram window
         * Arguments: 
         *
         * [i] CMDIChildWnd* pWindow,   histogram window, if NULL the
         *                              live view will no longer update
         *                              the histogram
         *
         * Return:    none
         * Comments:  none
         */ 
        void DoHistogram(CHistFrame* pWindow);

        /*
         * Method:    IsClosing()
         * Purpose:   check if the window is closing
         * Arguments: none
         * Return:    true or false
         * Comments:  none
         */ 
        BOOL IsClosing();

        /*
         * Method:    RequestGrab()
         * Purpose:   request the next received frame to be grabbed
         * Arguments: 
         *
         * [i] bool Grab, true or false
         *
         * Return:    none
         * Comments:  none
         */ 
        void RequestGrab(bool Grab);

       /*
         * Method:    IsGrabing()
         * Purpose:   check if the next frame is to be grabbed
         * Arguments: none
         * Return:    true or false
         * Comments:  none
         */ 
        BOOL IsGrabbing() const {return m_Grab;};

		tPvFrame* getFrames(){return m_Frames;}

    public: // from CMDIChildWnd

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

    protected: // from CChildFrame

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
         * Method:    OnPaint()
         * Purpose:   called by the framework when the client area of
         *            the window must be redraw
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnPaint();

        /*
         * Method:    OnEraseBkgnd()
         * Purpose:   called by the framework when the client area of
         *            the window should be erased
         * Arguments: ignored
         * Return:    TRUE if the background was erased
         * Comments:  none
         */
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

        /*
         * Method:    OnSize()
         * Purpose:   called by the framework when the window was resized
         * Arguments: 
         *
         * [i] UINT nType,  type of the resizing
         * [i] int cx,      new width of the client area
         * [i] int cy,      new height of the client area
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnSize(UINT nType, int cx, int cy);

        /*
         * Method:    OnDestroy()
         * Purpose:   called by the framework when window is been
         *            destroyed
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnDestroy();

        /*
         * Method:    OnClose()
         * Purpose:   called by the framework when window is been
         *            closed
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnClose();

        /*
         * Method:    OnContextMenu()
         * Purpose:   called by the framework when the user right
         *            clicked the mouse on the window
         * Arguments: 
         *
         * [i] CWnd* pWnd, window on which the event occured
         * [i] CPoint pos, position of the mouse (screen coord.)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnContextMenu( CWnd* pWnd, CPoint pos );

        /*
         * Method:    OnGetMinMaxInfo()
         * Purpose:   called by the framework to retreive the min&max
         *            size of the window
         * Arguments: 
         *
         * [o] MINMAXINFO FAR* lpMMI, structure where to store the sizes 
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );

        /*
         * Method:    OnTimer()
         * Purpose:   called by the framework when a timer event
         *            occurs
         * Arguments: 
         *
         * [i] UINT uId, ID of the timer
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnTimer(UINT_PTR uId);

        void OnShouldClose();

    protected: // new events

        /*
         * Method:    OnStart()
         * Purpose:   Live must start
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (event type)      
         * [i] LPARAM lParam, second parameter (camera UID)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnStart(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnStop()
         * Purpose:   Live must stop
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (event type)      
         * [i] LPARAM lParam, second parameter (camera UID)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnStop(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnHist()
         * Purpose:   the histogram window is either to be updated
         *            or no longer
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (true or false)      
         * [i] LPARAM lParam, second parameter (the window)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnHist(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnError()
         * Purpose:   an error have occured, report it.
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (error code)      
         * [i] LPARAM lParam, second parameter (none)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnError(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnGrabbed()
         * Purpose:   a frame have been grabbed, we need to
         *            save it then re-enqueue it
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (frame)      
         * [i] LPARAM lParam, second parameter (none)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnGrabbed(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnContextCommand()
         * Purpose:   execute a command from the context menu
         * Arguments: 
         *
         * [i] UINT nID, Id of the command to execute
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnContextCommand(UINT nID);

    private:

        /*
         * Method:    HandleFrameDone()
         * Purpose:   handle a frame that have been captured
         * Arguments: 
         *
         * [i] tPvFrame* pFrame, frame to be processed
         *
         * Return:    none
         * Comments:  none
         */
        void HandleFrameDone(tPvFrame* pFrame);

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private: // data

        // popup-menu
        CMenu               m_Menu;
        CMenu               m_SubMenu;
        CMenu               m_SubMenu2;
        // image render view
        CImageView*         m_View;
        // size of the image
        ULONG               m_Width;
        ULONG               m_Height;

        // handle to the camera
        tPvHandle           m_Camera;
        // array of frames used for the Liveing
        tPvFrame*           m_Frames;
        // current bytes per frame
        tPvUint32           m_BytesPerFrame;
        // timer for rendering
        UINT                m_Timer;

        // histogram window
        CHistFrame*         m_Hist;

        // true if the window is been closed
        BOOL                m_Closing;
        // true when the size of the window have changed
        BOOL                m_Update;
        // true when streaming from the camera
        BOOL                m_Streaming;
        // lock mecanism for some of the data updated
        // by the API thread and by the application thread
        CRITICAL_SECTION	m_Protection;

        // true when a bandwidth issue was already reported
        BOOL                m_Bandwidth;
        // true when the next frame to be displayed must be grabed
        BOOL                m_Grab;       
        // last if the size context id used
        UINT                m_LastCtxId;
        // true when the streaming is started for the very first time
        BOOL                m_First;

        // used with GetTickCount()
        DWORD                   m_Was;
        DWORD                   m_Now;

        std::stack<tPvFrame* >  m_FrameStack;

    friend void __stdcall FrameDoneCB(tPvFrame* Frame);
};

#endif // _FINDERFRAME_INCLUDED_
