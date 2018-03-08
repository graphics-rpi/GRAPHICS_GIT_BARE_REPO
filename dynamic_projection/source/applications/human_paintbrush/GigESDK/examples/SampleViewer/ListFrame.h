/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ListFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the camera listing frame	
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

#ifndef _LISTFRAME_INCLUDED_
#define _LISTFRAME_INCLUDED_

#include <PvAPI.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CListFrame
 * Purpose:  Derive the standard MDI child to implement the window displaying 
 *           the cameras list
 * Comments: none
 */
class CListFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CListFrame)

    public:

        /*
         * Method:    CListFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CListFrame();

        /*
         * Method:    ~CListFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CListFrame();

        /*
         * Method:    SelectCamera()
         * Purpose:   select a given camera
         * Arguments: 
         *
         * [i] ULONG nUID, UID of the camera
         *
         * Return:    none
         * Comments:  none
         */  
        void SelectCamera(ULONG nUID);

    public: // from CMDIListWnd

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

    protected: // on events

        /*
         * Method:    OnMDIActivate()
         * Purpose:   called by the framework when the window is (de)activated
         * Arguments: 
         *
         * [i] BOOL bActivate,       TRUE if the window is activated
         * [i] CWnd* pActivateWnd,   window been activated
         * [i] CWnd* pDeactivateWnd, window been de-activated
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnMDIActivate( BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd );

        /*
         * Method:    OnClose()
         * Purpose:   called by the framework when the window is about to
         *            be distroyed
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnDestroy();


        /*
         * Method:    OnNotify()
         * Purpose:   called by the framework when the window is been notified by one of its child
         * Arguments: 
         *
         * [i] WPARAM wParam,       ID of the child
         * [i] LPARAM lParam,       Pointer to a notification message (NMHDR) structure     
         * [i] LRESULT* pResult,    Pointer to an LRESULT variable in which to store the result code if the message is handled.
         *
         * Return:    none
         * Comments:  none
         */
        BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

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

    public:

        /*
         * Method:    OnCameraEvent()
         * Purpose:   handle a camera related event
         *
         * [i] WPARAM wParam, first parameter  (event type)      
         * [i] LPARAM lParam, second parameter (camera UID)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnCameraEvent(WPARAM wParam,LPARAM lParam);

        /*
         * Method:    OnShowInfo()
         * Purpose:   user requested the display of some info on
         *            the camera
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowInfo();

        /*
         * Method:    OnShowCtrl()
         * Purpose:   user requested the display of the camera
         *            control window
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowCtrl();

        /*
         * Method:    OnShowLive()
         * Purpose:   user requested the display of the camera
         *            live window
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowLive();

        /*
         * Method:    OnShowHist()
         * Purpose:   user requested the display of the camera
         *            histogram window
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowHist();

        /*
         * Method:    OnShowSeek()
         * Purpose:   user requested the display of the seek
         *            camera window
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowSeek();

        /*
         * Method:    OnShowExpo()
         * Purpose:   user requested to export the camera setup to
         *            a text file
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowExpo();

        /*
         * Method:    OnShowEvnt()
         * Purpose:   user requested to see the events for the camera
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowEvnt();

        /*
         * Method:    OnShowSrIO()
         * Purpose:   user requested to see the serial for the camera
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    afx_msg void OnShowSrIO();
       
    private:

        /*
         * Method:    DoCameraDiscovery()
         * Purpose:   discover all the cameras currently plugged to the host
         *            and add them to the tree
         * Arguments: none
         * Return:    none
         * Comments:  none
         */      
        void DoCameraDiscovery();

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

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:
        // tree widget
        CTreeCtrl*      m_Tree;
        // toolbar
        CToolBarCtrl*   m_Toolbar;
        // root item (in the tree widget)
        HTREEITEM       m_Root;
};

#endif // _ListFRM_INCLUDED_