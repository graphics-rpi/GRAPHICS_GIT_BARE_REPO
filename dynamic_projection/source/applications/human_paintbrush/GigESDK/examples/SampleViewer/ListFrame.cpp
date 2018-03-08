/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ListFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the camera listing frame	
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
#include "ListFrame.h"
#include "Controls.h"
#include "IPInput.h"

#include <PvAPI.h>

//===== DEFINES ===============================================================

#define MIN(a,b) (a < b ? a : b)

//===== CONSTANTS =============================================================

const tPvUint32 KMaxPossibleCameras = 50;

// commands to be sent to the main window
const int KCmdShowInfo = 0xA;
const int KCmdShowCtrl = 0xB;
const int KCmdShowLive = 0xC;
const int KCmdShowHist = 0xD;
const int KCmdShowSeek = 0xF;
const int KCmdShowExpo = 0x1;
const int KCmdShowEvnt = 0x2;
const int KCmdShowSrIO = 0x3;

//===== LOCAL DATA ============================================================

// register a message ID for the camera events
static UINT uCameraEvent = ::RegisterWindowMessage(_T("PvCameraEvent"));

// registered messages ID used to send message to the main frame
extern UINT uCameraAction;
extern UINT uCameraRemoval;
// register a message ID for error reporting
extern UINT uErrorReport;

// ??
IMPLEMENT_DYNCREATE(CListFrame, CMDIChildWnd)

// message map for the class
BEGIN_MESSAGE_MAP(CListFrame, CMDIChildWnd)

    ON_WM_DESTROY()
    ON_WM_MOUSEMOVE()
    ON_WM_NCMOUSEMOVE()
    ON_WM_MDIACTIVATE()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()

    ON_REGISTERED_MESSAGE( uCameraEvent,OnCameraEvent)

    ON_COMMAND(KCmdShowInfo,OnShowInfo)
    ON_COMMAND(KCmdShowCtrl,OnShowCtrl)
    ON_COMMAND(KCmdShowLive,OnShowLive)
    ON_COMMAND(KCmdShowHist,OnShowHist)
    ON_COMMAND(KCmdShowSeek,OnShowSeek)
    ON_COMMAND(KCmdShowExpo,OnShowExpo)
    ON_COMMAND(KCmdShowEvnt,OnShowEvnt)
    ON_COMMAND(KCmdShowSrIO,OnShowSrIO)

END_MESSAGE_MAP()

//===== FUNCTION PROTOTYPES ===================================================

/*
 * Function:  CameraEventCB()
 * Purpose:   Callback from PvAPI related to camera's event (addition/removal)
 * Arguments:
 *
 *  [i] void* Context,          Context pointer passed when registered the
 *                              callback
 *  [i] tPvInterface Interface, Interface on which the event occured
 *  [i] tPvLinkEvent Event,     Event that occured
 *  [i] unsigned long UniqueId, Unique ID of the camera related to the event
 *
 * Return:    none
 * Comments:  none
 */
void __stdcall CameraEventCB(void* Context,tPvInterface Interface,
                             tPvLinkEvent Event,unsigned long UniqueId);

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CListFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CListFrame::CListFrame()
{ 
    m_Tree      = NULL;
    m_Toolbar   = NULL;
}

/*
 * Method:    ~CListFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CListFrame::~CListFrame()
{
    delete m_Tree;
    delete m_Toolbar;
}

/*
 * Method:    SelectCamera()
 * Purpose:   select a given camera
 * Comments:  none
 */  
void CListFrame::SelectCamera(ULONG nUID)
{
    // loop over all the tree item (childs of the root) until
    // we find the camera that was removed and then remove the
    // item from the tree
    if(m_Tree->ItemHasChildren(m_Root))
    {
       HTREEITEM hChildItem = m_Tree->GetChildItem(m_Root);
       BOOL      bReselect  = FALSE;

       while (hChildItem != NULL)
       {
            if(m_Tree->GetItemData(hChildItem) == nUID)
            {
                m_Tree->SelectItem(hChildItem);
                break;
            }

            hChildItem = m_Tree->GetNextItem(hChildItem, TVGN_NEXT);
       }
    }       
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CListFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    // modify the construction parameters
    cs.cx    = 240;
    cs.cy    = 300;
    cs.style = WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_BORDER | WS_THICKFRAME;

    // register the callback for the camera events
    PvLinkCallbackRegister(CameraEventCB,ePvLinkAdd,this);
    PvLinkCallbackRegister(CameraEventCB,ePvLinkRemove,this);

    // and call the parent's method
	return CMDIChildWnd::PreCreateWindow(cs);
}

/*
 * Method:    OnCreateClient()
 * Purpose:   called by the framework during the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CListFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
    // set the window title
    SetWindowText(_T("Cameras"));

    // create the tree control
    m_Tree = new CTreeCtrl;
    if(m_Tree)
    {
        if(m_Tree->Create(WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,CRect(0, 0, 300, 260), this, KListFrameTree))
        {

            m_Toolbar = new CToolBarCtrl;
            if(m_Toolbar)
            {
                if(m_Toolbar->Create(WS_VISIBLE | WS_CHILD | CCS_BOTTOM | CBRS_FLYBY,CRect(0, 0, 0, 0),this,KListFrameToolbar))
                {
                    TBBUTTON Button;

                    memset(&Button,0,sizeof(TBBUTTON));

                    m_Toolbar->SetStyle(TBSTYLE_FLAT);
                    m_Toolbar->SetIndent(5);

                    Button.fsState   = TBSTATE_ENABLED;
                    Button.fsStyle   = TBSTYLE_BUTTON;

                    Button.idCommand = KCmdShowExpo;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP8);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowSeek;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP7);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowSrIO;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP10);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowEvnt;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP9);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowHist;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP4);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowLive;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP3);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowCtrl;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP2);
                    m_Toolbar->InsertButton(0,&Button);

                    Button.idCommand = KCmdShowInfo;
                    Button.iBitmap   = m_Toolbar->AddBitmap(1,IDB_BITMAP1);
                    m_Toolbar->InsertButton(0,&Button);

                    // and create the root item for the tree

                    TVINSERTSTRUCT item;

                    item.hParent        = NULL;
                    item.hInsertAfter   = TVI_FIRST;
                    item.item.mask      = TVIF_TEXT;
                    item.item.pszText   = "Host";

                    m_Root = m_Tree->InsertItem(&item);

                    // then list all the cameras already connected

                    DoCameraDiscovery();

                    m_Tree->SetFocus();

                    return CMDIChildWnd::OnCreateClient(lpcs,pContext);
                }
                else
                    return FALSE;
            }
            else
                return FALSE;
        } 
        else
            return FALSE;
    }
    else
        return FALSE;   
}

/*
 * Method:    OnMDIActivate()
 * Purpose:   called by the framework when the window is (de)activated
 * Comments:  none
 */
void CListFrame::OnMDIActivate( BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd )
{
   if(bActivate)
        m_Tree->SetFocus();
   
   CMDIChildWnd::OnMDIActivate(bActivate,pActivateWnd,pDeactivateWnd);
}


/*
 * Method:    OnClose()
 * Purpose:   called by the framework when the window is about to
 *            be distroyed
 * Comments:  none
 */
void CListFrame::OnDestroy()
{
    // unregister the callback for the camera events
    PvLinkCallbackUnRegister(CameraEventCB,ePvLinkAdd);
    PvLinkCallbackUnRegister(CameraEventCB,ePvLinkRemove);    

    // call the parent's method
    CMDIChildWnd::OnDestroy();
}

/*
 * Method:    OnSize()
 * Purpose:   called by the framework when the window was resized
 * Comments:  none
 */
void CListFrame::OnSize(UINT nType, int cx, int cy)
{
    CRect lRect;

    m_Toolbar->GetWindowRect(&lRect);
    m_Toolbar->SetWindowPos(NULL,0,cy - lRect.Height(),0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    m_Tree->SetWindowPos(NULL,0,0,cx,cy - lRect.Height(),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    CMDIChildWnd::OnSize(nType, cx, cy);
}

/*
 * Method:    OnGetMinMaxInfo()
 * Purpose:   called by the framework to retreive the min&max
 *            size of the window
 * Comments:  none
 */
void CListFrame::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
    CMDIChildWnd::OnGetMinMaxInfo(lpMMI);

    lpMMI->ptMinTrackSize.x = 240;
    lpMMI->ptMinTrackSize.y = 320;
    lpMMI->ptMaxTrackSize.x = 240;
}

/*
 * Method:    OnCameraEvent()
 * Purpose:   handle a camera related event
 * Comments:  none
 */
LRESULT CListFrame::OnCameraEvent(WPARAM wParam,LPARAM lParam)
{
    tPvLinkEvent Event = (tPvLinkEvent)wParam; 
    unsigned long UID  = (unsigned long)lParam;

    switch(Event)
    {
        case ePvLinkAdd: // a camera was plugged
        {
            BOOL bFound = FALSE;

            // let's make sure that we don't already know about this camera.
            // we could be notified if the camera is discovered before we did
            // the discovery ealier ...
            if(m_Tree->ItemHasChildren(m_Root))
            {
               HTREEITEM hNextItem;
               HTREEITEM hChildItem = m_Tree->GetChildItem(m_Root);
               
               while (hChildItem != NULL)
               {
                    hNextItem = m_Tree->GetNextItem(hChildItem, TVGN_NEXT);
   
                    if(m_Tree->GetItemData(hChildItem) == UID)
                    {
                        bFound = true;
                        break;
                    }

                    hChildItem = hNextItem;
               }
            }            

            if(!bFound)
            {
                tPvCameraInfoEx Info;

                // get the info on the camera
                if(PvCameraInfoEx(UID,&Info,sizeof(tPvCameraInfoEx)) == ePvErrSuccess)
                {
                    TVINSERTSTRUCT item;
                    CString

                    String =  Info.SerialNumber;
                    String += _T(" (");
                    String += Info.CameraName;
                    String += _T(")");

                    // then fill the item struct for the list
                    item.hParent       = m_Root;
                    item.hInsertAfter  = TVI_LAST;
                    item.item.mask     = TVIF_TEXT | TVIF_PARAM;
                    item.item.pszText  = const_cast<char *>((LPCTSTR)String);
                    item.item.lParam   = UID;

                    // insert the item
                    m_Tree->InsertItem(&item);

                    // and make sure the root is expanded
                    m_Tree->Expand(m_Root,TVE_EXPAND);

                    #ifdef _ANALOG_TEST
                    GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowLive);
                    GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowHist);
                    #endif

                }
            }

            break;
        }
        case ePvLinkRemove: // a camera was unplugged
        {
   
            // loop over all the tree item (childs of the root) until
            // we find the camera that was removed and then remove the
            // item from the tree
            if(m_Tree->ItemHasChildren(m_Root))
            {
               HTREEITEM hNextItem;
               HTREEITEM hChildItem = m_Tree->GetChildItem(m_Root);
               BOOL      bReselect  = FALSE;

               while (hChildItem != NULL)
               {
                    hNextItem = m_Tree->GetNextItem(hChildItem, TVGN_NEXT);
   
                    if(m_Tree->GetItemData(hChildItem) == UID)
                    {
                        // was the item selected?
                        bReselect = (hChildItem == m_Tree->GetSelectedItem());
                        m_Tree->DeleteItem(hChildItem);
                        break;
                    }

                    hChildItem = hNextItem;
               }

               // if the removed item was selected, then we select the first
               // item in the tree
               if(bReselect)
                    m_Tree->SelectItem(m_Tree->GetChildItem(m_Root));

               // notify the main frame that a camera is gone
               GetMDIFrame()->PostMessage(uCameraRemoval,UID);
            }

            break;
        }
    }

    return 0;
}

/*
 * Method:    OnShowInfo()
 * Purpose:   user requested the display of some info on
 *            the camera
 * Comments:  none
 */
void CListFrame::OnShowInfo()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowInfo);
    else
        ReportError(_T("You must select a camera first."));
}

/*
 * Method:    OnShowCtrl()
 * Purpose:   user requested the display of the camera
 *            control window
 * Comments:  none
 */
void CListFrame::OnShowCtrl()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowCtrl);
    else
        ReportError(_T("You must select a camera first."));
}

/*
 * Method:    OnShowLive()
 * Purpose:   user requested the display of the camera
 *            live window
 * Comments:  none
 */
void CListFrame::OnShowLive()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowLive);
    else
        ReportError(_T("You must select a camera first."));
}

/*
 * Method:    OnShowHist()
 * Purpose:   user requested the display of the camera
 *            histogram window
 * Comments:  none
 */
void CListFrame::OnShowHist()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowHist);
    else
        ReportError(_T("You must select a camera first."));
}

/*
 * Method:    OnShowHist()
 * Purpose:   user requested the display of the seek
 *            camera window
 * Comments:  none
 */
void CListFrame::OnShowSeek()
{
    CIPInput InputBox(this);

    if(InputBox.DoModal() == IDOK)
    {
        DWORD lAddr = 0;
        
        InputBox.GetAddress(lAddr);
        if(lAddr)
        {
            tPvCameraInfo Info;
            tPvErr lErr;

            BeginWaitCursor(); 
            if((lErr = PvCameraInfoByAddr(lAddr,&Info,NULL)))
                ReportError(_T("Sorry, no such camera was detected."));
            EndWaitCursor(); 
        }
    }
}

/*
 * Method:    OnShowExpo()
 * Purpose:   user requested to export the camera setup to
 *            a text file
 * Comments:  none
 */
void CListFrame::OnShowExpo()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowExpo);
    else
        ReportError(_T("You must select a camera first."));
}

/*
 * Method:    OnShowEvnt()
 * Purpose:   user requested to see the events for the camera
 * Comments:  none
 */
void CListFrame::OnShowEvnt()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowEvnt);
    else
        ReportError(_T("You must select a camera first."));    
}

/*
 * Method:    OnShowSrIO()
 * Purpose:   user requested to see the serial for the camera
 * Comments:  none
 */
void CListFrame::OnShowSrIO()
{
    HTREEITEM       Item;
    unsigned long   UID = 0xFF000000;

    // get the UID of the camera currently selected in the tree
    Item = m_Tree->GetSelectedItem();
    if(Item)
        UID = (unsigned long)m_Tree->GetItemData(Item);

    if(UID != 0xFF000000)
        GetMDIFrame()->PostMessage(uCameraAction,UID,KCmdShowSrIO);
    else
        ReportError(_T("You must select a camera first."));    
}

BOOL CListFrame::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{ 
    if(wParam == KListFrameToolbar)
    {
        NMHDR* pDetails = (NMHDR*)lParam;
      
        if(pDetails->code == TBN_HOTITEMCHANGE)
        {
            LPNMTBHOTITEM pHotItem = (LPNMTBHOTITEM)lParam;

            if(!(pHotItem->dwFlags & HICF_LEAVING))
            {
                switch(pHotItem->idNew)
                {
                    case KCmdShowInfo:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_INFO); 
                        break;
                    }
                    case KCmdShowCtrl:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_CTRL); 
                        break;
                    }
                    case KCmdShowLive:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_LIVE); 
                        break;
                    }
                    case KCmdShowHist:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_HIST); 
                        break;
                    }
                    case KCmdShowSeek:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_SEEK); 
                        break;
                    }
                    case KCmdShowExpo:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_EXPO); 
                        break;
                    }
                    case KCmdShowEvnt:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_EVENTS); 
                        break;
                    }
                    case KCmdShowSrIO:
                    {
                        GetMDIFrame()->SetMessageText(ID_STRTIP_SERIAL); 
                        break;
                    }
                    default:
                        break;
                }
            }
            else
                GetMDIFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
        }

        return true;
    }
    else
        return false;
}

/*
 * Method:    DoCameraDiscovery()
 * Purpose:   discover all the camera plugged to the host
 *            and add them to the tree
 * Comments:  none
 */      
void CListFrame::DoCameraDiscovery()
{
    tPvCameraInfo   Infos[KMaxPossibleCameras];
    tPvUint32       Count;    

    // list all the cameras currently connected
    if((Count == PvCameraList(Infos,KMaxPossibleCameras,&Count)))
    {
        TVINSERTSTRUCT item;
        CString String;

        // then fill the common part of the item
        item.hParent       = m_Root;
        item.hInsertAfter  = TVI_LAST;
        item.item.mask     = TVIF_TEXT | TVIF_PARAM;

        // and loop over all the cameras found to add item to the tree
        for(tPvUint32 i=0;i<MIN(KMaxPossibleCameras,Count);i++)
        {
            // the camera's model could be retreived trought accessing
            // the appropriate attribute if the camera was open. To 
            // avoid opening it just to read its model, we call bellow
            // an helper function which using the camera part number
            // return us the model string.

            String = Infos[i].SerialString;
            String += _T(" (");
            String += Infos[i].DisplayName;
            String += _T(")");

            // fill the part that is specific
            item.item.pszText  = const_cast<char *>((LPCTSTR)String);
            item.item.lParam   = Infos[i].UniqueId;

            // then insert the item
            m_Tree->InsertItem(&item);

            #ifdef _ANALOG_TEST
            GetMDIFrame()->PostMessage(uCameraAction,Infos[i].UniqueId,KCmdShowLive);
            GetMDIFrame()->PostMessage(uCameraAction,Infos[i].UniqueId,KCmdShowHist);
            #endif
        }

        // select the first item
        m_Tree->SelectItem(m_Tree->GetChildItem(m_Root));

        // make sure the root is expanded
        m_Tree->Expand(m_Root,TVE_EXPAND);
    }
}

/*
 * Method:    ReportError()
 * Purpose:   report an error to the user
 * Comments:  none
 */
void CListFrame::ReportError(const char* Text,tPvErr Error /* = ePvErrSuccess */)
{
    GetMDIFrame()->PostMessage(uErrorReport,*(WPARAM *)&Text,Error);  
}

//===== FUNCTION IMPLEMENTATIONS ==============================================

/*
 * Function:  CameraEventCB()
 * Purpose:   Callback from PvAPI related to camera's event (addition/removal)
 * Comments:  none
 */  
void __stdcall CameraEventCB(void* Context,tPvInterface Interface,
                             tPvLinkEvent Event,unsigned long UniqueId)
{
    CListFrame* pWindow = (CListFrame*)Context;
    
    pWindow->PostMessage(uCameraEvent,Event,UniqueId);
}
