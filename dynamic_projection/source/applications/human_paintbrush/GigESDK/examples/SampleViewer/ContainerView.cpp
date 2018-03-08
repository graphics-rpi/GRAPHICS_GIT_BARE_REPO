/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ContainerView.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the Histogram window	
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
#include "ContainerView.h"

//===== CONSTANTS =============================================================

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CContainerView, CView)

// message map
BEGIN_MESSAGE_MAP(CContainerView, CView)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CContainerView()
 * Purpose:   class constructor
 * Comments:  none
 */
CContainerView::CContainerView()
{
}

/*
 * Method:    ~CContainerView()
 * Purpose:   class destructor
 * Comments:  none
 */  
CContainerView::~CContainerView()
{
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CContainerView::Create(ULONG nWidth,ULONG nHeight,CWnd* pParentWnd,UINT nID)
{
    return CView::Create(NULL,_T("cntview"),WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,
                         CRect(0,0,nWidth-1,nHeight-1),pParentWnd,nID);
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CContainerView::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CView::PreCreateWindow(cs))
    {
        // remove a couple of flags from the window's style
        return TRUE;
    }
    else
	    return FALSE;
}

/*
 * Method:    OnDraw()
 * Purpose:   called by the framework when the client area of
 *            the window must be redraw
 * Comments:  none
 */
void CContainerView::OnDraw( CDC* pDC ) 
{
	RECT				wndRect;
	CGdiObject*			pOldObject;

	GetClientRect(&wndRect);
	
	pOldObject = pDC->SelectStockObject(LTGRAY_BRUSH);
	pDC->PatBlt(0, 0, wndRect.right, wndRect.bottom, PATCOPY);
	pDC->SelectObject(pOldObject);
}

/*
 * Method:    OnEraseBkgnd()
 * Purpose:   called by the framework when the client area of
 *            the window should be erased
 * Comments:  none
 */
BOOL CContainerView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
