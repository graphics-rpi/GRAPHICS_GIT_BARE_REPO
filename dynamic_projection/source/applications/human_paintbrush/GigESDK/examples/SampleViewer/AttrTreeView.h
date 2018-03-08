/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        AttrTreeView.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the Camera attributes tree view	
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

#ifndef _ATTRTREEVIEW_H_INCLUDE
#define _ATTRTREEVIEW_H_INCLUDE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== INCLUDE FILES =========================================================

#include "ColumnTreeView.h"

//===== CLASS DEFINITION ======================================================

/*
 * Class:    PAttrTreeViewObserver
 * Purpose:  Prototype for a tree item selection observer
 * Comments: none
 */
class PAttrTreeViewObserver
{
    public:

        /*
         * Method:    ~PAttrTreeViewObserver()
         * Purpose:   destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        virtual ~PAttrTreeViewObserver() {};

    protected:

        /*
         * Method:    AttrSelected()
         * Purpose:   called when an attribute is selected by the user
         * Arguments: 
         *
         * [i] CString &Label, label of the selected attribute
         *
         * Return:    none
         * Comments:  none
         */
        virtual void AttrSelected(CString &Label) = 0;

    friend class CAttrTreeView;

};

/*
 * Class:    CAttrTreeView
 * Purpose:  Attribute tree view
 * Comments: none
 */
class CAttrTreeView : public CColumnTreeView
{
    DECLARE_DYNCREATE(CAttrTreeView)

    public:
        
        /*
         * Method:    CAttrTreeView()
         * Purpose:   constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        CAttrTreeView();

        /*
         * Method:    ~CAttrTreeView()
         * Purpose:   destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        ~CAttrTreeView();

        /*
         * Method:    Create()
         * Purpose:   create the view
         * Arguments: none
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(PAttrTreeViewObserver* pObserver,ULONG nWidth,ULONG nHeight,CWnd* pParentWnd,UINT nID);

    protected: // from CView

        /*
         * Method:    OnCreateClient()
         * Purpose:   called by the framework during the creation of
         *            the view 
         * Arguments:
         *
         * [i] LPCREATESTRUCT lpcs ,        creation parameters
         * [i] CCreateContext* pContext,    creation context
         *
         * Return:    FALSE if an error occured
         * Comments:  none
         */
        BOOL OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext );

    protected:

        /*
         * Method:    OnItemSelection()
         * Purpose:   called when a item from the tree was selected
         * Arguments: 
         *
         * [i] NMHDR *pNMHDR,    
         * [i] LRESULT *pResult,
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        afx_msg void OnItemSelection(NMHDR *pNMHDR, LRESULT *pResult);

        /*
         * Method:    OnSize()
         * Purpose:   called by the framework when the view was resized
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

    protected:

        DECLARE_MESSAGE_MAP()

    private:

        // observer that will be called when an attribute is selected
        PAttrTreeViewObserver* m_Observer;

};

#endif // _ATTRTREEVIEW_H_INCLUDE
