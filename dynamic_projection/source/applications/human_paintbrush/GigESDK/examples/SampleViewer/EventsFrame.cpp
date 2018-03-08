/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        EventsFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the Events window	
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
#include "EventsFrame.h"

//===== DEFINES ===============================================================

#define WIDTH  450
#define HEIGHT 310

//===== CONSTANTS =============================================================

static UINT uCameraEvent = ::RegisterWindowMessage(_T("CameraEvent"));

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CEventsFrame, CChildFrame)

// message map
BEGIN_MESSAGE_MAP(CEventsFrame, CChildFrame)
    ON_REGISTERED_MESSAGE(uCameraEvent,OnCameraEvent)
END_MESSAGE_MAP()

void __stdcall CameraEventCB(void* Context,tPvHandle Camera,const tPvCameraEvent* EventList,unsigned long EventListLength);

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CEventsView
 * Purpose:  Derive the FormView to create a view to hold the various fields
 *           to be displayed by the window
 * Comments: none
 */
class CEventsView : public CFormView
{
    DECLARE_DYNCREATE(CEventsView)

    public:

        /*
         * Method:    CEventsView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CEventsView();

        /*
         * Method:    Create()
         * Purpose:   create the view
         * Arguments: 
         *
         * [i] const RECT& Rect, rect of the view
         * [i] CWnd* pParent,    view's parent
         *
         * Return:    true is no error
         * Comments:  none
         */
	    BOOL Create(const RECT& Rect, CWnd* pParent);

        afx_msg void OnClearList();
        afx_msg void OnPauseList();
        afx_msg void OnAutoSrollList();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

};

//===== CLASS IMPLEMENTATIONS =================================================

IMPLEMENT_DYNCREATE(CEventsView, CFormView)

// message map
BEGIN_MESSAGE_MAP(CEventsView, CFormView)
    ON_BN_CLICKED(IDC_BUTTON1,OnClearList)
    ON_BN_CLICKED(IDC_BUTTON2,OnPauseList)
    ON_BN_CLICKED(IDC_CHECK1,OnAutoSrollList)
END_MESSAGE_MAP()

/*
 * Method:    CEventsView()
 * Purpose:   class constructor
 * Comments:  none
 */
CEventsView::CEventsView()
    : CFormView(IDD_EVENTSVIEW)
{
}

/*
 * Method:    Create()
 * Purpose:   create the view
 * Comments:  none
 */
BOOL CEventsView::Create(const RECT& Rect, CWnd* pParent)
{
    if(CFormView::Create(NULL,_T("eventsview"),WS_CHILD | WS_VISIBLE,Rect,pParent,0,NULL))
    {
        ((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(true);

        return true;
    }
    else
        return false;
}

void CEventsView::OnClearList()
{
    CListBox* lList = (CListBox*)GetDlgItem(IDC_LIST1);    

    lList->ResetContent();
}

void CEventsView::OnPauseList()
{
    CEventsFrame* lFrame = (CEventsFrame*)this->GetParent();
    CButton*      lButton = (CButton*)GetDlgItem(IDC_BUTTON2); 

    lFrame->Pause();

    if(lFrame->IsPaused())
        lButton->SetWindowText("Resume");
    else
        lButton->SetWindowText("Pause");
}

void CEventsView::OnAutoSrollList()
{
    CEventsFrame* lFrame = (CEventsFrame*)this->GetParent();

    lFrame->SetAutoScroll(((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck());     
}

/*
 * Method:    CEventsFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CEventsFrame::CEventsFrame()
{
    m_Paused = false;
    m_Scroll = true;
    m_Stamp  = 0;
    m_Going  = false;
}

/*
 * Method:    ~CEventsFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CEventsFrame::~CEventsFrame()
{
    DeleteCriticalSection(&m_Lock);
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CEventsFrame::Create(tPvHandle aHandle,tPvCameraInfoEx* pInfo)
{
    BOOL result;

    // initialize the locking mecanism
    try
    {
        InitializeCriticalSection(&m_Lock);
    } catch (...) {
        return FALSE;
    }

    if((result = CChildFrame::Create(pInfo,CChildFrame::eEvents)))
    {
        m_Handle = aHandle;

        // create the view
        m_View = new CEventsView;
        if(m_View)
            result = m_View->Create(CRect(0,0,WIDTH,HEIGHT),this);
        else
            result = false;

        // register the event CB
        if(result)
        {
            tPvUint32 lFreq;

            if(PvAttrUint32Get(m_Handle,"TimeStampFrequency",&lFreq) == ePvErrSuccess)
                m_Frequency = (double)lFreq;
            else
                m_Frequency = 0;

            PvCameraEventCallbackRegister(m_Handle,CameraEventCB,this);
        }
    }

    return result;
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CEventsFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CMDIChildWnd::PreCreateWindow(cs))
    {
        // remove a few flags from the window style
        cs.style &= ~WS_MAXIMIZEBOX;
        cs.style &= ~WS_MINIMIZEBOX;
        cs.style &= ~WS_THICKFRAME;

        // set the window size
        cs.cx = WIDTH;
        cs.cy = HEIGHT;

        return TRUE;
    }
    else
	    return FALSE;
}

/*
 * Method:    OnDestroy()
 * Purpose:   called by the framework when the window is about to
 *            be destroyed
 * Arguments: none
 * Return:    none
 * Comments:  none
 */
void CEventsFrame::OnDestroy()
{
    EnterCriticalSection(&m_Lock);
    m_Going = TRUE;
    LeaveCriticalSection(&m_Lock);

    // remove the event CB
    PvCameraEventCallbackUnRegister(m_Handle,CameraEventCB);

    CChildFrame::OnDestroy();
}

/*
 * Method:    OnCommandReceived()
 * Purpose:   called when the window has received a command
 * Comments:  none
 */
void CEventsFrame::OnCommandReceived(const tCommand* pCommand)
{
}

/*
 * Method:    OnCameraEvent()
 * Purpose:   called when a camera's event was received
 * Comments:  none
 */
LRESULT CEventsFrame::OnCameraEvent(WPARAM wParam,LPARAM lParam)
{
    if(!m_Paused)
    {
        double           lTime;
        unsigned __int64 lStamp;
        int              lLast;
        CString          lString;
        CListBox*        lList = (CListBox*)m_View->GetDlgItem(IDC_LIST1);
        
        if(lList->GetCount() > 2000)
            lList->ResetContent();

        EnterCriticalSection(&m_Lock);
        
        for(int i=0;i<m_Events.size();i++)
        {
            lStamp = m_Events[i].TimestampHi;
            lStamp <<= 32;
            lStamp += m_Events[i].TimestampLo;

            if(m_Stamp)
                lTime = ((long double)lStamp - (long double)m_Stamp) / m_Frequency;
            else
            {
                lTime = 0;
                m_Stamp = lStamp;
            }

			lString.Format("% 10.2lf : %u [0x%08X,0x%08X,0x%08X,0x%08X]",lTime,m_Events[i].EventId,m_Events[i].Data[0],m_Events[i].Data[1],m_Events[i].Data[2],m_Events[i].Data[3]);

            lLast = lList->AddString(lString);

            lString.Empty();
        }

        m_Events.clear();

        LeaveCriticalSection(&m_Lock);

        if(m_Scroll)
        {
            int nCount = lList->GetCount();
            if (nCount > 0)
                lList->SetCurSel(nCount-1);
        }
    }
    
    return true;
}

void CEventsFrame::Pause()
{
    m_Paused = !m_Paused;

    if(!m_Paused)
        SendMessage(uCameraEvent);
}

void CEventsFrame::PushEvents(const tPvCameraEvent* pEvents,DWORD Count)
{
    EnterCriticalSection(&m_Lock);

    if(!m_Going)
    {
        for(int i=0;i<Count;i++)
            m_Events.push_back(pEvents[i]);

        PostMessage(uCameraEvent);
    }

    LeaveCriticalSection(&m_Lock); 
}

void __stdcall CameraEventCB(void* Context,tPvHandle Camera,const tPvCameraEvent* EventList,unsigned long EventListLength)
{
    CEventsFrame* lFrame = (CEventsFrame*)Context;

    if(lFrame)
        lFrame->PushEvents(EventList,EventListLength);

}