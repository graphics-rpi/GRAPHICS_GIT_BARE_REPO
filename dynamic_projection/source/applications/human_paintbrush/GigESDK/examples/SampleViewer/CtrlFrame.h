/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        CtrlFrame.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the camera controls window
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

#ifndef _CTRLFRAME_INCLUDED_
#define _CTRLFRAME_INCLUDED_

#include "ChildFrame.h"
#include "AttrTreeView.h"
#include "ContainerView.h"
#include "ValueView.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

class CFinderFrame; // forwardef

/*
 * Class:    CCtrlFrame
 * Purpose:  Derive our MDI child class for the control window
 * Comments: none
 */
class CCtrlFrame : public CChildFrame, public PAttrTreeViewObserver, public PValueObserver
{
	DECLARE_DYNCREATE(CCtrlFrame)

    public:

        /*
         * Method:    CCtrlFrame()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CCtrlFrame();

        /*
         * Method:    ~CCtrlFrame()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CCtrlFrame();

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments: 
         *
         * [i] tPvHandle Camera,        camera to control
         * [i] tPvCameraInfoEx* pInfo,  pointer to the camera's info
         * [i] bool bRW,                true if the controls shall be Read/Write
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(tPvHandle hCamera,tPvCameraInfoEx* pInfo,bool bRW);

        /*
         * Method:    SetLive()
         * Purpose:   indicate to the class that there is a 
         *            live view
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        void SetLive(CFinderFrame* pFrame);

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

    public: // new events

        /*
         * Method:    OnRefresh()
         * Purpose:   called when the streaming is started/stopped
         * Arguments: 
         *
         * [i] WPARAM wParam, first parameter  (none)      
         * [i] LPARAM lParam, second parameter (none)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg LRESULT OnRefresh(WPARAM wParam,LPARAM lParam);

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

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    protected: // from PAttrTreeViewObserver

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
        void AttrSelected(CString &Label);

    protected: // from PValueObserver

        /*
         * Method:    OnValueChanged()
         * Purpose:   called when the value of the attribute have changed
         * Arguments:
         *
         * [i] BOOL bValid, true when the change must be validated
         *
         * Return:    none
         * Comments:  none
         */
        void OnValueChanged(BOOL bValid);

    private:

        /*
         * Method:    OnPrepare()
         * Purpose:   called to finish off the creation of the frame
         * Arguments: none
         * Return:    false if error
         * Comments:  none
         */ 
        BOOL OnPrepare();

        /*
         * Method:    DoDiscovery()
         * Purpose:   discover all the attributes available on the camera
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        void DoDiscovery();

        /*
         * Method:    PreparePathToAttribute()
         * Purpose:   get (prepare if not existent) the path in the tree to
         *            an attribute
         * Arguments: 
         *
         * [i] const char* pPath, path to the attribute
         *
         * Return:    the last node on the path
         * Comments:  none
         */ 
        HTREEITEM PreparePathToAttribute(const char* pPath);

        /*
         * Method:    SeekChild()
         * Purpose:   seek a child in a parent
         * Arguments: 
         *
         * [i] HTREEITEM hRoot, parent
         * [i] const char* pLabel, label of the child to find
         *
         * Return:    NULL if not found
         * Comments:  none
         */ 
        HTREEITEM SeekChild(HTREEITEM hRoot,const char* pLabel);

        /*
         * Method:    SeekToLast()
         * Purpose:   seek to the last child on a path
         * Arguments: 
         *
         * [i] const char* pLabel,  label of the path
         * [i] bool bLeaf,          true if the method should seek the leaf    
         *
         * Return:    NULL if not found
         * Comments:  none
         */ 
        HTREEITEM SeekToLast(const char* pPath,bool bLeaf = false);

        /*
         * Method:    FormatAttributeValue()
         * Purpose:   format an attribute value in a string
         * Arguments: 
         *
         * [i] const char* pLabel,      label of the attribute
         * [i] tPvAttributeInfo &Info,  attribute's info
         * [i] CString& String,         string to write into
         *
         * Return:    true if no error
         * Comments:  none
         */ 
        BOOL FormatAttributeValue(const char* pLabel,tPvAttributeInfo &Info,CString& String);

        /*
         * Method:    RefreshImpacts()
         * Purpose:   refresh all the attributes which paths are
         *            listed in the given string
         * Arguments: 
         *
         * [i] const char* pImpacts, list of impacted paths
         *
         * Return:    none
         * Comments:  none
         */ 
        void RefreshImpacts(const char* pImpacts);

        /*
         * Method:    RefreshPath()
         * Purpose:   refresh all the attributes of a given
         *            path
         * Arguments: 
         *
         * [i] CString& Path, path
         *
         * Return:    none
         * Comments:  none
         */ 
        void RefreshPath(CString& Path);

        /*
         * Method:    DoesImpactTriggerRestart()
         * Purpose:   check if the impact refresh will requiere a restart of the streaming
         * Arguments: 
         *
         * [i] const char* pImpacts, list of impacted paths
         *
         * Return:    true if the streaming must be restarted
         * Comments:  none
         */ 
        bool DoesImpactTriggerRestart(const char* pImpacts);

        /*
         * Method:    DoesPathTriggerRestart()
         * Purpose:   check if the path refresh will requiere a restart of the streaming
         * Arguments: 
         *
         * [i] CString& Path, path
         *
         * Return:    true if the streaming must be restarted
         * Comments:  none
         */ 
        bool DoesPathTriggerRestart(CString& Path);

        /*
         * Method:    RefreshChilds()
         * Purpose:   refresh all the childs of a given tree item
         * Arguments: 
         *
         * [i] HTREEITEM hParent, parent item
         * [i] ULONG nFlags,      flag to be tested against the attr. flags    
         *
         * Return:    none
         * Comments:  if nFlags != 0, only the childs that have the flags will
         *            be refreshed
         */ 
        void RefreshChilds(HTREEITEM hParent,ULONG nFlags = 0);

        /*
         * Method:    RefreshChild()
         * Purpose:   refresh all the childs of a given tree item
         * Arguments: 
         *
         * [i] HTREEITEM hChild, child
         * [i] ULONG nFlags,     flag to be tested against the attr. flags    
         *
         * Return:    none
         * Comments:  if nFlags != 0, the child will be refreshed only if
         *            it have the requested flag(s)
         */ 
        void RefreshChild(HTREEITEM hChild,ULONG nFlags = 0);

        /*
         * Method:    RefreshCurrentForm()
         * Purpose:   refresh all the fields of the currently selected attribute
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        void RefreshCurrentForm();

    private: // data

        // handle to the camera to control
        tPvHandle        m_Camera;
        // attributes tree view
        CAttrTreeView*   m_TView;
        // attributes tree control
        CTreeCtrl*       m_Tree;
        // root item (in the tree widget)
        HTREEITEM        m_Root;
        // minimum size of the window
        ULONG            m_MinWidth;
        ULONG            m_MinHeight;

        // attribute currently selected
        CString          m_Attr;
        tPvAttributeInfo m_Info;

        // Value container
        CContainerView*  m_Container;
        CValueView*      m_Form;

        // live window
        CFinderFrame*    m_Live;
        // true when the live stream need to be restarted
        BOOL             m_NeedRestart;

        // timer for refreshing volatile attribute
        UINT_PTR         m_Timer;

        // true when the camera won't allow the write attribute
        BOOL             m_ReadOnly;
};

#endif // _CTRLFRAME_INCLUDED_
