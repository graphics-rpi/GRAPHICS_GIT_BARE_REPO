/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        HistFrame.cpp		
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
#include "HistFrame.h"

//===== CONSTANTS =============================================================

// initial size of the window
const UINT KWinIniWidth  = 230;
const UINT KWinIniHeight = 100;
const UINT KPadWidth     = 5;
const UINT KPadHeight    = 5;

const UINT KLengthInBits = 8;
// max data in the histogram
const UINT KMaxHistData  = 1 << KLengthInBits;

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CHistFrame, CChildFrame)

// message map
BEGIN_MESSAGE_MAP(CHistFrame, CChildFrame)
    ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CHistFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CHistFrame::CHistFrame()
{
    m_Data = NULL;
}

/*
 * Method:    ~CHistFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CHistFrame::~CHistFrame()
{
    DeleteCriticalSection(&m_Lock);
    delete [] m_Data;
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CHistFrame::Create(tPvCameraInfoEx* pInfo)
{
    BOOL result;

    // initialize the locking mecanism
    try
    {
        InitializeCriticalSection(&m_Lock);
    } catch (...) {
        return FALSE;
    }

    // allocate the data array
    m_Data = new ULONG[KMaxHistData];
    if(m_Data)
    {
        // then call the parent's method to create the window
        if((result = CChildFrame::Create(pInfo,CChildFrame::eHist)))
        {
            // resize the window to the size we wish
            CRect rect;
            CRect orect;
 
            rect.left   = 0;
            rect.top    = 0;
            rect.bottom = KWinIniHeight + KPadHeight - 1;
            rect.right  = KWinIniWidth + KPadWidth - 1;

            // get the current window rect
            GetWindowRect(&orect);
            // and convert them to the parent coord.
            #ifdef _WIN64
            POINT P1 = orect.TopLeft();
            POINT P2 = orect.BottomRight();
            ::ScreenToClient(GetMDIFrame()->m_hWnd,&P1);
            ::ScreenToClient(GetMDIFrame()->m_hWnd,&P2);
            orect.SetRect(P1,P2);
            #else
            GetMDIFrame()->ScreenToClient(&orect);
            #endif
            // calculate the window size for the window area we wish
            CalcWindowRect(&rect);

            // them move/resize the window
            MoveWindow(orect.left,orect.top, rect.Width(), rect.Height());
        }
    }
    else
        result = FALSE;
    
    return result;
}

/*
 * Method:    Update()
 * Purpose:   update the histogram with a given frame
 * Comments:  none
 */ 
void CHistFrame::Update(tPvFrame* pFrame)
{
	const ULONG pixels = pFrame->Width * pFrame->Height;
	const ULONG bitshift = pFrame->BitDepth - KLengthInBits;

	//
	// Note: for now, this only works with monochrome and bayer images.
	//

    // lock to avoid modification of the data whiles we are using then
    // to redraw the window (::Update() is called from another thread)
    EnterCriticalSection(&m_Lock);

	// Clear old histogram.
	memset(m_Data, 0, sizeof(ULONG) * KMaxHistData);

	// Calculate new histogram.
	if(pFrame->Format == ePvFmtMono8 || pFrame->Format == ePvFmtBayer8) // 8 bits
	{
		unsigned char* pData = (unsigned char*) pFrame->ImageBuffer;
		const unsigned char* pDataEnd = pData + pixels;

		while (pData < pDataEnd)
		{
			m_Data[*pData]++;
			pData++;
		}
	}
	else // 16 bits
    if(pFrame->Format == ePvFmtMono16 || pFrame->Format == ePvFmtBayer16) 
	{
		unsigned short* pData = (unsigned short*) pFrame->ImageBuffer;
		const unsigned short* pDataEnd = pData + pixels;

		while (pData < pDataEnd)
		{
            if((*pData >> bitshift) <= KMaxHistData)
			    m_Data[*pData >> bitshift]++;
			pData++;
		}
	}

    // unlock
    LeaveCriticalSection(&m_Lock);

	// Trigger redraw.
	Invalidate();
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CHistFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CChildFrame::PreCreateWindow(cs))
    {
        // remove a couple of flags from the window's style
        cs.style &= ~WS_MAXIMIZEBOX;
        cs.style &= ~WS_MINIMIZEBOX;
        cs.style &= ~WS_THICKFRAME;

        return TRUE;
    }
    else
	    return FALSE;
}

/*
 * Method:    OnPaint()
 * Purpose:   called by the framework when the client area of
 *            the window must be redraw
 * Comments:  none
 */
void CHistFrame::OnPaint()
{
    // try to lock the "lock". if it is currently locked, we will
    // just not draw as another thread is currently updating the
    // data then requesting the window to be redraw ...
    if(TryEnterCriticalSection(&m_Lock))
    {    
	    CPaintDC			dc(this); // device context for painting
	    RECT				wndRect;
	    CPen				linePen;
	    CPen*				pOldPen;
	    ULONG				yDivisor;
	    int					xPos;
	    ULONG				dataMax;
	    int					i;

	    GetClientRect(&wndRect);

	    linePen.CreatePen(PS_SOLID, 0, RGB(0, 0, 200));

	    pOldPen = dc.SelectObject(&linePen);

	    // Find largest data value.
	    dataMax = 0;
	    for (i = 0; i < KMaxHistData; i++)
		    if (m_Data[i] > dataMax)
			    dataMax = m_Data[i];

	    // Calculate y divisor.  Each data point is divided by this value to fit
	    // in the drawing area.
	    yDivisor = (dataMax + wndRect.bottom - 1) / wndRect.bottom; // Round up
	    if (yDivisor == 0)
		    yDivisor = 1;

	    // Draw the histogram.

	    dc.MoveTo(0, wndRect.bottom - (m_Data[0] / yDivisor));

	    xPos = 0;
	    for (i = 0; i < KMaxHistData; i++)
	    {
		    int				yVal;

		    yVal = m_Data[i] / yDivisor;

		    // If data point is not zero, always show 
		    if (m_Data[i] && (yVal == 0))
			    yVal = 1;

		    dc.LineTo(xPos / KMaxHistData, wndRect.bottom - yVal);

		    xPos += wndRect.right;
	    }

	    dc.SelectObject(pOldPen);

        // unlock the lock so that the data can be refreshed
        LeaveCriticalSection(&m_Lock);
    }
}

/*
 * Method:    OnEraseBkgnd()
 * Purpose:   called by the framework when the client area of
 *            the window should be erased
 * Comments:  none
 */
BOOL CHistFrame::OnEraseBkgnd(CDC* pDC)
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
