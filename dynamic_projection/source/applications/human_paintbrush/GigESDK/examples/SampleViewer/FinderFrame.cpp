/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        FinderFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the Live view window	
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
#include "FinderFrame.h"
#include "HistFrame.h"
#include "ImageView.h"
#include "TrackColor.h"

#include <ImageLib.h>
#include <math.h>
#include <iostream>
#include <fstream>

#define _ANCILLARY

//===== DEFINES ===============================================================

// ID used for the context menu commands
#define ID_SIZE_GRAB        2000
#define ID_SIZE_SAVE        (ID_SIZE_GRAB + 1)
#define ID_SIZE_SHOWANC     (ID_SIZE_GRAB + 2)
#define ID_SIZE_INTERPOLATE (ID_SIZE_GRAB + 3)
#define ID_GAIN_0           (ID_SIZE_GRAB + 4)
#define ID_GAIN_1           (ID_SIZE_GRAB + 5)
#define ID_GAIN_2           (ID_SIZE_GRAB + 6)
#define ID_GAIN_3           (ID_SIZE_GRAB + 7)
#define ID_GAIN_4           (ID_SIZE_GRAB + 8)
#define ID_GAIN_5           (ID_SIZE_GRAB + 9)
#define ID_GAIN_6           (ID_SIZE_GRAB + 10)
#define ID_SIZE_SCALE       (ID_SIZE_GRAB + 11)
#define ID_SIZE_100         (ID_SIZE_GRAB + 12)
#define ID_SIZE_090         (ID_SIZE_100 + 1)
#define ID_SIZE_080         (ID_SIZE_100 + 2)
#define ID_SIZE_070         (ID_SIZE_100 + 3)
#define ID_SIZE_060         (ID_SIZE_100 + 4)
#define ID_SIZE_050         (ID_SIZE_100 + 5)
#define ID_SIZE_040         (ID_SIZE_100 + 6)
#define ID_SIZE_030         (ID_SIZE_100 + 7)
#define ID_SIZE_020         (ID_SIZE_100 + 8)
#define ID_SIZE_010         (ID_SIZE_100 + 9)

extern void _OutputDebugString(const char* format, ...);

//===== CONSTANTS =============================================================

// number of frames used for streaming
const UINT KMaxFrames = 5;
//const UINT KMaxFrames = 1;
// target frame rate for rendering
const UINT kFrameRate = 45;
//const UINT kFrameRate = 5;

#ifdef _ANCILLARY
// maximum expected size of the ancillary buffer
const UINT kMaxAncillary = 1228800;
#endif


//===== FUNCTION PROTOTYPES ===================================================

/*
 * Function:  FrameDoneCB()
 * Purpose:   Callback from PvAPI related to the post handling of a frame
 * Arguments:
 *
 *  [i] tPvFrame* Frame, frame handled by the API
 *
 * Return:    none
 * Comments:  none
 */
void __stdcall FrameDoneCB(tPvFrame* pFrame);

//===== LOCAL DATA ============================================================

// register some events used internaly
static UINT uLiveStart      = ::RegisterWindowMessage(_T("LiveStart"));
static UINT uLiveStop       = ::RegisterWindowMessage(_T("LiveStop"));
static UINT uLiveHist       = ::RegisterWindowMessage(_T("LiveHist"));
static UINT uLiveError      = ::RegisterWindowMessage(_T("LiveErr"));
static UINT uLiveGrabbed    = ::RegisterWindowMessage(_T("LiveGrabbed"));

IMPLEMENT_DYNCREATE(CFinderFrame, CChildFrame)

// message map
BEGIN_MESSAGE_MAP(CFinderFrame, CChildFrame)
    ON_WM_ERASEBKGND()
	ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_CONTEXTMENU()
    ON_WM_GETMINMAXINFO()
    ON_COMMAND_RANGE(ID_SIZE_GRAB,ID_SIZE_010,OnContextCommand)
    ON_REGISTERED_MESSAGE( uLiveStart,OnStart)
    ON_REGISTERED_MESSAGE( uLiveStop,OnStop)
    ON_REGISTERED_MESSAGE( uLiveHist,OnHist)
    ON_REGISTERED_MESSAGE( uLiveError,OnError)
    ON_REGISTERED_MESSAGE( uLiveGrabbed,OnGrabbed)
END_MESSAGE_MAP()

// filters for saving frame
static char BASED_CODE szFilter[]    = "TIFF Files (*.tif)|*.tif||";
static char BASED_CODE szFilterBmp[] = "BMP Files (*.bmp)|*.bmp||";

// scale array
static double gScales[] = {
    1.0,0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1
};

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CFinderFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CFinderFrame::CFinderFrame()
{ 
    m_View          = NULL;
    m_Camera        = NULL;
    m_Model         = NULL;
    m_Hist          = NULL;
    m_Closing       = FALSE;
    m_Update        = FALSE;
    m_Streaming     = FALSE;
    m_Grab          = false;
    m_Width         = 0;
    m_Height        = 0;
    m_LastCtxId     = ID_SIZE_100;
    m_First         = true;
    m_Was           = 0;
    m_Now           = 0;
 
    // now allocate the frames
    m_Frames = new tPvFrame[KMaxFrames];
    if(m_Frames)
        memset(m_Frames,0,sizeof(tPvFrame) * KMaxFrames);
}

/*
 * Method:    ~CFinderFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CFinderFrame::~CFinderFrame()
{
    DeleteCriticalSection(&m_Protection);

    // delete the image buffer of the frames
    for(UINT i=0;i<KMaxFrames;i++)
        delete [] m_Frames[i].ImageBuffer;

    // and delete the frames
    delete [] m_Frames;
}

/*
 * Method:    Start()
 * Purpose:   start streaming
 * Comments:  none
 */  
void CFinderFrame::Start(BOOL bNow /* = false */)
{
    if(bNow)
        SendMessage(uLiveStart);    
    else
        PostMessage(uLiveStart);
}

/*
 * Method:    Stop()
 * Purpose:   stop streaming
 * Comments:  none
 */ 
void CFinderFrame::Stop(BOOL bNow /* = false */)
{
    if(bNow)
        SendMessage(uLiveStop);
    else
        PostMessage(uLiveStop);
}

/*
 * Method:    DoHistogram()
 * Purpose:   update (or not) the histogram window
 * Comments:  none
 */ 
void CFinderFrame::DoHistogram(CHistFrame* pWindow)
{
    // send a message to ask the window to handle
    // the histogram updates
    SendMessage(uLiveHist,0,*(LPARAM *)&pWindow);
}

/*
 * Method:    IsClosing()
 * Purpose:   check if the window is closing
 * Arguments: none
 * Return:    true or false
 * Comments:  none
 */ 
BOOL CFinderFrame::IsClosing()
{
    return m_Closing;
}

/*
 * Method:    RequestGrab()
 * Purpose:   request the next received frame to be grabbed
 * Comments:  none
 */ 
void CFinderFrame::RequestGrab(bool Grab)
{
    // lock the flag
    EnterCriticalSection(&m_Protection);
    // set the flag to true
    m_Grab = Grab;
    // unlock the flag
    LeaveCriticalSection(&m_Protection);    
}



/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CFinderFrame::Create(tPvHandle hCamera,tPvCameraInfoEx* pInfo,int Width,int Height)
{
    BOOL result;

    // if the frame array wasn't allocated, the window can't be
    // created
    if(!m_Frames)
        return false;

    m_Width  = Width;
    m_Height = Height;
    m_Camera = hCamera;
    
    // initialize the locking mecanism
    try
    {
        InitializeCriticalSection(&m_Protection);
    } catch (...) {
        return FALSE;
    }

    //// adjust packet size

    tPvUint32 MaxSize = 8228;

    // read the current packet size of the camera
    PvAttrUint32Get(hCamera,"PacketSize",&MaxSize);

    // then we adjust the packet size (max is the current stored value)
    PvCaptureAdjustPacketSize(hCamera,MaxSize);

    ////

    // create the window now
    if((result = CChildFrame::Create(pInfo,CChildFrame::eView)))
    {
        tPvUint32 lDepth = 12;

        PvAttrUint32Get(m_Camera,"SensorBits",&lDepth);

        ResizeToFit(Width,Height);
        CenterWindow();
    
        m_SubMenu.CreatePopupMenu();
        m_SubMenu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_GRAB,_T("as TIFF"));
        m_SubMenu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_SAVE,_T("as BMP"));

        m_SubMenu2.CreatePopupMenu();
        m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_0,_T("0"));
        m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_1,_T("1"));
        m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_2,_T("2"));
        m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_3,_T("3"));
        m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_4,_T("4"));
        if(lDepth > 12)
        {
            m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_5,_T("5"));
            m_SubMenu2.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_GAIN_6,_T("6"));
        }

        // create the pop-up menu and fill it
        m_Menu.CreatePopupMenu();
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_POPUP,(UINT)m_SubMenu.m_hMenu,_T("Save to disk ..."));
        m_Menu.AppendMenu(MF_ENABLED | MF_CHECKED | MF_STRING,ID_SIZE_INTERPOLATE,_T("Color Interpolation"));
#ifdef _ANCILLARY
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_SHOWANC,_T("Show ancillary data"));
#endif
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_SCALE,_T("Scale"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_POPUP,(UINT)m_SubMenu2.m_hMenu,_T("Digital gain ..."));
        m_Menu.AppendMenu(MF_SEPARATOR);
        m_Menu.AppendMenu(MF_ENABLED | MF_CHECKED   | MF_STRING,ID_SIZE_100,_T("100%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_090,_T("90%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_080,_T("80%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_070,_T("70%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_060,_T("60%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_050,_T("50%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_040,_T("40%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_030,_T("30%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_020,_T("20%"));
        m_Menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING,ID_SIZE_010,_T("10%"));

        // start streaming
        Start();
    }

    return result;
}

/*
 * Method:    OnCreateClient()
 * Purpose:   called by the framework during the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CFinderFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
    BOOL result;

    m_View = new CImageView;

    if(m_View)
    {
       result = m_View->Create(this,m_Info->InterfaceType);

       if(!result)
            m_View->DestroyWindow();
       #ifdef _ZOOM
       else
           m_View->m_Camera = m_Camera;
        #endif
    }

    return CChildFrame::OnCreateClient(lpcs,pContext);
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CFinderFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    // just call the parent's method
	return CChildFrame::PreCreateWindow(cs);
}

/*
 * Method:    OnEraseBkgnd()
 * Purpose:   called by the framework when the client area of
 *            the window should be erased
 * Comments:  none
 */
BOOL CFinderFrame::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

/*
 * Method:    OnSize()
 * Purpose:   called by the framework when the window was resized
 * Comments:  none
 */
void CFinderFrame::OnSize(UINT nType, int cx, int cy)
{
    // we resize the view as well   
    m_View->SetWindowPos(NULL,0,0,cx,cy,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE); 
    CChildFrame::OnSize(nType, cx, cy);
}

/*
 * Method:    OnTimer()
 * Purpose:   called by the framework when a timer event
 *            occurs
 * Comments:  none
 */
void CFinderFrame::OnTimer(UINT_PTR uId)
{
    EnterCriticalSection(&m_Protection);

    if(m_Streaming)
    {
        if(!m_FrameStack.empty())
        {   
            HandleFrameDone(m_FrameStack.top());
            m_FrameStack.pop();

            while(!m_FrameStack.empty())
            {
                PvCaptureQueueFrame(m_Camera,m_FrameStack.top(),FrameDoneCB);
                m_FrameStack.pop();
            }
        }
    }
    else
    {
        while(!m_FrameStack.empty())
            m_FrameStack.pop();
    }

    LeaveCriticalSection(&m_Protection);
}

/*
 * Method:    OnClose()
 * Purpose:   called by the framework when window is been
 *            closed
 * Comments:  none
 */
void CFinderFrame::OnShouldClose()
{
    // flag that the window is closing, so that no more
    // frame will be handled once returned by the callback
    m_Closing = TRUE;

    // stop streaming
    OnStop(0,0);
}

/*
 * Method:    OnClose()
 * Purpose:   called by the framework when window is been
 *            closed
 * Comments:  none
 */
void CFinderFrame::OnClose()
{
    // flag that the window is closing, so that no more
    // frame will be handled once returned by the callback
    m_Closing = TRUE;

    // stop streaming
    OnStop(0,0);

    // and call the parent's class method
    CChildFrame::OnClose();    
}

/*
 * Method:    OnDestroy()
 * Purpose:   called by the framework when window is been
 *            destroyed
 * Comments:  none
 */
void CFinderFrame::OnDestroy()
{
    // when the window is destroyed and not properly closed
    // (for example when the application is closed and that
    // the window is still on screen), ::OnClose() will not
    // be run, so we handle this case here was we still need
    // to stop the streaming
    EnterCriticalSection(&m_Protection);
    if(!m_Closing)
    {
        m_Closing = TRUE;
        LeaveCriticalSection(&m_Protection);
        // stop streaming (clean)
        OnStop(0,0);
    }
    else
        LeaveCriticalSection(&m_Protection);

    CChildFrame::OnDestroy();  
}

/*
 * Method:    OnContextMenu()
 * Purpose:   called by the framework when the user right
 *            clicked the mouse on the window
 * Comments:  none
 */
void CFinderFrame::OnContextMenu( CWnd* pWnd, CPoint pos )
{
    for(UINT i=0;i<7;i++)
        m_Menu.CheckMenuItem(ID_GAIN_0 + i,MF_BYCOMMAND | MF_UNCHECKED);

    m_Menu.CheckMenuItem(ID_GAIN_0 + m_View->GetGain(),MF_BYCOMMAND | MF_CHECKED);

    if(m_View->IsScaling())
        m_Menu.CheckMenuItem(ID_SIZE_SCALE,MF_BYCOMMAND | MF_CHECKED);
    else
        m_Menu.CheckMenuItem(ID_SIZE_SCALE,MF_BYCOMMAND | MF_UNCHECKED);

    if(m_View->IsInterpolate())
        m_Menu.CheckMenuItem(ID_SIZE_INTERPOLATE,MF_BYCOMMAND | MF_CHECKED);
    else  
        m_Menu.CheckMenuItem(ID_SIZE_INTERPOLATE,MF_BYCOMMAND | MF_UNCHECKED);

#ifdef _ANCILLARY
    if(m_View->IsShowAncillary())
        m_Menu.CheckMenuItem(ID_SIZE_SHOWANC,MF_BYCOMMAND | MF_CHECKED);
    else  
        m_Menu.CheckMenuItem(ID_SIZE_SHOWANC,MF_BYCOMMAND | MF_UNCHECKED);
#endif

    // simply display the popup-menu at the position of the mouse
    m_Menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pos.x, pos.y, this);
}

/*
 * Method:    OnGetMinMaxInfo()
 * Purpose:   called by the framework to retreive the min&max
 *            size of the window
 * Comments:  none
 */
void CFinderFrame::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
    CChildFrame::OnGetMinMaxInfo(lpMMI);

    lpMMI->ptMinTrackSize.x = 100;
    lpMMI->ptMinTrackSize.y = 150;
}

/*
 * Method:    OnContextCommand()
 * Purpose:   execute a command from the context menu
 * Comments:  none
 */
void CFinderFrame::OnContextCommand(UINT nID)
{
    if(nID == ID_SIZE_GRAB)
    {
        // lock the flag
        EnterCriticalSection(&m_Protection);
        // set the flag to true
        m_Grab = true;
        // unlock the flag
        LeaveCriticalSection(&m_Protection);        
    }
    else
    if(nID == ID_SIZE_SAVE)
    {
        CFileDialog* pDialog = NULL;
        char szTempName[512];  
        char lpPathBuffer[512];  
        UINT dwRetVal;
        UINT uRetVal;

        // Get the temp path.
        dwRetVal = GetTempPath(512,lpPathBuffer);
        if (dwRetVal > 512 || (dwRetVal == 0))
            return;

        // Create a temporary file. 
        uRetVal = GetTempFileName(lpPathBuffer,"NEW",0,szTempName); 
        if (uRetVal == 0)
            return;
  
        BeginWaitCursor(); 

        // lock the flag
        EnterCriticalSection(&m_Protection);       

        // save the bitmap rendering of the last frame in a temporary file
        if(m_View->SaveAsBitmap(szTempName))
        {
            // unlock the flag
            LeaveCriticalSection(&m_Protection);

            // block the window to make sure it will not be destroyed whiles
            // the dialog is been displayed (e.g when the camera is unplugged)
            Block();

            // create dialog
            pDialog = new CFileDialog(FALSE,_T("bmp"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilterBmp,NULL);

            // display dialog and wait for the user to select a file or cancel
            if(pDialog->DoModal() == IDOK )
            {
                CString lName = pDialog->GetPathName();

                RestoreWaitCursor();

                if(!lName.IsEmpty())
                {
                    // delete the file so that if we're overwritting it the MoveFile() it will work.
                    DeleteFile(lName);
                    if(!MoveFile(szTempName,lName))
                        DeleteFile(szTempName);
                }
                else
                    DeleteFile(szTempName);
            }
            else
                DeleteFile(szTempName);

            delete pDialog;
       
            // unblock now
            Unblock();
        }
        else
            LeaveCriticalSection(&m_Protection);

        EndWaitCursor();

        // and destroy it if needed
        DestroyIfRequested();
    }
    else
    if(nID == ID_SIZE_SCALE)
    {
        m_View->SetToScale(!m_View->IsScaling());
    }
    else
    if(nID == ID_SIZE_INTERPOLATE)
    {
        m_View->SetInterpolate(!m_View->IsInterpolate());
    }
    else
#ifdef _ANCILLARY
    if(nID == ID_SIZE_SHOWANC)
    {
        m_View->SetShowAncillary(!m_View->IsShowAncillary());
    }
    else
#endif
    if(nID >= ID_GAIN_0 && nID <= ID_GAIN_6)
    {
        m_View->SetGain(nID - ID_GAIN_0);
    }
    else
    {
        CRect rect;
        CRect orect;
        double fScale = 0;
        UINT nIndex = ID_SIZE_100;

        m_Menu.CheckMenuItem(m_LastCtxId,MF_BYCOMMAND | MF_UNCHECKED);
        m_Menu.CheckMenuItem(nID,MF_BYCOMMAND | MF_CHECKED);
        m_LastCtxId = nID;

        // if the window is zoomed so we unzoom it first
        if(IsZoomed())
            MDIRestore();

        // then we compute the size of the client area needed
        // according to what was selected by the user
        fScale = gScales[nID - nIndex];

        ResizeToFit((long)ceil((double)m_Width * fScale),(long)ceil((double)m_Height * fScale));
    }
}

/*
 * Method:    OnStart()
 * Purpose:   streaming must start
 * Comments:  none
 */
LRESULT CFinderFrame::OnStart(WPARAM wParam,LPARAM lParam)
{
    ULONG   Capturing;
    tPvErr  Err;

    // is the camera IDLE?
    if((Err = PvCaptureQuery(m_Camera, &Capturing)) == ePvErrSuccess)
    {
        if(!Capturing)
        {
            UINT i;
            static UINT lCount = 1;

            // start streaming
            if((Err = PvCaptureStart(m_Camera)) == ePvErrSuccess)
            {
                // get the number of bytes needed for each frame as well as the width&height
                if((Err = PvAttrUint32Get(m_Camera,"TotalBytesPerFrame",&m_BytesPerFrame)) == ePvErrSuccess)
                {
                   // ask the rendering view to reset as the streaming is been restarted
                    if(m_First)
                        m_First = false;
                    else 
                        m_View->Reset();

                    if(m_BytesPerFrame)
                    {
                        // allocate the image buffer for each of the frames
                        for(i=0;i<KMaxFrames && (Err == ePvErrSuccess);i++)
                        {
                            if(m_Frames[i].ImageBufferSize != m_BytesPerFrame)
                            {
                                // delete the image buffer in case we have stopped then restarted
                                delete m_Frames[i].ImageBuffer;
                                // then allocate the new one
                                m_Frames[i].ImageBuffer     = new BYTE[m_BytesPerFrame];
                                m_Frames[i].ImageBufferSize = m_BytesPerFrame;
                                
                                if(!m_Frames[i].ImageBuffer)
                                    Err = ePvErrResources;
                                else
                                {
                                    #ifdef _ANCILLARY
                                    if(!m_Frames[i].AncillaryBuffer)
                                    {
                                        m_Frames[i].AncillaryBuffer     = new BYTE[kMaxAncillary];
                                        m_Frames[i].AncillaryBufferSize = kMaxAncillary;

                                        if(!m_Frames[i].AncillaryBuffer)
                                        {
                                            delete m_Frames[i].ImageBuffer;
                                            Err = ePvErrResources;
                                        }
                                    }
                                    #endif

                                    m_Frames[i].Context[0]      = this;
                                    m_Frames[i].Context[1]      = (void*)lCount;
                                    m_Frames[i].Context[2]      = (void*)i;
                                }
                            }
                        }

                        lCount++;

                        if(Err == ePvErrSuccess)
                        {
                            // and the bandwith issue indicator
                            m_Bandwidth = false;
               
                            // and enqueue all the frames
                            for(i=0;i<KMaxFrames && (Err == ePvErrSuccess);i++)
                                Err = PvCaptureQueueFrame(m_Camera,&m_Frames[i],FrameDoneCB);
                
                            if(Err == ePvErrSuccess)
                            {
                                EnterCriticalSection(&m_Protection);
                                m_Streaming = true;
                                while(!m_FrameStack.empty())
                                    m_FrameStack.pop();
                                LeaveCriticalSection(&m_Protection);

                                // then force the acquisition mode to continuous to get the camera streaming
                                Err = PvCommandRun(m_Camera,"AcquisitionStart");

                                // if the camera wasn't open by us, the above command will fail with "ePvErrForbidden"
                                // which is normal.
                                if(Err && Err != ePvErrForbidden)
                                    ReportError(_T("Some error occured when starting acquisition"),Err);
                                else
                                {
                                    tCommand lCommand;

                                    lCommand.Uid = m_Info->UniqueId;
                                    lCommand.Cid = 0xA;
                                    lCommand.Ptr = "AcquisitionStart";

                                    // request attributes impact for AcquisitionStart to be refreshed
                                    SendCommandTo(eCtrl,&lCommand);

                                    // set the timer
                                    m_Timer = SetTimer(1,1000 / kFrameRate,NULL);
                                    // get the tick count now
                                    m_Was   = GetTickCount();
                                }
                            }
                            else
                            {
                                PvCaptureQueueClear(m_Camera);
                                ReportError(_T("Some error occured when enqueuing frames"),Err);
                            }
                        }
                        else
                            ReportError(_T("Failed to allocate the buffers"),Err);
                    }
                    else
                        ReportError(_T("Incorrect frame size"),Err);
                }
            }
            else
                ReportError(_T("Failed to start continuous capture"),Err);
        } 
        else
            ReportError(_T("Camera is not in IDLE mode"),Err);
    }
    else
        ReportError(_T("Failed to talk to the camera"),Err);

    return TRUE;
}

/*
 * Method:    OnStop()
 * Purpose:   streaming must stop
 * Comments:  none
 */
LRESULT CFinderFrame::OnStop(WPARAM wParam,LPARAM lParam)
{
    ULONG   Capturing;
    tPvErr  Err;

    // is the camera not IDLE?
    if((Err = PvCaptureQuery(m_Camera, &Capturing)) == ePvErrSuccess)
    {
        if(Capturing)
        {
            tCommand lCommand;

            // then force the acquisition mode to stopped to get the camera to stop streaming
            PvCommandRun(m_Camera,"AcquisitionStop");
            PvCaptureEnd(m_Camera);

            lCommand.Uid = m_Info->UniqueId;
            lCommand.Cid = 0xA;
            lCommand.Ptr = "AcquisitionStop";

            // request attributes impact for AcquisitionStop to be refreshed
            SendCommandTo(eCtrl,&lCommand);
        }
       
        EnterCriticalSection(&m_Protection);
        m_Streaming = false;
        LeaveCriticalSection(&m_Protection);

        // then dequeue all the frame still in the queue
        // in case there is any left in it and that the camera
        // was unplugged (we will ignore any error as the
        // capture was stopped anyway)
        PvCaptureQueueClear(m_Camera); 

        EnterCriticalSection(&m_Protection);
        while(!m_FrameStack.empty())
            m_FrameStack.pop();
        LeaveCriticalSection(&m_Protection);
       
        // kill the timer
        KillTimer(m_Timer);
    }
    else
    if(Err == ePvErrUnplugged) // the camera was unplugged while we were streaming
    {
        EnterCriticalSection(&m_Protection);
        m_Streaming = false;
        LeaveCriticalSection(&m_Protection);

        // we still need to dequeue all queued frames
        PvCaptureQueueClear(m_Camera);

        EnterCriticalSection(&m_Protection);
        while(!m_FrameStack.empty())
            m_FrameStack.pop();
        LeaveCriticalSection(&m_Protection);
        
        // kill the timer
        KillTimer(m_Timer);

#ifndef _ANALOG_TEST
        ReportError(_T("Failed to talk to the camera"),Err);
#endif
    }

    return TRUE;
}

/*
 * Method:    HandleFrameDone()
 * Purpose:   a frame have been captured
 * Comments:  none
 */
void CFinderFrame::HandleFrameDone(tPvFrame* pFrame)
{       
    if(pFrame->Status == ePvErrCancelled || !m_Streaming)
        return;

    if(pFrame->Status != ePvErrDataMissing)
    {
        BOOL bGrabbed  = false;
        BOOL bRendered = false;

        // else we will draw the frame if complete
        if(pFrame->Status == ePvErrSuccess)
        {
			//save a frames to a folder on the desktop
			CString lName = "C:/users/grfx/Desktop/plzwork.tif";

			//RestoreWaitCursor();

			if(!lName.IsEmpty())
			{
				// display wait cursor
				//BeginWaitCursor(); 
				// write frame and display error message if something went wrong
				//if(ImageWriteTiff(lName,pFrame) == false)
				//	ReportError(_T("Failed to save the grabbed frame!"));

				////add a text file========================================================================
				////========================================================================
				////========================================================================
				////========================================================================
				//if(1){
				//	//std::ofstream debugFile("C:/users/grfx/Desktop/debug_hp.txt");
				//	//debugFile.open("C:/users/grfx/Desktop/debug_hp.txt");
				//	//debugFile << "hello" << std::endl;
				//	//debugFile.close();
				//}
				//trackColor trackC(pFrame);
				//trackC.analyzeFrame();

			}
            // reset the bandwidth issue indicator
            m_Bandwidth = false;

            // then update the histogram if needed
            // (we use a lock to make sure m_Hist won't
            // be set to NULL after the if statement)
            if(m_Hist)
                m_Hist->Update(pFrame);

            // if the frame is to be grabbed and its a Bayer16, we won't render it so
            // that it can be saved without be altered for the rendering
            if((m_Grab && pFrame->Format != ePvFmtBayer16 && pFrame->Format != ePvFmtBayer12Packed) || !m_Grab)
            {
                // if the window is iconified, we won't render the frame,
                // to save CPU time
                if(!IsIconic() && IsWindowVisible())
                {
                    // we'll ask window to render the frame, but only if the
                    // frame was rendered
                    if(m_View->Update(pFrame))
                        bRendered = TRUE;
                }
            }

            // if the frame need to be grabbed, we do it now
            if(m_Grab && ((pFrame->Format != ePvFmtBayer16 && pFrame->Format != ePvFmtBayer12Packed) || !bRendered))
            {
                // reset the grab. indicator
                m_Grab = false;
                // then post a message for the window
                PostMessage(uLiveGrabbed,*(WPARAM *)&pFrame);
                // and set the flag to skip the enqueing for this frame
                bGrabbed = true;
            }
        }
        else
        if(pFrame->Status == ePvErrBandwidth)
        {
            if(!m_Bandwidth)
            {
                // set the flag to indicate that we have reported the
                // error
                m_Bandwidth = true;
                // then post a message to the window for it to
                // report the error to the user
                PostMessage(uLiveError,ePvErrBandwidth);
                // and stop the streaming
                Stop();
            }
        }

        // and push it back on the queue to be reused is there is no
        // bandwidth issue
        if(!m_Bandwidth && !bGrabbed)  
            PvCaptureQueueFrame(m_Camera,pFrame,FrameDoneCB);
    }
    else
    {
        // the frame couldn't be filled (too much data missing?), so we
        // re-enqueue it
        PvCaptureQueueFrame(m_Camera,pFrame,FrameDoneCB);
    }
}

/*
 * Method:    OnHist()
 * Purpose:   the histogram window is either to be updated
 *            or no longer
 * Comments:  none
 */
LRESULT CFinderFrame::OnHist(WPARAM wParam,LPARAM lParam)
{
    // protect the access to m_Hist as the CB from
    // PvAPI may try to use m_Hist at the same time
    m_Hist = *(CHistFrame**)&lParam;

    return NULL;
}

/*
 * Method:    OnError()
 * Purpose:   an error have occured, report it.
 * Comments:  none
 */
LRESULT CFinderFrame::OnError(WPARAM wParam,LPARAM lParam)
{
    ReportError(_T("Streaming error detected"),(tPvErr)wParam); 
    return NULL;
}

/*
 * Method:    OnGrabbed()
 * Purpose:   a frame have been grabbed, we need to
 *            save it then re-enqueue it
 * Comments:  none
 */
LRESULT CFinderFrame::OnGrabbed(WPARAM wParam,LPARAM lParam)
{
    CFileDialog* pDialog = NULL;
    tPvFrame *pFrame    = *(tPvFrame **)&wParam;

    BeginWaitCursor(); 

    // block the window to make sure it will not be destroyed whiles
    // the dialog is been displayed (e.g when the camera is unplugged)
    Block();

    if(pFrame->Format != ePvFmtYuv411 && pFrame->Format != ePvFmtYuv422 && pFrame->Format != ePvFmtYuv444)
    {
        // create dialog
        pDialog = new CFileDialog(FALSE,_T("tif"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter,NULL);

        // display dialog and wait for the user to select a file or cancel
        if(pDialog->DoModal() == IDOK )
        {
            CString lName = pDialog->GetPathName();

            RestoreWaitCursor();

            if(!lName.IsEmpty())
            {
                // display wait cursor
                //BeginWaitCursor(); 
                // write frame and display error message if something went wrong
                if(ImageWriteTiff(lName,pFrame) == false)
                    ReportError(_T("Failed to save the grabbed frame!"));
                // remove wait cursor
                //EndWaitCursor();
            }
        }

        delete pDialog;
    }
    else
        ReportError(_T("Sorry, YUV formated frame cannot be saved."));

    // if we are still streaming, we enqueue the frame
    if(m_Streaming && !m_Destroy)
        PvCaptureQueueFrame(m_Camera,pFrame,FrameDoneCB);

    // unblock now
    Unblock();

    EndWaitCursor();

    // and destroy it if needed
    DestroyIfRequested();
    
    return NULL;
}

/*
 * Function:  FrameDoneCB()
 * Purpose:   Callback from PvAPI related to the post handling of a frame
 * Comments:  none
 */
void __stdcall FrameDoneCB(tPvFrame* pFrame)
{
    CFinderFrame* Window = (CFinderFrame*)pFrame->Context[0];

    if(Window)
    {
        EnterCriticalSection(&Window->m_Protection);

        if(Window->m_Streaming)
            Window->m_FrameStack.push(pFrame);
        
        LeaveCriticalSection(&Window->m_Protection);
    }
}

