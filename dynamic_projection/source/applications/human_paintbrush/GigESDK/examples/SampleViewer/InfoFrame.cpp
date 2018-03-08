/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        InfoFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the Info window	
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
#include "InfoFrame.h"

//===== DEFINES ===============================================================

//===== CONSTANTS =============================================================

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CInfoFrame, CChildFrame)

// message map
BEGIN_MESSAGE_MAP(CInfoFrame, CChildFrame)
END_MESSAGE_MAP()

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CInfoView
 * Purpose:  Derive the FormView to create a view to hold the various fields
 *           to be displayed by the window
 * Comments: none
 */
class CInfoView : public CFormView
{
    DECLARE_DYNCREATE(CInfoView)

    public:

        /*
         * Method:    CInfoView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CInfoView();

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

};

//===== CLASS IMPLEMENTATIONS =================================================

IMPLEMENT_DYNCREATE(CInfoView, CFormView)

/*
 * Method:    CInfoView()
 * Purpose:   class constructor
 * Comments:  none
 */
CInfoView::CInfoView()
    : CFormView(IDD_INFOVIEW)
{
}

/*
 * Method:    Create()
 * Purpose:   create the view
 * Comments:  none
 */
BOOL CInfoView::Create(const RECT& Rect, CWnd* pParent)
{
    return CFormView::Create(NULL,_T("infoview"),WS_CHILD | WS_VISIBLE,Rect,pParent,0,NULL);
}

/*
 * Method:    CInfoFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CInfoFrame::CInfoFrame()
{
}

/*
 * Method:    ~CInfoFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CInfoFrame::~CInfoFrame()
{
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CInfoFrame::Create(tPvCameraInfoEx* pInfo)
{
    BOOL result;

    if((result = CChildFrame::Create(pInfo,CChildFrame::eInfo)))
    {
        // create the view
        CInfoView* pView = new CInfoView;
        if(pView)
        {
            result = pView->Create(CRect(0,0,299,199),this);
            if(result)
            {
                CEditView* pEdit;
                CString String;

                // UID
                pEdit = (CEditView*)pView->GetDlgItem(IDC_INFOVIEW_UID);
                String.Format(_T("%u"),m_Info->UniqueId);
                pEdit->GetEditCtrl().ReplaceSel(String);

                // Serial number
                pEdit = (CEditView*)pView->GetDlgItem(IDC_INFOVIEW_SNUM);
                pEdit->GetEditCtrl().ReplaceSel(m_Info->SerialNumber);

                // part number
                pEdit = (CEditView*)pView->GetDlgItem(IDC_INFOVIEW_PNUM);
                pEdit->GetEditCtrl().ReplaceSel(m_Info->PartNumber);

                // interface type
                pEdit = (CEditView*)pView->GetDlgItem(IDC_INFOVIEW_INTY);
                if(m_Info->InterfaceType == ePvInterfaceFirewire)
                    pEdit->GetEditCtrl().ReplaceSel(_T("Firewire"));
                else
                    pEdit->GetEditCtrl().ReplaceSel(_T("Giga-Ethernet"));

                // interface id
                pEdit = (CEditView*)pView->GetDlgItem(IDC_INFOVIEW_INID);
                String.Format(_T("%u"),m_Info->InterfaceId);
                pEdit->GetEditCtrl().ReplaceSel(String);
            }
        }
        else
            result = false;
    }

    return result;
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CInfoFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CMDIChildWnd::PreCreateWindow(cs))
    {
        // remove a few flags from the window style
        cs.style &= ~WS_MAXIMIZEBOX;
        cs.style &= ~WS_MINIMIZEBOX;
        cs.style &= ~WS_THICKFRAME;

        // set the window size
        cs.cx = 300;
        cs.cy = 200;

        return TRUE;
    }
    else
	    return FALSE;
}