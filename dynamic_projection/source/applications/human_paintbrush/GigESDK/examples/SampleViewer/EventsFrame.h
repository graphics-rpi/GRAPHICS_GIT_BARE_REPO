/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        EventsFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the Events window
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

#ifndef _EVENTSFRAME_INCLUDED_
#define _EVENTSFRAME_INCLUDED_

#include "ChildFrame.h"
#include <deque>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

class CEventsView;

/*
 * Class:    CEventsFrame
 * Purpose:  Derive our MDI child class for the Histogram window
 * Comments: none
 */
class CEventsFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CEventsFrame)

    public:

        /*
         * Method:    CEventsFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CEventsFrame();

        /*
         * Method:    ~CEventsFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CEventsFrame();

    public: // from CChildFrame

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments: none
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(tPvHandle aHandle,tPvCameraInfoEx* pInfo);

    protected:

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
        void OnCommandReceived(const tCommand* pCommand);

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

    protected: // from CChildView

        /*
         * Method:    OnDestroy()
         * Purpose:   called by the framework when the window is about to
         *            be destroyed
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnDestroy();

    public: // new

        /*
         * Method:    OnCameraEvent()
         * Purpose:   called when a camera's event was received
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (number of event)      
         * [i] LPARAM lParam, second parameter (tPvCameraEvent*)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnCameraEvent(WPARAM wParam,LPARAM lParam);

    public:

        void SetAutoScroll(bool Scroll) {m_Scroll = Scroll;};
        bool IsPaused() const {return m_Paused;};
        void Pause();
        void PushEvents(const tPvCameraEvent* pEvents,DWORD Count);

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:

        double                      m_Frequency;
        CEventsView*                m_View;
        tPvHandle                   m_Handle;
        CRITICAL_SECTION	        m_Lock;
        std::deque<tPvCameraEvent>  m_Events;
        bool                        m_Paused;
        bool                        m_Scroll;
        unsigned __int64            m_Stamp;
        bool                        m_Going;

    friend class CMainFrame;

};

#endif // _EVENTSFRAME_INCLUDED_
