/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ChildFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement our MDI child class	
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
#include "ChildFrame.h"

//===== CONSTANTS =============================================================

// child's event used to report the closing of the window to the main window
const int KEvnClosed    = 0xA;
// child's event used to report the activation of the window
const int KEvnActivated = 0xB;

//===== LOCAL DATA ============================================================

// event used to notify the parent class of some event
extern UINT uChildEvent;
// register a message ID for error reporting
extern UINT uErrorReport;
// register a message ID for warning reporting
extern UINT uWarningReport;
// register a message ID for forwarding command
extern UINT uCommandForwarding;
// get a camera handle
extern UINT uCommandGetHandle;

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

// message map
BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
    ON_WM_DESTROY()
    ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CChildFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CChildFrame::CChildFrame()
{
    m_Info      = NULL;
    m_Model     = NULL;
    m_Blocked   = false;
    m_Destroy   = false;
}

/*
 * Method:    ~CChildFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CChildFrame::~CChildFrame()
{
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CChildFrame::Create(tPvCameraInfoEx* pInfo,TChildType Type)
{
    // set the type
    m_Type  = Type;
    // set the info
    m_Info  = pInfo;
    // get the camera's model
    m_Model = m_Info->CameraName;

    // call the parent class create method
    return CMDIChildWnd::Create(NULL,_T("child"),WS_CHILD | WS_OVERLAPPEDWINDOW);
}

/*
 * Method:    Show()
 * Purpose:   show the frame on screen
 * Comments:  none
 */ 
void CChildFrame::Show(HICON hIcon)
{
    // set the icon of the window
    SetIcon(hIcon,true);
    // display the window
    ShowWindow(SW_SHOW);
    MDIActivate();
    // and update it
    UpdateWindow(); 
}

/*
 * Method:    Close()
 * Purpose:   close the frame
 * Comments:  destroy the frame
 */
void CChildFrame::Close()
{
    if(!m_Blocked)
        MDIDestroy();
    else
    {   
        OnShouldClose();
        m_Destroy = true;
    }
}

void CChildFrame::ResizeToFit(UINT nWidth,UINT nHeight)
{
    CRect rect;
    CRect limit,limit2;
  
    // get the parent area rect, so that we can limit the
    // size of the frame to fit in it
    GetMDIFrame()->GetClientRect(&limit);
    limit2 = limit;
    CalcWindowRect(&limit2,CWnd::adjustOutside);
    limit.right -= limit2.Width() - limit.Width() + 25;
    limit.bottom -= limit2.Height() - limit.Height() + 25;

    // set the rect. of the frame
    rect.left   = 0;
    rect.top    = 0;
    rect.bottom = min(nHeight,(UINT)limit.Height());
    rect.right  = min(nWidth,(UINT)limit.Width());

    // calculate the window size for the window area we wish
    CalcWindowRect(&rect,CWnd::adjustOutside);
    // them move/resize the window
    SetWindowPos(NULL,0,0,rect.Width(),rect.Height(),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);  
}

/*
 * Method:    GetHandleToCamera()
 * Purpose:   get the handle to the camera
 * Arguments: none
 * Return:    the camera handle
 * Comments:  none
 */
tPvHandle CChildFrame::GetHandleToCamera()
{
    tPvHandle lHandle = 0;

    GetMDIFrame()->SendMessage(uCommandGetHandle,m_Info->UniqueId,(LPARAM)&lHandle);  

    return lHandle;
}

/*
 * Method:    Block()
 * Purpose:   block the frame
 * Comments:  none
 */
void CChildFrame::Block()
{
    m_Blocked = true;
}

/*
 * Method:    Unblock()
 * Purpose:   unblock the frame
 * Comments:  none
 */
void CChildFrame::Unblock()
{
    m_Blocked = false;
}

/*
 * Method:    DestroyIfRequested()
 * Purpose:   destroy the frame if it was requested
 * Comments:  none
 */
void CChildFrame::DestroyIfRequested()
{
    if(m_Destroy)
        MDIDestroy();
}

/*
 * Method:    SendCommandTo()
 * Purpose:   send a command to another child window
 * Comments:  none
 */
void CChildFrame::SendCommandTo(TChildType aTarget,tCommand* pCommand)
{
    pCommand->Uid = m_Info->UniqueId;

    GetMDIFrame()->SendMessage(uCommandForwarding,aTarget,(LPARAM)pCommand);    
}

/*
 * Method:    ReportError()
 * Purpose:   report an error to the user
 * Comments:  none
 */
void CChildFrame::ReportError(const char* Text,tPvErr Error /* = ePvErrSuccess */)
{
    GetMDIFrame()->PostMessage(uErrorReport,*(WPARAM *)&Text,Error);        
}

/*
 * Method:    ReportWarning()
 * Purpose:   report a warning to the user
 * Comments:  none
 */
void CChildFrame::ReportWarning(const char* Text)
{
    GetMDIFrame()->PostMessage(uWarningReport,*(WPARAM *)&Text,0);        
}

/*
 * Method:    OnCreateClient()
 * Purpose:   called by the framework during the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
    CString Title;
 
    // format the title of the window
    Title = m_Info->SerialNumber;
    Title += _T(" (");
    Title += m_Model;
    Title += _T(") ");
    switch(m_Type)
    {
        case eView:
        {
            Title += _T("- View");
            break;
        }
        case eInfo:
        {
            Title += _T("- Information");
            break;
        }
        case eCtrl:
        {
            Title += _T("- Controls");
            break;
        }
        case eCtrlRO:
        {
            Title += _T("- Controls (RO)");
            break;
        }
        case eHist:
        {
            Title += _T("- Histogram");
            break;
        }
        case eEvents:
        {
            Title += _T("- Events");
            break;
        }
        case eSerial:
        {
            Title += _T("- SerialIO");
            break;
        }
    }

    // set the window
    SetWindowText(Title);
    
    return CMDIChildWnd::OnCreateClient(lpcs,pContext); 
}

/*
 * Method:    OnClose()
 * Purpose:   called by the framework when the window is about to
 *            be distroyed
 * Comments:  none
 */
void CChildFrame::OnDestroy()
{
    // little trick needed to pass the @ of the object as one of
    // the message parameter
    CChildFrame* pFrame = this;

    // signal to the parent window that we are closing. This message is
    // sent Synchronously else this window may be already destroyed by
    // the time the parent handle the message
    GetMDIFrame()->SendMessage(uChildEvent,KEvnClosed,*(LPARAM *)&pFrame);

    // call the parent's method
    CMDIChildWnd::OnDestroy();
}

/*
 * Method:    OnMDIActivate()
 * Purpose:   called by the framework when the window is been activated
 * Comments:  none
 */
void CChildFrame::OnMDIActivate( BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd )
{
    // send a message to the main window when activated
    if(bActivate)
        GetMDIFrame()->SendMessage(uChildEvent,KEvnActivated,(LPARAM)m_Info->UniqueId);

    CMDIChildWnd::OnMDIActivate(bActivate,pActivateWnd,pDeactivateWnd);

    SetFocus();
}

