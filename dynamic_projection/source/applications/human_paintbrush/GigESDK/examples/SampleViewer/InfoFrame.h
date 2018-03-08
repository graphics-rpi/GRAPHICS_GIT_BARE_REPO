/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        InfoFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the Info window
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

#ifndef _INFOFRAME_INCLUDED_
#define _INFOFRAME_INCLUDED_

#include "ChildFrame.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CInfoFrame
 * Purpose:  Derive our MDI child class for the Histogram window
 * Comments: none
 */
class CInfoFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CInfoFrame)

    public:

        /*
         * Method:    CInfoFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CInfoFrame();

        /*
         * Method:    ~CInfoFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CInfoFrame();

    public: // from CChildFrame

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments:
         *
         * [i] tPvCameraInfoEx* pInfo, info to be displayed 
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(tPvCameraInfoEx* pInfo);

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

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()
};

#endif // _INFOFRAME_INCLUDED_
