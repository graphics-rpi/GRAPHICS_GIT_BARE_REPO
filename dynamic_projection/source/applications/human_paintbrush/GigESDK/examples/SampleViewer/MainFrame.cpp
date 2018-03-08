/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        MainFrm.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the main frame class	
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

#include "MainFrame.h"
#include "HistFrame.h"
#include "ImageView.h"

#include <ImageLib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>

//===== CONSTANTS =============================================================

// commands sent to the main window by the cameras list window
const int KCmdShowInfo = 0xA;
const int KCmdShowCtrl = 0xB;
const int KCmdShowLive = 0xC;
const int KCmdShowHist = 0xD;
const int KCmdShowExpo = 0x1;
const int KCmdShowEvnt = 0x2;
const int KCmdShowSrIO = 0x3;

// child's event
const int KEvnClosed    = 0xA;
const int KEvnActivated = 0xB;

//===== LOCAL DATA ============================================================

// register a message ID for acting on a camera (sent to us by the ListFrame)
UINT uCameraAction      = ::RegisterWindowMessage(_T("CameraAction"));
UINT uCameraRemoval     = ::RegisterWindowMessage(_T("CameraRemoval"));

// register a message ID for child window event
UINT uChildEvent        = ::RegisterWindowMessage(_T("ChildEvent"));

// register a message ID for error reporting
UINT uErrorReport       = ::RegisterWindowMessage(_T("ErrorReport"));
// register a message ID for warning reporting
UINT uWarningReport     = ::RegisterWindowMessage(_T("WarningReport"));
// register a message ID for commands
UINT uCommandForwarding = ::RegisterWindowMessage(_T("CommandForwarding"));
// register a message ID for getting camera handle
UINT uCommandGetHandle = ::RegisterWindowMessage(_T("CommandGetHandle"));

static UINT BASED_CODE GIndicators[] = 
{
    ID_SEPARATOR
};

// filters for exporting setup to text file
static char BASED_CODE szFilter[]    = "TEXT Files (*.txt)|*.txt||";

// ??
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

// message map for the class
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)

    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_WM_QUERYENDSESSION()
    ON_REGISTERED_MESSAGE(uCameraAction,OnCameraAction)
    ON_REGISTERED_MESSAGE(uCameraRemoval,OnCameraRemoval)
    ON_REGISTERED_MESSAGE(uChildEvent,OnChildEvent)
    ON_REGISTERED_MESSAGE(uErrorReport,OnErrorReport)
    ON_REGISTERED_MESSAGE(uWarningReport,OnWarningReport)
    ON_REGISTERED_MESSAGE(uCommandForwarding,OnCommandForwarding)
    ON_REGISTERED_MESSAGE(uCommandGetHandle,OnQueryHandle)

END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CMainFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CMainFrame::CMainFrame()
{
    // load the icons we need from the resources
    m_IconInfo = AfxGetApp()->LoadIcon(IDI_ICON1); 
    m_IconCtrl = AfxGetApp()->LoadIcon(IDI_ICON2);
    m_IconLive = AfxGetApp()->LoadIcon(IDI_ICON3);
    m_IconHist = AfxGetApp()->LoadIcon(IDI_ICON6);
    m_IconAppl = AfxGetApp()->LoadIcon(IDI_ICON5);
    m_IconEvnt = AfxGetApp()->LoadIcon(IDI_ICON4);
    m_IconSrIO = AfxGetApp()->LoadIcon(IDI_ICON7);

    m_ListFrame = new CListFrame;
    m_Current   = 0;
}

/*
 * Method:    ~CMainFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CMainFrame::~CMainFrame()
{
}

/*
 * Method:    Show()
 * Purpose:   show the window
 * Comments:  none
 */
void CMainFrame::Show()
{
    SetIcon(m_IconAppl,true);
    ShowWindow(1);
	UpdateWindow();
 
    if(m_ListFrame)
    {
        if(m_ListFrame->Create(NULL,_T("Cameras")))
        {
	        m_ListFrame->ShowWindow(1);
	        m_ListFrame->UpdateWindow();
        }
        else
        {
            ReportError(_T("Failed to create the list window"));
            delete m_ListFrame;
        }
    }
    else
        ReportError(_T("Failed to create the list window"));
}

/*
 * Method:    IsThereAStream()
 * Purpose:   check is there is an ongoing streaming
 * Arguments: none
 * Return:    true or false
 * Comments:  none
 */
BOOL CMainFrame::IsThereAStream() const
{
    if(m_Current)
    {
        tCameraMap::const_iterator  Iter;
   
        Iter = m_Cameras.find(m_Current);
        if(Iter != m_Cameras.end())
            return Iter->second.pLive != NULL;
        else
            return false;
    }
    else
        return false;
}

/*
 * Method:    GrabNextFrame()
 * Purpose:   instruct the current stream to "grab" the next frame
 * Comments:  none
 */
void CMainFrame::GrabNextFrame(bool Setup)
{
    if(m_Current)
    {
        tCameraMap::const_iterator  Iter;
   
        Iter = m_Cameras.find(m_Current);

        if(Iter != m_Cameras.end())
            if(Iter->second.pLive)
                Iter->second.pLive->RequestGrab(Setup);
    }

}

/*
 * Method:    IsGrabNextFrame()
 * Purpose:   check if current stream is grabbing the next frame
 * Comments:  none
 */
BOOL CMainFrame::IsGrabNextFrame()
{
    if(m_Current)
    {
        tCameraMap::const_iterator  Iter;
   
        Iter = m_Cameras.find(m_Current);

        if(Iter != m_Cameras.end())
        {
            if(Iter->second.pLive)
                return Iter->second.pLive->IsGrabbing();
            else
                return false;
        }
        else
            return false;
    }
    else
        return false;
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    // modify cs if needed

    // and call the parent's method
	return CMDIFrameWnd::PreCreateWindow(cs);
}

/*
 * Method:    OnCreate()
 * Purpose:   called by the framework when the frame is created 
 * Comments:  none
 */
int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    int Status;

    Status = CMDIFrameWnd::OnCreate(lpCreateStruct);

    if(!Status)
    {
        if(!m_StatusBar.Create(this) || !m_StatusBar.SetIndicators(GIndicators,1))
            m_StatusBar.SetPaneInfo(0,ID_SEPARATOR,SBPS_STRETCH,100);
        else
            Status = 0;
    }

    return Status;
}

/*
 * Method:    OnClose()
 * Purpose:   called by the framework prior to closing the window
 * Comments:  none
 */
void CMainFrame::OnClose()
{
    tCameraMap::iterator Iter;

    // we loop over all the open camera and hide their live windows (if they have any)
    for(Iter = m_Cameras.begin();Iter!=m_Cameras.end();Iter++)
    {
        if(Iter->second.hCamera)
            if(Iter->second.pLive)
                Iter->second.pLive->ShowWindow(SW_HIDE);
    }

    // then we loop over all the open camera and close the windows
    for(Iter = m_Cameras.begin();Iter!=m_Cameras.end();Iter++)
    {
        if(Iter->second.hCamera)
        {
            // we close the live window if any
            if(Iter->second.pLive)
                Iter->second.pLive->Close();

            // if the hist window is open, we close it
            if(Iter->second.pHist)
                Iter->second.pHist->Close();

            // we close the control window if any
            if(Iter->second.pCtrl)
                Iter->second.pCtrl->Close();

            // else, if the info window is open, we close it
            if(Iter->second.pInfo)
                Iter->second.pInfo->MDIDestroy();

            // we close the event window if any
            if(Iter->second.pEvnt)
                Iter->second.pEvnt->Close();

            // we close the serial window if any
            if(Iter->second.pSrIO)
                Iter->second.pSrIO->Close();
        }
    }

    CMDIFrameWnd::OnClose(); 
}

/*
 * Method:    OnDestroy()
 * Purpose:   called by the framework prior to the destruction of
 *            the window. 
 * Comments:  none
 */
void CMainFrame::OnDestroy()
{
    tCameraMap::iterator Iter;

    // we loop over all the open camera and we close them
    for(Iter = m_Cameras.begin();Iter!=m_Cameras.end();Iter++)
    {
        if(Iter->second.hCamera)
            PvCameraClose(Iter->second.hCamera);
    }

    CMDIFrameWnd::OnDestroy();    
}

/*
 * Method:    OnQueryEndSession()
 * Purpose:   called by the framework when the user session is ending (restart or shutdown)
 * Comments:  none
 */
BOOL CMainFrame::OnQueryEndSession()
{
    tCameraMap::iterator Iter;

    // we loop over all the open camera and close the windows
    for(Iter = m_Cameras.begin();Iter!=m_Cameras.end();Iter++)
    {
        if(Iter->second.hCamera)
        {
            // we close the live window if any
            if(Iter->second.pLive)
                Iter->second.pLive->Close();

            // if the hist window is open, we close it
            if(Iter->second.pHist)
                Iter->second.pHist->Close();

            // we close the control window if any
            if(Iter->second.pCtrl)
                Iter->second.pCtrl->Close();

            // we close the event window if any
            if(Iter->second.pEvnt)
                Iter->second.pEvnt->Close();

            // we close the serial window if any
            if(Iter->second.pSrIO)
                Iter->second.pSrIO->Close();

            // else, if the info window is open, we close it
            if(Iter->second.pInfo)
                Iter->second.pInfo->MDIDestroy();
        }
    }

    return true;
}

/*
 * Method:    OnCameraAction()
 * Purpose:   handle an action on a given camera
 * Comments:  none
 */
LRESULT CMainFrame::OnCameraAction(WPARAM wParam,LPARAM lParam)
{
    tCameraMap::const_iterator  Iter;
    tPvErr                      Err = ePvErrSuccess;

    BeginWaitCursor();
    
    // if we don't know yet about this camera, we need
    // to create an entry for it in the map and get some
    // info on it
    Iter = m_Cameras.find(wParam);
    if(Iter == m_Cameras.end())
    {
        tCameraData sData;

        // clear the memory
        memset(&sData,0,sizeof(tCameraData));
        
        // get the camera's info, report an error if this fail
        if((Err = PvCameraInfoEx(wParam,&(sData.sInfo),sizeof(tPvCameraInfoEx))))
            ReportError(_T("Failed to get camera's info"),Err);
        else
        {
            // else we insert the camera in our map. As the map::insert() can
            // throw an exception if there is a memory issue, we will trap it
            // here and report the error
            try
            {
                m_Cameras.insert(tCameraMap::value_type(wParam,sData));

            } catch (...) {
    
                Err = ePvErrResources;
                ReportError(_T("The following error occured"),Err);
            }           
        }
    }

    // if there was no error (or if the camera was already known)
    // we can continue and perform the requested action
    if(Err == ePvErrSuccess)
    {
        switch(lParam)
        {
            case KCmdShowInfo:
            {
                DoShowInfo(wParam);
                break;
            }
            case KCmdShowCtrl:
            {
                DoShowCtrl(wParam);
                break;
            }
            case KCmdShowLive:
            {
                DoShowLive(wParam);
                break;
            }
            case KCmdShowHist:
            {
                DoShowHist(wParam);
                break;
            }
            case KCmdShowExpo:
            {
                DoShowExpo(wParam);
                break;
            }
            case KCmdShowEvnt:
            {
                DoShowEvnt(wParam);
                break;
            }
            case KCmdShowSrIO:
            {
                DoShowSrIO(wParam);
                break;
            }
        }
    }

    EndWaitCursor();

    return 0;    
}

/*
 * Method:    OnCameraRemoval()
 * Purpose:   handle the removal of a camera
 * Comments:  none
 */
LRESULT CMainFrame::OnCameraRemoval(WPARAM wParam,LPARAM lParam)
{
    tCameraMap::iterator  Iter;
    tPvErr                Err = ePvErrSuccess;

    // if we don't know about this camera, there is nothing
    // for us to do
    Iter = m_Cameras.find(wParam);
    if(Iter != m_Cameras.end())
    {
        CString lString = Iter->second.sInfo.SerialNumber;

        // else, if the hist window is open, we close it
        if(Iter->second.pHist)
            Iter->second.pHist->Close();

        // we close the control window if any
        if(Iter->second.pCtrl)
            Iter->second.pCtrl->Close();

        // we close the live window if any
        if(Iter->second.pLive)
            Iter->second.pLive->Close();

        // we close the event window if any
        if(Iter->second.pEvnt)
            Iter->second.pEvnt->Close();

        // we close the serial window if any
        if(Iter->second.pSrIO)
            Iter->second.pSrIO->Close();

        // else, if the info window is open, we close it
        if(Iter->second.pInfo)
            Iter->second.pInfo->MDIDestroy();
        
        // and finaly remove the camera from the map
        m_Cameras.erase(Iter);
    }

    return 0;
}

/*
 * Method:    OnChildEvent()
 * Purpose:   handle an event from a child window
 * Comments:  none
 */
LRESULT CMainFrame::OnChildEvent(WPARAM wParam,LPARAM lParam)
{
    switch(wParam)
    {
        case KEvnActivated:
        {
            m_Current = (tPvUint32)lParam;

            m_ListFrame->SelectCamera(lParam);
            break;
        }
        case KEvnClosed:
        {
            CMDIChildWnd* pChild = *(CMDIChildWnd**)&lParam; 
            tCameraMap::iterator Iter;

            // loop over all the known camera and set the associated window
            // (if any) to NULL
            for(Iter = m_Cameras.begin();Iter!=m_Cameras.end();Iter++)
            {
                if(Iter->second.pCtrl)
                    if(*(Iter->second.pCtrl) == *pChild)
                    {
                        Iter->second.pCtrl = NULL;
                        break;
                    }

                if(Iter->second.pLive)
                    if(*(Iter->second.pLive) == *pChild)
                    {
                        Iter->second.pLive = NULL;

                        // if the control window is open we indicate to
                        // it that there is no more live view
                        if(Iter->second.pCtrl)
                            Iter->second.pCtrl->SetLive(NULL);

                        // we close the hist window if it's open
                        if(Iter->second.pHist)
                            Iter->second.pHist->MDIDestroy();

                        break;
                    }

                if(Iter->second.pInfo)
                    if(*(Iter->second.pInfo) == *pChild)
                    {
                        Iter->second.pInfo = NULL;
                        break;
                    } 

                if(Iter->second.pEvnt)
                    if(*(Iter->second.pEvnt) == *pChild)
                    {
                        Iter->second.pEvnt = NULL;
                        break;
                    } 

                if(Iter->second.pSrIO)
                    if(*(Iter->second.pSrIO) == *pChild)
                    {
                        Iter->second.pSrIO = NULL;
                        break;
                    } 
            
                if(Iter->second.pHist)
                    if(*(Iter->second.pHist) == *pChild)
                    {
                        // if there is still a live window, we instruct it
                        // not to update the histogram anymore
                        if(Iter->second.pLive)
                            Iter->second.pLive->DoHistogram(NULL);

                        Iter->second.pHist = NULL;
                        break;
                    }
            } 

            // if there is no active window open for this camera, we
            // close it (e.g so that another applications could use the camera)
            if(!Iter->second.pCtrl && !Iter->second.pLive)
            {
                // then we close the camera handle if we had one
                if(Iter->second.hCamera)
                {
                    PvCameraClose(Iter->second.hCamera);
                    Iter->second.hCamera = NULL;
                }

                if(Iter->first == m_Current)
                    m_Current = 0;
            }
            
            break;
        }
    }

    return 0;
}

/*
 * Method:    OnErrorReport()
 * Purpose:   handle a request to report an error
 * Comments:  none
 */
LRESULT CMainFrame::OnErrorReport(WPARAM wParam,LPARAM lParam)
{
    const char* pString = *(const char**)&wParam;
    ReportError(pString,(tPvErr)lParam);
    return 0;
}

/*
 * Method:    OnWarningReport()
 * Purpose:   handle a request to report a warning
 * Comments:  none
 */
LRESULT CMainFrame::OnWarningReport(WPARAM wParam,LPARAM lParam)
{
    const char* pString = *(const char**)&wParam;
    ReportWarning(pString);
    return 0;
}

/*
 * Method:    OnCommandForwarding()
 * Purpose:   handle a request to forward a command
 * Comments:  none
 */
LRESULT CMainFrame::OnCommandForwarding(WPARAM wParam,LPARAM lParam)
{
    const tCommand* lCommand = (const tCommand*)lParam;

    switch(wParam)
    {
        case CChildFrame::eView:
        {
            if(m_Cameras[lCommand->Uid].pLive)
                m_Cameras[lCommand->Uid].pLive->OnCommandReceived(lCommand);  

            break;
        }
        case CChildFrame::eCtrl:
        case CChildFrame::eCtrlRO:
        {
            if(m_Cameras[lCommand->Uid].pCtrl)
                ((CChildFrame*)m_Cameras[lCommand->Uid].pCtrl)->OnCommandReceived(lCommand);  

            break;
        }
        default:
            break;
    }

    return 0;   
}

/*
 * Method:    OnQueryHandle()
 * Purpose:   handle a query for the camera
 * Comments:  none
 */
LRESULT CMainFrame::OnQueryHandle(WPARAM wParam,LPARAM lParam)
{
    tPvHandle* lHandle = (tPvHandle*)lParam;

    *lHandle = m_Cameras[wParam].hCamera;

    return 0;
}

/*
 * Method:    DoShowInfo()
 * Purpose:   display the Info window for a given camera
 * Comments:  none
 */
void CMainFrame::DoShowInfo(UINT lUID)
{
    // if there is already a live frame
    if(m_Cameras[lUID].pInfo)
    {
        // we bring it foreground  
        m_Cameras[lUID].pInfo->MDIActivate();     
    }
    else
    {
        // else we create it
        CInfoFrame* pFrame = new CInfoFrame;
        if(pFrame)
        {
            // and pass the camera handle + serial string to it   
            if(pFrame->Create(&(m_Cameras[lUID].sInfo)))
            {
                // almost ready, we just have to show it now
                pFrame->Show(m_IconInfo);
                // and store the window object in our camera map
                m_Cameras[lUID].pInfo = pFrame; 
            }
            else
            {
                delete pFrame;
                ReportError(_T("Failed to create the info window"),ePvErrSuccess);
            }
        }
        else
            ReportError(_T("The following error occured"),ePvErrResources);
    }
}

/*
 * Method:    DoShowCtrl()
 * Purpose:   display the Control window for a given camera
 * Comments:  none
 */
void CMainFrame::DoShowCtrl(UINT lUID)
{
    tPvErr Err;

    Err = DoCameraOpen(lUID);
    if(Err == ePvErrSuccess)
    {
        // if there is already a live frame
        if(m_Cameras[lUID].pCtrl)
        {
            // we bring it foreground  
            m_Cameras[lUID].pCtrl->MDIActivate();     
        }
        else
        {
            // else we create it
            CCtrlFrame* pFrame = new CCtrlFrame;
            if(pFrame)
            {
                // and pass the camera info to it   
                if(pFrame->Create(m_Cameras[lUID].hCamera,&(m_Cameras[lUID].sInfo),m_Cameras[lUID].bOwner))
                {
                    // if there is already a live view, we pass it
                    // down to the control view
                    if(m_Cameras[lUID].pLive)
                        pFrame->SetLive(m_Cameras[lUID].pLive);

                    // almost ready, we just have to show it now
                    pFrame->Show(m_IconCtrl);
                    // and store the window object in our camera map
                    m_Cameras[lUID].pCtrl = pFrame; 
                }
                else
                {
                    delete pFrame;
                    ReportError(_T("Failed to create the control window"));
                }
            }
            else
                ReportError(_T("The following error occured"),ePvErrResources);
        }
    }
    else
        ReportError(_T("Failed to open the camera"),Err); 
}

/*
 * Method:    DoShowLive()
 * Purpose:   display the Live window for a given camera
 * Comments:  none
 */
void CMainFrame::DoShowLive(UINT lUID)
{
    tPvErr Err;

    Err = DoCameraOpen(lUID);
    if(Err == ePvErrSuccess)
    {
        // if there is already a live frame
        if(m_Cameras[lUID].pLive)
        {
            // we bring it foreground  
            m_Cameras[lUID].pLive->MDIActivate();     
        }
        else
        {
            ULONG width,height;

            // get the current width&height of the image to size the window accordingly
            if(((Err = PvAttrUint32Get(m_Cameras[lUID].hCamera,"Width",&width)) == ePvErrSuccess) &&
               ((Err = PvAttrUint32Get(m_Cameras[lUID].hCamera,"Height",&height)) == ePvErrSuccess))
            {
                // else we create it
                CFinderFrame* pFrame = new CFinderFrame;
                if(pFrame)
                {
                    // and pass the camera info to it   
                    if(pFrame->Create(m_Cameras[lUID].hCamera,&(m_Cameras[lUID].sInfo),width,height))
                    {
                        // almost ready, we just have to show it now
                        pFrame->Show(m_IconLive);
                        // and store the window object in our camera map
                        m_Cameras[lUID].pLive = pFrame; 

                        // if there is already a control view, we pass the
                        // new live view to it
                        if(m_Cameras[lUID].pCtrl)
                            m_Cameras[lUID].pCtrl->SetLive(pFrame);

						//save a frames to a folder on the desktop
						CString lName = "C:/users/grfx/Desktop/new.tif";

						//RestoreWaitCursor();

						//if(!lName.IsEmpty())
						//{
						//	// display wait cursor
						//	//BeginWaitCursor(); 
						//	// write frame and display error message if something went wrong


						//	//if(ImageWriteTiff(lName,pFrame->getFrames()) == false)
						//		ReportError(_T("Failed to save the grabbed frame!"));
						//	/*std::fstream tfile;
						//	tfile.open("newthing.txt");
						//	tfile << pFrame->getFrames() << std::endl;
						//	tfile.close();*/


						//	//std::cout << pFrame->getFrames() << std::endl;


						//	// remove wait cursor
						//	//EndWaitCursor();
						//}
                    }
                    else
                    {
                        delete pFrame;
                        ReportError(_T("Failed to create the live window"));
                    }
                }
                else
                    ReportError(_T("The following error occured"),ePvErrResources);
            }
            else
               ReportError(_T("Failed to open the camera"),Err);
        }
    }
    else
        ReportError(_T("Failed to open the camera"),Err);
}

/*
 * Method:    DoShowHist()
 * Purpose:   display the Histogram window for a given camera
 * Comments:  none
 */
void CMainFrame::DoShowHist(UINT lUID)
{
     if(m_Cameras[lUID].pLive)
     {
        // if there is already a hist frame
        if(m_Cameras[lUID].pHist)
        {
            // we bring it foreground  
            m_Cameras[lUID].pHist->MDIActivate();     
        }
        else
        {
            // else we create it
            CHistFrame* pFrame = new CHistFrame;
            if(pFrame)
            {
                // and pass the camera info to it  
                if(pFrame->Create(&(m_Cameras[lUID].sInfo)))
                {
                    // almost ready, we just have to show it now
                    pFrame->Show(m_IconHist);
                    // and store the window object in our camera map
                    m_Cameras[lUID].pHist = pFrame; 
                    // inform the live window that it shall update the histogram
                    m_Cameras[lUID].pLive->DoHistogram(pFrame);
                }
                else
                {
                    delete pFrame;
                    ReportError(_T("Failed to create the info window"),ePvErrSuccess);
                }
            }
            else
                ReportError(_T("The following error occured"),ePvErrResources);
        }
     }
     else
        ReportError(_T("You must be streaming from this camera before opening the histogram"));
}

/*
 * Method:    DoShowExpo()
 * Purpose:   export the camera setup to a text file
 * Comments:  none
 */
void CMainFrame::DoShowExpo(UINT lUID)
{
    CFileDialog *Dialog = new CFileDialog(FALSE,_T("txt"),m_Cameras[lUID].sInfo.SerialNumber,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter,NULL);
    
    Dialog->m_ofn.lpstrTitle = _T("Exporting camera setup ...");
        
    // display dialog and wait for the user to select a file or cancel
    if(Dialog->DoModal() == IDOK )
    {
        CString lName = Dialog->GetPathName();

        if(!lName.IsEmpty())
        {
            tPvErr Err;

            // display wait cursor
            BeginWaitCursor(); 
           
            // open the camera (if it's not already open)
            if((Err = DoCameraOpen(lUID)) == ePvErrSuccess)
            {
                // export the camera setup to a text file
                DoCameraExport(lUID,lName);
    
                // close the camera if no one else has it open
                if(!m_Cameras[lUID].pLive && !m_Cameras[lUID].pCtrl)
                    DoCameraClose(lUID);         
            }
            else
                ReportError(_T("Failed to open the camera"),Err);

            // remove wait cursor
            EndWaitCursor();
        }
    }

    Dialog->m_ofn.lpstrTitle = NULL;

    delete Dialog;

}

/*
 * Method:    DoShowEvnt()
 * Purpose:   display the events window
 * Comments:  none
 */
void CMainFrame::DoShowEvnt(UINT lUID)
{
    tPvErr Err;

    Err = DoCameraOpen(lUID);
    if(Err == ePvErrSuccess)
    {
        // if there is already a frame
        if(m_Cameras[lUID].pEvnt)
        {
            // we bring it foreground  
            m_Cameras[lUID].pEvnt->MDIActivate();     
        }
        else
        {
            // else we create it
            CEventsFrame* pFrame = new CEventsFrame;
            if(pFrame)
            {
                // and pass the camera info to it   
                if(pFrame->Create(m_Cameras[lUID].hCamera,&(m_Cameras[lUID].sInfo)))
                {
                    // almost ready, we just have to show it now
                    pFrame->Show(m_IconEvnt);
                    // and store the window object in our camera map
                    m_Cameras[lUID].pEvnt = pFrame; 
                }
                else
                {
                    delete pFrame;
                    ReportError(_T("Failed to create the events window"));
                }
            }
            else
                ReportError(_T("The following error occured"),ePvErrResources);
        }
    }
    else
        ReportError(_T("Failed to open the camera"),Err);     
}

/*
 * Method:    DoShowSrIO()
 * Purpose:   display the serial window
 * Comments:  none
 */
void CMainFrame::DoShowSrIO(UINT lUID)
{
    tPvErr Err;

    Err = DoCameraOpen(lUID);
    if(Err == ePvErrSuccess)
    {
        // if there is already a frame
        if(m_Cameras[lUID].pSrIO)
        {
            // we bring it foreground  
            m_Cameras[lUID].pSrIO->MDIActivate();     
        }
        else
        {
            // else we create it
            CSerialFrame* pFrame = new CSerialFrame;
            if(pFrame)
            {
                // and pass the camera info to it   
                if(pFrame->Create(m_Cameras[lUID].hCamera,&(m_Cameras[lUID].sInfo)))
                {
                    // almost ready, we just have to show it now
                    pFrame->Show(m_IconSrIO);
                    // and store the window object in our camera map
                    m_Cameras[lUID].pSrIO = pFrame; 
                }
                else
                {
                    delete pFrame;
                    ReportError(_T("Failed to create the serial window"));
                }
            }
            else
                ReportError(_T("The following error occured"),ePvErrResources);
        }
    }
    else
        ReportError(_T("Failed to open the camera"),Err);
}

/*
 * Method:    DoCameraOpen()
 * Purpose:   open a given camera for control&imaging
 * Comments:  none
 */
tPvErr CMainFrame::DoCameraOpen(UINT lUID)
{
    tCameraData sData;
    
    // get the camera's data from the map
    sData = m_Cameras[lUID];

    // if we have an handle, then the camera
    // is already open
    if(!sData.hCamera)
    {
        tPvErr Err;

        // else we will try to open it
        Err = PvCameraOpen(lUID,ePvAccessMaster,&(sData.hCamera));
        if(Err == ePvErrSuccess)
        {
            //tPvUint32 MaxSize = 8228;

            m_Cameras[lUID].hCamera = sData.hCamera;  
            m_Cameras[lUID].bOwner  = true;

            // read the current packet size of the camera
            //PvAttrUint32Get(sData.hCamera,"PacketSize",&MaxSize);

            // then we adjust the packet size (max is the current stored value)
            //PvCaptureAdjustPacketSize(sData.hCamera,MaxSize);
        }
        else
        if(Err == ePvErrAccessDenied)
        {
            Err = PvCameraOpen(lUID,ePvAccessMonitor,&(sData.hCamera));
            if(Err == ePvErrSuccess)
            {
                m_Cameras[lUID].hCamera = sData.hCamera;  
                m_Cameras[lUID].bOwner  = false;
            }
        }

        return Err;
    }
    else
        return ePvErrSuccess;
}

/*
 * Method:    DoCameraClose()
 * Purpose:   close a given camera (previously open)
 * Comments:  none
 */
tPvErr CMainFrame::DoCameraClose(UINT lUID)
{
    tCameraData sData;
    
    // get the camera's data from the map
    sData = m_Cameras[lUID];

    // if we don't have an handle, then the camera
    // is already closed (or never opened)
    if(sData.hCamera)
    {
        tPvErr Err;

        // else we will try to close it
        Err = PvCameraClose(sData.hCamera);
        if(Err == ePvErrSuccess)
            m_Cameras[lUID].hCamera = NULL;  

        return Err;
    }
    else
        return ePvErrSuccess;    
}


// encode the value of a given attribute in a string
bool Value2String(tPvHandle aCamera,const char* aLabel,tPvDatatype aType,char* aString,unsigned long aLength)
{   
    switch(aType)
    {           
        case ePvDatatypeString:
        {   
            if(PvAttrStringGet(aCamera,aLabel,aString,aLength,NULL) == ePvErrSuccess)
                return true;
            else
                return false;     
        }
        case ePvDatatypeEnum:
        {            
            if(PvAttrEnumGet(aCamera,aLabel,aString,aLength,NULL) == ePvErrSuccess)
                return true;
            else
                return false;
        }
        case ePvDatatypeUint32:
        {
            tPvUint32 lValue;
            
            if(PvAttrUint32Get(aCamera,aLabel,&lValue) == ePvErrSuccess)
            {
                sprintf(aString,"%lu",lValue);
                return true;
            }
            else
                return false;
            
        }
        case ePvDatatypeFloat32:
        {
            tPvFloat32 lValue;
            
            if(PvAttrFloat32Get(aCamera,aLabel,&lValue) == ePvErrSuccess)
            {
                sprintf(aString,"%g",lValue);
                return true;
            }
            else
                return false;
        }
        default:
            return false;
    }        
}

// write a given attribute in a text file
void WriteAttribute(tPvHandle aCamera,const char* aLabel,FILE* aFile)
{
    tPvAttributeInfo lInfo;

    if(PvAttrInfo(aCamera,aLabel,&lInfo) == ePvErrSuccess)
    {
        if(lInfo.Datatype != ePvDatatypeCommand)
        {
            char lValue[128];
            
            if(Value2String(aCamera,aLabel,lInfo.Datatype,lValue,128))
                fprintf(aFile,"%s = %s\n",aLabel,lValue);           
        }   
    }
}


/*
 * Method:    DoCameraExport()
 * Purpose:   export the setup of a given camera
 * Comments:  none
 */
BOOL CMainFrame::DoCameraExport(UINT lUID,const char* Filename)
{
    FILE* lFile = fopen(Filename,"w+");
    
    if(lFile)
    {
        bool            lRet = true;
        tPvAttrListPtr  lAttrs; 
        tPvUint32       lCount;    
        
        if(PvAttrList(m_Cameras[lUID].hCamera,&lAttrs,&lCount) == ePvErrSuccess)
        {
            for(tPvUint32 i=0;i<lCount;i++)
                WriteAttribute(m_Cameras[lUID].hCamera,lAttrs[i],lFile);
        }     
        else
            lRet = false;   
        
        fclose(lFile);
        
        return lRet;
    }
    else
        return false;   
}

/*
 * Method:    ReportError()
 * Purpose:   report an error to the user
 * Comments:  none
 */
void CMainFrame::ReportError(const char* Text,tPvErr Error /* = ePvErrSuccess */)
{
    CString String;

    // concat the user supplied string
    String += Text;

    // if there is an error code, we add it
    if(Error)
    {
        CString SCode;

        String += _T(" : ");

        switch(Error)
        {
            case ePvErrSuccess:
            {
                String += "No error";
                break;
            }
            case ePvErrCameraFault:
            {
                String += "Unexpected camera fault";
                break;
            }
            case ePvErrInternalFault:
            {
                String += "Unexpected fault in PvApi or driver";
                break;
            }
            case ePvErrBadHandle:
            {
                String += "Camera handle is invalid";
                break;
            }
            case ePvErrBadParameter:
            {
                String += "Bad parameter to API call";
                break;
            }
            case ePvErrBadSequence:
            {
                String += "Sequence of API calls is incorrect";
                break;
            }
            case ePvErrNotFound:
            {
                String += "Camera or attribute not found";
                break;
            }
            case ePvErrAccessDenied:
            {
                String += "Camera cannot be opened in the specified mode";
                break;
            }
            case ePvErrUnplugged:
            {
                String += "Camera was unplugged";
                break;
            }
            case ePvErrInvalidSetup:
            {
                String += "Setup is invalid (an attribute is invalid)";
                break;
            }
            case ePvErrResources:
            {
                String += "System/network resources or memory not available";
                break;
            }
            case ePvErrBandwidth:
            {
                String += "1394 bandwidth not available";
                break;
            }
            case ePvErrQueueFull:
            {
                String += "Too many frames on queue";
                break;
            }
            case ePvErrBufferTooSmall:
            {
                String += "Frame buffer is too small";
                break;
            }
            case ePvErrCancelled:
            {
                String += "Frame cancelled by user";
                break;
            }
            case ePvErrDataLost:
            {
                String += "The data for the frame was lost";
                break;
            }
            case ePvErrDataMissing:
            {
                String += "Some data in the frame is missing";
                break;
            }
            case ePvErrTimeout:
            {
                String += "Timeout during wait";
                break;
            }
            case ePvErrOutOfRange:
            {
                String += "Attribute value is out of the expected range";
                break;
            }
            case ePvErrWrongType:
            {
                String += "Attribute is not this type (wrong access function) ";
                break;
            }
            case ePvErrForbidden:
            {
                String += "Attribute write forbidden at this time";
                break;
            }
            case ePvErrUnavailable:
            {
                String += "Attribute is not available at this time";
                break;
            }
            case ePvErrFirewall:
            {
                String += "Streaming port is blocked by the firewall";
                break;
            }
            default:
                String += "Unknown error";
        }
    }

    // then display the message box
    MessageBox(String, "Error ...", MB_ICONEXCLAMATION | MB_OK);
}

/*
 * Method:    ReportWarning()
 * Purpose:   report an error to the user
 * Comments:  none
 */
void CMainFrame::ReportWarning(const char* Text)
{
    CString String;

    // concat the user supplied string
    String += Text;

    // then display the message box
    MessageBox(String, "Warning ...", MB_ICONASTERISK | MB_OK);
}

/*
 * Method:    ReportLost()
 * Purpose:   report a lost of connection
 * Comments:  none
 */
void CMainFrame::ReportLost(const char* aSerial)
{
    CString String;

    String.LoadString(ID_MSGBOX_LOST);
  
    // then display the message box
    MessageBox(String, aSerial, MB_ICONEXCLAMATION | MB_OK);
}


