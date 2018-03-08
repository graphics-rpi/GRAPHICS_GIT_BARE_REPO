/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        AboutDlg.h		
|
| Project/lib:	SampleViewer
|
| Target:		
|
| Description:  Define the aboutbox dialog class	
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

#ifndef _ABOUTDLG_INCLUDED_
#define _ABOUTDLG_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== INCLUDE FILES =========================================================

#include "stdafx.h"
#include "resource.h"       // main symbols

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CAboutDlg
 * Purpose:  Derive the standard MFC dialog to implement the about box of the
 *           application
 * Comments: none
 */
class CAboutDlg : public CDialog
{
    public:

        /*
         * Method:    CAboutDlg()
         * Purpose:   class constructor
         * Arguments:
         *
         * [i] CWnd* pParentWnd, parent window
         *
         * Return:    none
         * Comments:  none
         */
	    CAboutDlg(CWnd* pParentWnd = NULL);

	protected: // from CDialog

        /*
         * Method:    DoDataExchange()
         * Purpose:   called by the framework to validate the contents of
         *            the dialog
         * Arguments:
         *
         * [i] CDataExchange* pDX, context information
         *
         * Return:    none
         * Comments:  none
         */
	    void DoDataExchange(CDataExchange* pDX);

    protected:

        // declare the message map of the dialog
	    DECLARE_MESSAGE_MAP()

    public: // data

        // id of the resource
        enum { IDD = IDD_ABOUTBOX };
};

#endif // _ABOUTDLG_INCLUDED_
