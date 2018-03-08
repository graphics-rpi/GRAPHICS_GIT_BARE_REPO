/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        CtrlFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the camera controls window	
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

//===== INCLUDE FILES =========================================================

#include "stdafx.h"
#include "SampleViewer.h"
#include "CtrlFrame.h"
#include "Controls.h"

#include "FinderFrame.h"

//===== CONSTANTS =============================================================

// initial size of the window
const UINT KWinIniWidth  = 400;
const UINT KWinIniHeight = 350;
const UINT KPadWidth     = 5;
const UINT KPadHeight    = 5;

// how often will the volatile attributes be refreshed (in ms)
const UINT KTimerDelayRW = 350;
// how often will the attributes be refreshed (in ms) when the camera is open is monitor
const UINT KTimerDelayRO = 1000;

const char* kBooleanRange = "False,True";
const char* kBooleanTrue  = "True";
const char* kBooleanFalse = "False";

//===== LOCAL DATA ============================================================

// register some events used internaly
static UINT uCtrlRefresh = ::RegisterWindowMessage(_T("CtrlRefresh"));

IMPLEMENT_DYNCREATE(CCtrlFrame, CChildFrame)

// message map
BEGIN_MESSAGE_MAP(CCtrlFrame, CChildFrame)
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(uCtrlRefresh,OnRefresh)
END_MESSAGE_MAP()

//===== CLASS DEFINITION ======================================================

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CCtrlFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CCtrlFrame::CCtrlFrame()
{
    m_TView         = NULL;
    m_Tree          = NULL;
    m_Container     = NULL;
    m_Form          = NULL;
    m_Live          = NULL;
    m_NeedRestart   = false;
}

/*
 * Method:    ~CCtrlFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CCtrlFrame::~CCtrlFrame()
{
}

/*
 * Method:    SetLive()
 * Purpose:   indicate to the class that there is a 
 *            live view
 * Arguments: none
 * Return:    none
 * Comments:  none
 */ 
void CCtrlFrame::SetLive(CFinderFrame* pFrame)
{
    // set the member variable
    m_Live = pFrame;
    // and post a message to trigger a refreshment
    PostMessage(uCtrlRefresh);
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CCtrlFrame::Create(tPvHandle hCamera,tPvCameraInfoEx* pInfo,bool bRW)
{
    BOOL result;

    m_Camera    = hCamera;
    m_ReadOnly  = !bRW;

    // then call the parent's method to create the window
    if((result = CChildFrame::Create(pInfo,( m_ReadOnly ? CChildFrame::eCtrlRO : CChildFrame::eCtrl))))
    {
        // set the timer
        if(m_ReadOnly)
            m_Timer = SetTimer(1,KTimerDelayRO,NULL);
        else
            m_Timer = SetTimer(1,KTimerDelayRW,NULL);
    }
    else
        result = FALSE;
    
    return result;
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CCtrlFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CChildFrame::PreCreateWindow(cs))
    {
        // remove a couple of flags from the window's style
        cs.style &= ~WS_MAXIMIZEBOX;
        cs.style |= WS_CLIPCHILDREN;
        return TRUE;
    }
    else
	    return FALSE;
}

/*
 * Method:    OnCreateClient()
 * Purpose:   called by the framework during the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CCtrlFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
    BOOL result;

    m_TView     = new CAttrTreeView;
    m_Container = new CContainerView;

    if(m_TView && m_Container)
    {
       if(m_ReadOnly)
        result = m_TView->Create(NULL,0,0,this,KCtrlFrameTree) &&
                 m_Container->Create(0,0,this,KCtrlFrameCntr);
       else
        result = m_TView->Create(this,0,0,this,KCtrlFrameTree) &&
                 m_Container->Create(0,0,this,KCtrlFrameCntr);

       if(result == false)
       {
            m_TView->DestroyWindow();
            m_Container->DestroyWindow();
       }  
       else
           result = OnPrepare();
    }
    else
    {
        delete m_TView;
        delete m_Container;
    }    

    return CChildFrame::OnCreateClient(lpcs,pContext);
}

/*
 * Method:    OnSize()
 * Purpose:   called by the framework when the window was resized
 * Comments:  none
 */
void CCtrlFrame::OnSize(UINT nType, int cx, int cy)
{
    m_TView->MoveWindow(0,0,cx-1,cy-1-30-1);
    m_Container->MoveWindow(0,cy-1-30,cx-1,cy-1);

    if(m_Form)
    {
        CRect rect;
        m_Container->GetClientRect(&rect);
        m_Form->MoveWindow(&rect);
    }

    CChildFrame::OnSize(nType, cx, cy);
}

/*
 * Method:    OnGetMinMaxInfo()
 * Purpose:   called by the framework to retreive the min&max
 *            size of the window
 * Comments:  none
 */
void CCtrlFrame::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
    lpMMI->ptMinTrackSize.x = KWinIniWidth; //m_MinWidth;
    lpMMI->ptMinTrackSize.y = KWinIniHeight; //m_MinHeight;
}

/*
 * Method:    OnEraseBkgnd()
 * Purpose:   called by the framework when the client area of
 *            the window should be erased
 * Comments:  none
 */
BOOL CCtrlFrame::OnEraseBkgnd(CDC* pDC)
{
	RECT				wndRect;
	CGdiObject*			pOldObject;

	GetClientRect(&wndRect);
	
    // simply fill the client area with the white color
	pOldObject = pDC->SelectStockObject(WHITE_BRUSH);
	pDC->PatBlt(0, 0, wndRect.right, wndRect.bottom, PATCOPY);
	pDC->SelectObject(pOldObject);

	return TRUE;
}

/*
 * Method:    OnTimer()
 * Purpose:   called by the framework when a timer event
 *            occurs
 * Comments:  none
 */
void CCtrlFrame::OnTimer(UINT_PTR uId)
{
    if(m_ReadOnly)
    {
        BeginWaitCursor(); 
        RefreshChilds(m_Root);
        EndWaitCursor(); 
    }
    else
        // refresh all the attributes that are volatile
        RefreshChilds(m_Root,ePvFlagVolatile);    
}

/*
 * Method:    OnDestroy()
 * Purpose:   called by the framework when the window is about to
 *            be destroyed
 * Arguments: none
 * Return:    none
 * Comments:  none
 */
void CCtrlFrame::OnDestroy()
{
    KillTimer(m_Timer);
    CChildFrame::OnDestroy();
}

/*
 * Method:    OnCommandReceived()
 * Purpose:   called when the window has received a command
 * Comments:  none
 */
void CCtrlFrame::OnCommandReceived(const tCommand* pCommand)
{
    if(pCommand->Cid == 0xA)
    {
        tPvAttributeInfo lInfo;

        if(PvAttrInfo(m_Camera,(const char*)pCommand->Ptr,&lInfo) == ePvErrSuccess)
            RefreshImpacts(lInfo.Impact);
    }
}

/*
 * Method:    OnPrepare()
 * Purpose:   called to finish off the creation of the frame
 * Comments:  none
 */ 
BOOL CCtrlFrame::OnPrepare()
{
    m_Tree = &m_TView->GetTreeCtrl();

    // get the root item
    m_Root = m_Tree->GetRootItem();

    // time to discovers the attributes
    DoDiscovery();

    ResizeToFit(KWinIniWidth,KWinIniHeight);

    // set the focus
    m_Tree->SetFocus();

    return true;
}

/*
 * Method:    OnRefresh()
 * Purpose:   refresh some of the attributes when the streaming is stopped/started
 * Comments:  none
 */
LRESULT CCtrlFrame::OnRefresh(WPARAM wParam,LPARAM lParam)
{
    // refresh of the Acquisition category attributes
    RefreshPath(CString("/Acquisition"));

    return true;
}

/*
 * Method:    AttrSelected()
 * Purpose:   called when an attribute is selected by the user
 * Comments:  none
 */
void CCtrlFrame::AttrSelected(CString &Label)
{
    CRect rect;
    tPvErr Err;

    m_Attr = Label;

    m_Container->GetClientRect(&rect);

    // destroy the previous control (if any)
    if(m_Form)
    {
        m_Form->DestroyWindow();
        m_Form = NULL;
    }

    if((Err = PvAttrInfo(m_Camera,(LPCTSTR)Label,&m_Info)) == ePvErrSuccess)
    {
        // is this attribute have impact on the image?
        CString Impact = m_Info.Impact;
        
        m_NeedRestart = (Label == "PacketSize") || DoesImpactTriggerRestart(m_Info.Impact);

        if((m_Info.Flags & ePvFlagWrite || m_Info.Datatype == ePvDatatypeCommand) &&
           (PvAttrIsAvailable(m_Camera,(LPCTSTR)Label) == ePvErrSuccess))
        {
            switch(m_Info.Datatype)
            {
                case ePvDatatypeEnum:
                {
                    CEnumValueView* pForm = new CEnumValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            char Range[512];
                            char Value[64]; 

                            // retreive the range
                            if(((Err = PvAttrRangeEnum(m_Camera,(LPCTSTR)Label,Range,512,NULL)) == ePvErrSuccess) &&
                               ((Err = PvAttrEnumGet(m_Camera,(LPCTSTR)Label,Value,64,NULL)) == ePvErrSuccess))
                            {
                               pForm->SetLabel(Label);
                               if(!pForm->SetRange(Range) || !pForm->SetValue(Value))
                                    pForm->DestroyWindow(); 
                               else
                                   m_Form = pForm;
                            }
                            else
                                pForm->DestroyWindow();
                        }
                        else
                            delete pForm;
                    }

                    break;
                }
                case ePvDatatypeBoolean:
                {
                    CEnumValueView* pForm = new CEnumValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            tPvBoolean Value;

                            // retreive the range
                            if(((Err = PvAttrBooleanGet(m_Camera,(LPCTSTR)Label,&Value)) == ePvErrSuccess))
                            {
                               pForm->SetLabel(Label);
                               if(!pForm->SetRange(kBooleanRange) ||
                                   !pForm->SetValue(Value ? kBooleanTrue : kBooleanFalse))
                                    pForm->DestroyWindow(); 
                               else
                                   m_Form = pForm;
                            }
                            else
                                pForm->DestroyWindow();
                        }
                        else
                            delete pForm;
                    }

                    break;
                }
                case ePvDatatypeUint32:
                {
                    CUintValueView* pForm = new CUintValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            tPvUint32 nValue = 0;
                            tPvUint32 nMin,nMax;

                            // retreive the range
                            if(((Err = PvAttrRangeUint32(m_Camera,(LPCTSTR)Label,&nMin,&nMax)) == ePvErrSuccess) &&
                               ((Err = PvAttrUint32Get(m_Camera,(LPCTSTR)Label,&nValue)) == ePvErrSuccess))
                            {
                               pForm->SetLabel(Label);
                               if(!pForm->SetRange(nMin,nMax) || !pForm->SetValue(nValue))
                                    pForm->DestroyWindow(); 
                               else
                                   m_Form = pForm;
                            }
                            else
                                pForm->DestroyWindow();
                        }
                        else
                            delete pForm;
                    }
                    break;
                }
                case ePvDatatypeInt64:
                {
                    CSintValueView* pForm = new CSintValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            tPvInt64 nValue = 0;
                            tPvInt64 nMin,nMax;

                            // retreive the range
                            if(((Err = PvAttrRangeInt64(m_Camera,(LPCTSTR)Label,&nMin,&nMax)) == ePvErrSuccess) &&
                               ((Err = PvAttrInt64Get(m_Camera,(LPCTSTR)Label,&nValue)) == ePvErrSuccess))
                            {
                               pForm->SetLabel(Label);
                               if(!pForm->SetRange(nMin,nMax) || !pForm->SetValue(nValue))
                                    pForm->DestroyWindow(); 
                               else
                                   m_Form = pForm;
                            }
                            else
                                pForm->DestroyWindow();
                        }
                        else
                            delete pForm;
                    }
                    break;
                }
                case ePvDatatypeFloat32:
                {
                    CFloatValueView* pForm = new CFloatValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            tPvFloat32 nValue;
                            tPvFloat32 nMin,nMax;

                            // retreive the range
                            if(((Err = PvAttrRangeFloat32(m_Camera,(LPCTSTR)Label,&nMin,&nMax)) == ePvErrSuccess) &&
                               ((Err = PvAttrFloat32Get(m_Camera,(LPCTSTR)Label,&nValue)) == ePvErrSuccess))
                            {
                               pForm->SetLabel(Label);
                               if(!pForm->SetRange(nMin,nMax) || !pForm->SetValue(nValue))
                                    pForm->DestroyWindow();
                               else
                                   m_Form = pForm;
                            }
                            else
                                pForm->DestroyWindow();
                        }
                        else
                            delete pForm;
                    }

                    break;
                }
                case ePvDatatypeCommand:
                {
                    CCmdValueView* pForm = new CCmdValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            m_Form = pForm;
                            pForm->SetLabel(Label);                                
                        }
                        else
                            delete pForm;
                    }
                    break;
                }
                case ePvDatatypeString:
                {
                    CStringValueView* pForm = new CStringValueView();

                    if(pForm)
                    {
                        if(pForm->Create(this,rect,m_Container))
                        {
                            char Value[256];
                            CString String;

                            // retreive the range
                            if(((Err = PvAttrStringGet(m_Camera,(LPCTSTR)Label,Value,256,NULL)) == ePvErrSuccess))
                            {
                               String = Value;

                               pForm->SetLabel(Label);
                               if(!pForm->SetValue(String))
                                    pForm->DestroyWindow(); 
                               else
                                   m_Form = pForm;
                            }
                            else
                                pForm->DestroyWindow();
                        }
                        else
                            delete pForm;
                    }

                    break;
                }
                default:
                    break;
            }

            if(!m_Form)
                ReportError("Failed to create attribute edition field");
        }
    }
    else
        ReportError("Failed to get info on the attribute",Err);
}

/*
 * Method:    OnValueChanged()
 * Purpose:   called when the value of the attribute have changed
 * Comments:  none
 */
void CCtrlFrame::OnValueChanged(BOOL bValid)
{
    if(bValid)
    {
        tPvErr Err = ePvErrSuccess;

        // if the impact of the attribute include /image, the
        // stream must be stopped then restarted once the change
        // was made.
        if(m_Live && m_NeedRestart)
            m_Live->Stop(true);

        // write the selected value 

        switch(m_Info.Datatype)
        {
            case ePvDatatypeEnum:
            {
                CEnumValueView* pForm = (CEnumValueView*)m_Form;

                if(pForm)
                {
                    CString Value;

                    if(pForm->GetValue(Value))
                    {
                        Err = PvAttrEnumSet(m_Camera,m_Attr,(LPCTSTR)Value);

                        if(Err == ePvErrSuccess)
                        {
                            // update the value displayed in the tree
                            HTREEITEM hItem = m_Tree->GetSelectedItem();

                            if(hItem)
                                RefreshChild(hItem);

                            RefreshImpacts(m_Info.Impact);
                        }
                        else
                        {
                            ReportError("Failed to write value",Err);
                        }
                    }
                    else
                    {
                        ReportError("Failed to get the new value",Err);
                    }
                }

                break;
            }
            case ePvDatatypeBoolean:
            {
                CEnumValueView* pForm = (CEnumValueView*)m_Form;

                if(pForm)
                {
                    CString Value;

                    if(pForm->GetValue(Value))
                    {
                        if(Value == kBooleanTrue)
                            Err = PvAttrBooleanSet(m_Camera,m_Attr,true);
                        else
                            Err = PvAttrBooleanSet(m_Camera,m_Attr,false);

                        if(Err == ePvErrSuccess)
                        {
                            // update the value displayed in the tree
                            HTREEITEM hItem = m_Tree->GetSelectedItem();

                            if(hItem)
                                RefreshChild(hItem);

                            RefreshImpacts(m_Info.Impact);
                        }
                        else
                        {
                            ReportError("Failed to write value",Err);
                        }
                    }
                    else
                    {
                        ReportError("Failed to get the new value",Err);
                    }
                }

                break;
            }
            case ePvDatatypeUint32:
            {
                CUintValueView* pForm = (CUintValueView*)m_Form;

                if(pForm)
                {
                    tPvUint32 nValue;

                    if(pForm->GetValue(nValue))
                    {
                        Err = PvAttrUint32Set(m_Camera,m_Attr,nValue);

                        if(Err == ePvErrSuccess)
                        {
                            // update the value displayed in the tree
                            HTREEITEM hItem = m_Tree->GetSelectedItem();

                            if(hItem)
                                RefreshChild(hItem);

                            RefreshImpacts(m_Info.Impact);
                        }
                        else
                        {
                            ReportError("Failed to write value",Err);
                        }
                    }
                    else
                    {
                        ReportError("Failed to get the new value",Err);
                    }
                }               

                break;
            }
            case ePvDatatypeInt64:
            {
                CSintValueView* pForm = (CSintValueView*)m_Form;

                if(pForm)
                {
                    tPvInt64 nValue;

                    if(pForm->GetValue(nValue))
                    {
                        Err = PvAttrInt64Set(m_Camera,m_Attr,nValue);

                        if(Err == ePvErrSuccess)
                        {
                            // update the value displayed in the tree
                            HTREEITEM hItem = m_Tree->GetSelectedItem();

                            if(hItem)
                                RefreshChild(hItem);

                            RefreshImpacts(m_Info.Impact);
                        }
                        else
                        {
                            ReportError("Failed to write value",Err);
                        }
                    }
                    else
                    {
                        ReportError("Failed to get the new value",Err);
                    }
                }               

                break;
            }
            case ePvDatatypeFloat32:
            {
                CFloatValueView* pForm = (CFloatValueView*)m_Form;

                if(pForm)
                {
                    tPvFloat32 nValue;

                    if(pForm->GetValue(nValue))
                    {
                        Err = PvAttrFloat32Set(m_Camera,m_Attr,nValue);

                        if(Err == ePvErrSuccess)
                        {
                            // update the value displayed in the tree
                            HTREEITEM hItem = m_Tree->GetSelectedItem();

                            if(hItem)
                                RefreshChild(hItem);

                            RefreshImpacts(m_Info.Impact);
                        }
                        else
                        {
                            ReportError("Failed to write value",Err);
                        }
                    }
                    else
                    {
                        ReportError("Failed to get the new value",Err);
                    }
                }               

                break;
            }
            case ePvDatatypeCommand:
            {
                tPvErr Err;

                Err = PvCommandRun(m_Camera,m_Attr);
                
                if(Err)
                    ReportError("Failed to run the command",Err);
                else
                    RefreshImpacts(m_Info.Impact);

                break;
            }
            case ePvDatatypeString:
            {
                CStringValueView* pForm = (CStringValueView*)m_Form;

                if(pForm)
                {
                    CString Value;

                    if(pForm->GetValue(Value))
                    {
                        Err = PvAttrStringSet(m_Camera,m_Attr,(LPCTSTR)Value);

                        if(Err == ePvErrSuccess)
                        {
                            // update the value displayed in the tree
                            HTREEITEM hItem = m_Tree->GetSelectedItem();

                            if(hItem)
                                RefreshChild(hItem);

                            RefreshImpacts(m_Info.Impact);
                        }
                        else
                        {
                            ReportError("Failed to write value",Err);
                        }
                    }
                    else
                    {
                        ReportError("Failed to get the new value",Err);
                    }
                }
                break;
            }
            default:
                break;
        }

        if(Err == ePvErrSuccess)
        {       
            Err = PvAttrIsValid(m_Camera,m_Attr);
            if(Err == ePvErrOutOfRange)
            {
                ReportWarning("Some side effects have made this attribute invalid.");    
            }
        }

        if(m_Live && m_NeedRestart)
            m_Live->Start(true);
    }
    else
    {
        // re-read the current value and update the form
        RefreshCurrentForm();
    }
}

/*
 * Method:    DoDiscovery()
 * Purpose:   discover all the attributes available on the camera
 * Comments:  none
 */ 
void CCtrlFrame::DoDiscovery()
{
    ULONG           nCount;
    tPvAttrListPtr  Attrs;
    tPvErr          Err;

    if((Err = PvAttrList(m_Camera,&Attrs,&nCount)) == ePvErrSuccess)
    {
        const char* pLabel;
        HTREEITEM hRoot;
        tPvAttributeInfo Info;

        for(tPvUint32 i=0;i<nCount;i++)
        {
           pLabel = Attrs[i];
           
           if(PvAttrInfo(m_Camera,pLabel,&Info) == ePvErrSuccess)
           {
               // the category of the attribute must be
               // be parsed and each needed node created 

               if((hRoot = PreparePathToAttribute(Info.Category)))
               {
                    TVINSERTSTRUCT item;
                    CString String;
                    CString Value;

                    if(FormatAttributeValue(pLabel,Info,Value))
                    {
                        String = pLabel;
                        String += "\t";
                        String += Value;

                        // fill the item
                        item.hParent       = hRoot;
                        item.hInsertAfter  = TVI_LAST;
                        item.item.mask     = TVIF_TEXT;
                        item.item.pszText  = const_cast<char *>((LPCTSTR)String); //(char *)pLabel;

                        // then insert it
                        m_Tree->InsertItem(&item);  
                    }
               }
           }               
        }

        // sort the items
        m_Tree->SortChildren(m_Root);

        // select the first item
        m_Tree->SelectItem(m_Tree->GetChildItem(m_Root));

        // make sure the root is expanded
        m_Tree->Expand(m_Root,TVE_EXPAND);

        // then expand all the direct childs
        HTREEITEM hChildItem = m_Tree->GetChildItem(m_Root);

        while (hChildItem != NULL)
        {
            m_Tree->Expand(hChildItem,TVE_EXPAND);
            hChildItem = m_Tree->GetNextItem(hChildItem, TVGN_NEXT);
        }  
        
        // then select the root
        m_Tree->SelectItem(m_Root);

    }
}

/*
 * Method:    PreparePathToAttribute()
 * Purpose:   get (prepare if not existent) the path in the tree to
 *            an attribute
 * Comments:  none
 */ 
HTREEITEM CCtrlFrame::PreparePathToAttribute(const char* pPath)
{
    CString Part;
    CString Path = pPath;
    int nPos = 0,nPrv = 0;
    HTREEITEM hParent = m_Root;
    HTREEITEM hChild;
    TVINSERTSTRUCT Item;

    Item.hInsertAfter  = TVI_LAST;
    Item.item.mask     = TVIF_TEXT;

    while(nPos < Path.GetLength())
    {
        nPrv = nPos + 1;
        nPos = Path.Find('/',nPrv);
        if(nPos == -1)
            nPos = Path.GetLength() - 1;
        else
            nPos--;

        Part = Path.Mid(nPrv,nPos - nPrv + 1);

        if(Part.GetLength())
        {
            hChild = SeekChild(hParent,Part);

            if(!hChild)
            {
                // fill the item
                Item.hParent       = hParent;
                Item.item.pszText  = const_cast<char *>((LPCTSTR)Part);

                // then insert it
                hParent = m_Tree->InsertItem(&Item);
            }
            else
                hParent = hChild;
        }

        nPos++;
    }

    return hParent;
}

/*
 * Method:    SeekChild()
 * Purpose:   seek a child in a parent
 * Comments:  none
 */
HTREEITEM CCtrlFrame::SeekChild(HTREEITEM hRoot,const char* pLabel)
{
    HTREEITEM hNextItem;
    HTREEITEM hChildItem = m_Tree->GetChildItem(hRoot);
    BOOL      bReselect  = FALSE;
    CString   strLabel;
    int       nPos;

    while (hChildItem != NULL)
    {
        hNextItem = m_Tree->GetNextItem(hChildItem, TVGN_NEXT);

        // get the attribute label
	    strLabel = m_Tree->GetItemText(hChildItem);
	    nPos = strLabel.Find('\t');
	    if (nPos >= 0)
		    strLabel = strLabel.Left(nPos);

        if(strLabel == pLabel)
            break;

        hChildItem = hNextItem;
    }   
    
    return hChildItem;
}

/*
 * Method:    FormatAttributeValue()
 * Purpose:   format an attribute value in a string
 * Comments:  none
 */ 
BOOL CCtrlFrame::FormatAttributeValue(const char* pLabel,tPvAttributeInfo &Info,CString& String)
{
    BOOL  bRet = true;

    /*
    if(PvAttrIsAvailable(m_Camera,pLabel))
    {
        String = "unavailable";
        return true;        
    }
    */

    // if the attribute cannot be read, we will display
    // a special string instead of it's value
    if(!(Info.Flags & ePvFlagRead))
    {
        String = "N/A";
        return true;
    }

    // else we will format the value according to the type of the attribute
    switch(Info.Datatype)
    {
        case ePvDatatypeUnknown:
        case ePvDatatypeCommand:
        case ePvDatatypeRaw:
            break;
        case ePvDatatypeString:
        {
            char Value[256];

            if(PvAttrStringGet(m_Camera,pLabel,Value,256,NULL) == ePvErrSuccess)
                String = Value;
            else
                bRet = false;

            break;
        }
        case ePvDatatypeEnum:
        {
            char Value[256];

            if(PvAttrEnumGet(m_Camera,pLabel,Value,256,NULL) == ePvErrSuccess)
                String = Value;
            else
                bRet = false;            

            break;
        }
        case ePvDatatypeBoolean:
        {
            tPvBoolean Value;

            if(PvAttrBooleanGet(m_Camera,pLabel,&Value) == ePvErrSuccess)
                String = Value ? kBooleanTrue : kBooleanFalse;
            else
                bRet = false;            

            break;
        }
        case ePvDatatypeUint32:
        {
            tPvUint32 nValue;

            if(PvAttrUint32Get(m_Camera,pLabel,&nValue) == ePvErrSuccess)
                String.Format("%u",nValue);
            else
                bRet = false;

            break;
        }
        case ePvDatatypeInt64:
        {
            tPvInt64 nValue;

            if(PvAttrInt64Get(m_Camera,pLabel,&nValue) == ePvErrSuccess)
                String.Format("%I64d",nValue);
            else
                bRet = false;

            break;
        }
        case ePvDatatypeFloat32:
        {
            tPvFloat32 fValue;

            if(PvAttrFloat32Get(m_Camera,pLabel,&fValue) == ePvErrSuccess)
                String.Format("%.3f",fValue);
            else
                bRet = false;
            break;
        }
    }

    if(!bRet)
    {
        String = "unavailable";
        bRet = true;
    }

    return bRet;
}

/*
 * Method:    RefreshImpacts()
 * Purpose:   refresh all the attributes which paths are
 *            listed in the given string
 * Comments:  none
 */ 
void CCtrlFrame::RefreshImpacts(const char* pImpacts)
{
    if(pImpacts)
    {
        int nIndex = 0;
        CString Impacts = pImpacts;
        int i,j = 0;

        // parse the impact string and refresh all the path
        // we find in it
        for(i = 0;i<Impacts.GetLength();i++)
        {
            if(Impacts[i] == ',')
            {
                RefreshPath(Impacts.Mid(j,i-j));
                j = i+1;
            }
        }

        if(j<i)
            RefreshPath(Impacts.Mid(j,i-j));
    }
}

/*
 * Method:    RefreshPath()
 * Purpose:   refresh all the attributes of a given
 *            path
 * Comments:  none
 */ 
void CCtrlFrame::RefreshPath(CString& Path)
{
    HTREEITEM hParent;

    // is it a path?
    if(Path.Find('/',0) == 0)
    {
        hParent = SeekToLast((LPCTSTR)Path);
        if(hParent)
            RefreshChilds(hParent);  
    }
    else // nope, we need to get the path
    {
        tPvAttributeInfo Info;

        if(PvAttrInfo(m_Camera,(LPCTSTR)Path,&Info) == ePvErrSuccess)
        {
            Path = CString(Info.Category) + "/" + Path;

            hParent = SeekToLast((LPCTSTR)Path,true);
            if(hParent)
                RefreshChilds(hParent);
        }
    }
}

/*
 * Method:    DoesImpactTriggerRestart()
 * Purpose:   check if the impact refresh will requiere a restart of the streaming
 * Comments:  none
 */ 
bool CCtrlFrame::DoesImpactTriggerRestart(const char* pImpacts)
{
    if(pImpacts)
    {
        int nIndex = 0;
        CString Impacts = pImpacts;
        int i,j = 0;
        bool lRestart = false;

        // parse the impact string and refresh all the path
        // we find in it
        for(i = 0;i<Impacts.GetLength();i++)
        {
            if(Impacts[i] == ',')
            {
                if(DoesPathTriggerRestart(Impacts.Mid(j,i-j)))
                    lRestart = true;
                j = i+1;
            }
        }

        if(j<i)
            if(DoesPathTriggerRestart(Impacts.Mid(j,i-j)))
                lRestart = true;

        return lRestart;
    }
    else
        return false;
}

/*
 * Method:    DoesPathTriggerRestart()
 * Purpose:   check if the path refresh will requiere a restart of the streaming
 * Comments:  none
 */ 
bool CCtrlFrame::DoesPathTriggerRestart(CString& Path)
{
    bool lRestart = false;

    // is it a path?
    if(Path.Find('/',0) == 0)
        lRestart = Path.Find(_T("/Image"),0) != -1 || Path == "/";
    else // nope, we need to get the path
    {
        if(Path == "PacketSize")
            lRestart = true;
        else
        {
            tPvAttributeInfo Info;

            if(PvAttrInfo(m_Camera,(LPCTSTR)Path,&Info) == ePvErrSuccess)
                lRestart = CString(Info.Category).Find(_T("/Image"),0) != -1;
        }
    }

    return lRestart;
}

/*
 * Method:    SeekToLast()
 * Purpose:   seek to the last child on the path
 * Comments:  none
 */ 
HTREEITEM CCtrlFrame::SeekToLast(const char* pPath,bool bLeaf /* = false */)
{
    CString Part;
    CString Path = pPath;
    int nPos = 0,nPrv = 0;
    HTREEITEM hChild;
    HTREEITEM hParent = m_Root;

    while(nPos < Path.GetLength())
    {
        nPrv = nPos + 1;
        nPos = Path.Find('/',nPrv);
        if(nPos == -1)
            nPos = Path.GetLength() - 1;
        else
            nPos--;

        Part = Path.Mid(nPrv,nPos - nPrv + 1);

        if(Part.GetLength())
        {
            hChild = SeekChild(hParent,Part);
            if(hChild || bLeaf)
                hParent = hChild;
        }

        nPos++;
    }

    return hParent;
}

/*
 * Method:    RefreshChilds()
 * Purpose:   refresh all the childs of a given tree item
 * Comments:  none
 */ 
void CCtrlFrame::RefreshChilds(HTREEITEM hParent,ULONG nFlags)
{
    HTREEITEM hChildItem = m_Tree->GetChildItem(hParent);

    if(hChildItem)
    {
        while (hChildItem != NULL)
        {
            RefreshChilds(hChildItem,nFlags);
            hChildItem = m_Tree->GetNextItem(hChildItem, TVGN_NEXT);
        }     
    }
    else
        RefreshChild(hParent,nFlags);
}

/*
 * Method:    RefreshChild()
 * Purpose:   refresh all the childs of a given tree item
 * Comments:  none
 */
void CCtrlFrame::RefreshChild(HTREEITEM hChild,ULONG nFlags)
{
    CString strLabel;
    CString Value;
    int nPos;
    tPvAttributeInfo Info;

    // get the attribute label
	strLabel = m_Tree->GetItemText(hChild);
	nPos = strLabel.Find('\t');
	if (nPos >= 0)
		strLabel = strLabel.Left(nPos);

    if(PvAttrInfo(m_Camera,strLabel,&Info) == ePvErrSuccess) 
    {
        BOOL Cont;

        if(nFlags)
            Cont = Info.Flags & nFlags;
        else
            Cont = true;

        if(Cont)
            if(FormatAttributeValue(strLabel,Info,Value))
            {
                CString String = strLabel;

                String += "\t";
                String += Value;

                m_Tree->SetItemText(hChild,String);

                /*
                // if this attribute is currently been
                // edited we may need to update the
                // value displayed
                if(m_Form && m_Attr == strLabel)
                    RefreshCurrentForm();  
                */    
            }
    }
}

/*
 * Method:    RefreshCurrentForm()
 * Purpose:   refresh all the fields of the currently selected attribute
 * Comments:  none
 */ 
void CCtrlFrame::RefreshCurrentForm()
{
    if(m_Info.Flags & ePvFlagWrite)
    {
        tPvErr Err;

        switch(m_Info.Datatype)
        {
            case ePvDatatypeEnum:
            {  
                CEnumValueView* pForm = (CEnumValueView*)m_Form;
                char Range[256];
                char Value[32];  

                // retreive the range
                if(((Err = PvAttrRangeEnum(m_Camera,(LPCTSTR)m_Attr,Range,256,NULL)) == ePvErrSuccess) &&
                   ((Err = PvAttrEnumGet(m_Camera,(LPCTSTR)m_Attr,Value,32,NULL)) == ePvErrSuccess))
                {
                   if(pForm->SetRange(Range))
                       pForm->SetValue(Value);                            
                }

                break;
            }
            case ePvDatatypeBoolean:
            {  
                CEnumValueView* pForm = (CEnumValueView*)m_Form;
                tPvBoolean Value;

                if((Err = PvAttrBooleanGet(m_Camera,(LPCTSTR)m_Attr,&Value)) == ePvErrSuccess)
                {
                   if(pForm->SetRange(kBooleanRange))
                       pForm->SetValue(Value ? kBooleanTrue : kBooleanFalse);                            
                }

                break;
            }
            case ePvDatatypeUint32:
            {
                CUintValueView* pForm = (CUintValueView*)m_Form;
                tPvUint32 nValue;
                tPvUint32 nMin,nMax;

                // retreive the range
                if(((Err = PvAttrRangeUint32(m_Camera,(LPCTSTR)m_Attr,&nMin,&nMax)) == ePvErrSuccess) &&
                   ((Err = PvAttrUint32Get(m_Camera,(LPCTSTR)m_Attr,&nValue)) == ePvErrSuccess))
                {
                   if(pForm->SetRange(nMin,nMax))
                       pForm->SetValue(nValue); 
                }
                       
                break;
            }
            case ePvDatatypeInt64:
            {
                CSintValueView* pForm = (CSintValueView*)m_Form;
                tPvInt64 nValue;
                tPvInt64 nMin,nMax;

                // retreive the range
                if(((Err = PvAttrRangeInt64(m_Camera,(LPCTSTR)m_Attr,&nMin,&nMax)) == ePvErrSuccess) &&
                   ((Err = PvAttrInt64Get(m_Camera,(LPCTSTR)m_Attr,&nValue)) == ePvErrSuccess))
                {
                   if(pForm->SetRange(nMin,nMax))
                       pForm->SetValue(nValue); 
                }
                       
                break;
            }
            case ePvDatatypeFloat32:
            {
                CFloatValueView* pForm = (CFloatValueView*)m_Form;
                tPvFloat32 nValue;
                tPvFloat32 nMin,nMax;

                // retreive the range
                if(((Err = PvAttrRangeFloat32(m_Camera,(LPCTSTR)m_Attr,&nMin,&nMax)) == ePvErrSuccess) &&
                   ((Err = PvAttrFloat32Get(m_Camera,(LPCTSTR)m_Attr,&nValue)) == ePvErrSuccess))
                {
                   if(pForm->SetRange(nMin,nMax))
                       pForm->SetValue(nValue);                             
                }
                 
                break;
            }
            case ePvDatatypeString:
            {
                CStringValueView* pForm = (CStringValueView*)m_Form;

                if(pForm)
                {                  
                    char Value[256];
                    CString String;

                    // retreive the value
                    if((Err = PvAttrStringGet(m_Camera,(LPCTSTR)m_Attr,Value,256,NULL)) == ePvErrSuccess)
                    {
                       String = Value;
                       pForm->SetValue(String);                          
                    }
                }

                break;
            }
            default:
                break;
        }
    } 
}
