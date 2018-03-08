/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        SerialFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the RS232 SerialIO window
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

#ifndef _SERIALFRAME_INCLUDED_
#define _SERIALFRAME_INCLUDED_

#include "ChildFrame.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

class CSerialView;

/*
 * Class:    CSerialFrame
 * Purpose:  Derive our MDI child class for the Histogram window
 * Comments: none
 */
class CSerialFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CSerialFrame)

    public:

        /*
         * Method:    CSerialFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CSerialFrame();

        /*
         * Method:    ~CSerialFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CSerialFrame();

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

    public: // change CBs

        afx_msg void OnRateChange(unsigned int Value);
        afx_msg void OnParityChange(unsigned int Value);
        afx_msg void OnCharLenChange(unsigned int Value);
        afx_msg void OnStopBitChange(unsigned int Value);

        afx_msg void OnSendClicked();
        afx_msg void OnConnectClicked();
        afx_msg void OnDisconnectClicked();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:

        CSerialView*    m_View;
        tPvHandle       m_Handle;
        UINT_PTR        m_Timer;
        char            m_String[512];
        unsigned int    m_Index;

    friend class CMainFrame;

};

#endif // _SERIALFRAME_INCLUDED_
