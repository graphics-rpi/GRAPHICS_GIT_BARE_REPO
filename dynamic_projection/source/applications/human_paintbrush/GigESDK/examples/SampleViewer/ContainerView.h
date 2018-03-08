/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ContainerView.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define an empty container view
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

#ifndef _CONTAINERVIEW_INCLUDED_
#define _CONTAINERVIEW_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CContainerView
 * Purpose:  Derive our MDI child class for the Histogram window
 * Comments: none
 */
class CContainerView : public CView
{
	DECLARE_DYNCREATE(CContainerView)

    public:

        /*
         * Method:    CContainerView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CContainerView();

        /*
         * Method:    ~CContainerView()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CContainerView();

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments: 
         *
         * [i] ULONG nWidth,     initial width of the view
         * [i] ULONG nHeight,    initial height of the view
         * [i] CWnd* pParentWnd, parent
         * [i] UINT nID,         ID of the view
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(ULONG nWidth,ULONG nHeight,CWnd* pParentWnd,UINT nID);

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

        /*
         * Method:    OnDraw()
         * Purpose:   called by the framework when the client area of
         *            the window must be redraw
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnDraw( CDC* pDC );

        /*
         * Method:    OnEraseBkgnd()
         * Purpose:   called by the framework when the client area of
         *            the window should be erased
         * Arguments: 
         *
         * [i] CDC* pDC, device-context object
         *
         * Return:    TRUE if the background was erased
         * Comments:  none
         */
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()
};

#endif // _CONTAINERVIEW_INCLUDED_
