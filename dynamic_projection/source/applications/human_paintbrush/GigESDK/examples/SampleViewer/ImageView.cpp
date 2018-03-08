/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ImageView.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Render the live view
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
#include "ImageView.h"
#include "TrackColor.h"

#include <math.h>
#include <iostream>
using namespace std;

//===== CONSTANTS =============================================================

#define ULONG_PADDING(x) (((x+3) & ~3) - x)

//===== TYPES =================================================================

typedef struct {

    unsigned char LByte;
    unsigned char MByte;
    unsigned char UByte;

} Packed12BitsPixel_t;

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CImageView, CScrollView)

// message map
BEGIN_MESSAGE_MAP(CImageView, CScrollView)
	ON_WM_ERASEBKGND()
    ON_WM_SIZE()
#ifdef _ZOOM
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
#endif
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

// convert YUV to RGB
inline void YUV2RGB(int y,int u,int v,int& r,int& g,int& b)
{
   // u and v are +-0.5
   u -= 128;
   v -= 128;

   // Conversion (clamped to 0..255)
   r = min(max(0,(int)(y + 1.370705 * (float)v)),255);
   g = min(max(0,(int)(y - 0.698001 * (float)v - 0.337633 * (float)u)),255);
   b = min(max(0,(int)(y + 1.732446 * (float)u)),255);
}

/*
 * Method:    CImageView()
 * Purpose:   class constructor
 * Comments:  none
 */
CImageView::CImageView()
{
    m_InitDone      = FALSE;
    m_Buffer        = NULL;
    m_Width         = 0;
    m_Height        = 0;
    m_Left          = 0;
    m_Top           = 0;
    m_IsBgr         = FALSE;
    m_Scale         = FALSE;
    m_ShowAncillary = FALSE;
    m_Interpolate   = TRUE;
    m_Gain          = 0;
    #ifdef _ZOOM
    m_Mode          = 0;
    m_Down          = false;
    #endif
   
    // set the bitmap info
	m_BitmapInfo.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.bmiHeader.biPlanes			= 1;
	m_BitmapInfo.bmiHeader.biCompression	= BI_RGB;
	m_BitmapInfo.bmiHeader.biSizeImage		= 0;
	m_BitmapInfo.bmiHeader.biXPelsPerMeter	= 72;
	m_BitmapInfo.bmiHeader.biYPelsPerMeter	= 72;
	m_BitmapInfo.bmiHeader.biClrUsed		= 0;
	m_BitmapInfo.bmiHeader.biClrImportant	= 0;

	// the bmiColors array is used for monochrome display.
	for (int i = 0;
		 i < sizeof(m_BitmapInfo.bmiColors)/sizeof(m_BitmapInfo.bmiColors[0]);
		 i++)
	{
		m_BitmapInfo.bmiColors[i].rgbBlue       = i;
		m_BitmapInfo.bmiColors[i].rgbGreen      = i;
		m_BitmapInfo.bmiColors[i].rgbRed        = i;
		m_BitmapInfo.bmiColors[i].rgbReserved   = 0;
	}
}

/*
 * Method:    ~CImageView()
 * Purpose:   class destructor
 * Comments:  none
 */  
CImageView::~CImageView()
{
    // delete the internal buffer
    delete [] m_Buffer;
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CImageView::Create(CWnd* pParentWnd,tPvInterface Interface)
{
    #ifdef _ZOOM
    m_Interface = Interface;
    #endif

	cout << "look I got here how the fuckKK" << endl;

	m_Font.CreateFont(
    -11,                       // nHeight
    0,                         // nWidth
    0,                         // nEscapement
    0,                         // nOrientation
    FW_NORMAL,                 // nWeight
    FALSE,                     // bItalic
    FALSE,                     // bUnderline
    0,                         // cStrikeOut
    ANSI_CHARSET,              // nCharSet
    OUT_DEFAULT_PRECIS,        // nOutPrecision
    CLIP_DEFAULT_PRECIS,       // nClipPrecision
    DEFAULT_QUALITY,           // nQuality
    DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
    "Courier New");            // lpszFacename

    return CScrollView::Create(NULL,_T("cntview"),WS_VISIBLE,
                               CRect(0,0,0,0),pParentWnd,0);
}

/*
 * Method:    PreCreateWindow()
 * Purpose:   called by the framework prior to the creation of
 *            the window. 
 * Comments:  none
 */
BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CScrollView::PreCreateWindow(cs))
        return TRUE;
    else
	    return FALSE;
}

/*
 * Method:    SetToScale()
 * Purpose:   instruct the view to scale the image to fit
 *            in the window
 * Comments:  none
 */ 
void CImageView::SetToScale(BOOL aScale)
{
    m_Scale = aScale;

    AdaptToSize();
}

void CImageView::AdaptToSize()
{
    if(!m_Scale)
    {
        CRect image(0,0,m_Width,m_Height);
        CRect view(0,0,m_VWidth,m_VHeight);
        CPoint cimage = image.CenterPoint();
        CPoint cview  = view.CenterPoint();
        
        if(m_VWidth > m_Width)
            m_Left = cview.x - cimage.x;
        else
            m_Left = 0;
        if(m_VHeight > m_Height)
            m_Top   = cview.y - cimage.y;
        else
            m_Top   = 0;

        int lW = max(m_VWidth,m_Width) - 1;
        int lH = max(m_VHeight,m_Height) - 1;

        SetScrollSizes(MM_TEXT,CSize(max(0,lW),max(0,lH)));
    }
    else
    {
        m_Left = 0;
        m_Top  = 0;
        //SetScaleToFitSize(CSize(m_Width,m_Height));
        SetScrollSizes(MM_TEXT,CSize(m_VWidth,m_VHeight));
    }
}

/*
 * Method:    Reset()
 * Purpose:   do whatever need to be done when the streaming is re-started
 * Comments:  none
 */
void CImageView::Reset()
{
}

/*
 * Method:    SaveAsBitmap()
 * Purpose:   Save the bitmap in a bmp file
 * Comments:  none
 */
BOOL CImageView::SaveAsBitmap(const char* pFilename)
{
    if(m_BitmapInfo.bmiHeader.biBitCount == 8)
    {
        FILE*            pFile;
        BITMAPFILEHEADER Info; 
        char*            pMarker;
        BITMAPINFOHEADER Header = m_BitmapInfo.bmiHeader;

        // set the bitmap header
        Header.biBitCount = 24;  
        Header.biSizeImage = (((m_Width * 3) + 3) & ~3) * m_Height;

        // set the marker value in the info structure
	    pMarker = (char *)&(Info.bfType);
	    pMarker[0]='B';
	    pMarker[1]='M';

        // set the offset to the bitmap data
	    Info.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        // and the total size of the file
	    Info.bfSize      = Info.bfOffBits + Header.biSizeImage;
	    Info.bfReserved1 = 0;
        Info.bfReserved2 = 0;

        // open the file and write the data
        if((pFile = fopen(pFilename,"w+b")))
        {
            unsigned char lPixel[3];

            fwrite(&Info,sizeof(BITMAPFILEHEADER),1,pFile);
            fwrite(&Header,sizeof(BITMAPINFOHEADER),1,pFile);

            for(unsigned long i=0;i<m_BitmapInfo.bmiHeader.biSizeImage;i++)
            {
                memset(&lPixel,m_Buffer[i],3);
                fwrite(&lPixel,1,3,pFile);
            }
          
            fclose(pFile);

            return true;
        }
        else
            return false;
    }
    else
    {
        FILE*            pFile;
        BITMAPFILEHEADER Info; 
        char*            pMarker;

        // set the marker value in the info structure
	    pMarker = (char *)&(Info.bfType);
	    pMarker[0]='B';
	    pMarker[1]='M';

        // set the offset to the bitmap data
	    Info.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        // and the total size of the file
	    Info.bfSize      = Info.bfOffBits + m_BitmapInfo.bmiHeader.biSizeImage;
	    Info.bfReserved1 = 0;
        Info.bfReserved2 = 0;

        // open the file and write the data
        if((pFile = fopen(pFilename,"w+b")))
        {
            fwrite(&Info,sizeof(BITMAPFILEHEADER),1,pFile);
            fwrite(&m_BitmapInfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,pFile);
            fwrite(m_Buffer,1,m_BitmapInfo.bmiHeader.biSizeImage,pFile);
          
            fclose(pFile);

            return true;
        }
        else
            return false;
    }
}

/*
 * Method:    OnInitialUpdate()
 * Purpose:   called by the framework on the first update of the view
 * Comments:  none
 */
void CImageView::OnInitialUpdate()
{
    CScrollView::OnInitialUpdate();

    SetScrollSizes(MM_TEXT,CSize(10,10));
    m_InitDone = TRUE;
}

void CImageView::OnPrepareDC( CDC* pDC, CPrintInfo* pInfo /*= NULL*/ )
{
    if(!m_InitDone)
    {
        OnInitialUpdate();
    }

    SetStretchBltMode(pDC->m_hDC,COLORONCOLOR);
    CScrollView::OnPrepareDC(pDC,pInfo);
}

/*
 * Method:    OnDraw()
 * Purpose:   called by the framework when the client area of
 *            the window must be redraw
 * Comments:  none
 */
void CImageView::OnDraw( CDC* pDC )
{
    CRect Rect;

    Rect.left = 0;
    Rect.top  = 0;
    Rect.right = 100;
    Rect.bottom = 100;

    if(!m_Scale)
    ::StretchDIBits(pDC->m_hDC,
                    m_Left,
                    m_Top,
                    m_Width,
                    m_Height,
                    0,
                    0,
                    m_Width,
                    m_Height,
                    m_Buffer,
				    (BITMAPINFO*)&m_BitmapInfo,
			        DIB_RGB_COLORS,
				    SRCCOPY);
    else
    ::StretchDIBits(pDC->m_hDC,
                    0,
                    0,
                    m_VWidth,
                    m_VHeight,
                    0,
                    0,
                    m_Width,
                    m_Height,
                    m_Buffer,
				    (BITMAPINFO*)&m_BitmapInfo,
			        DIB_RGB_COLORS,
				    SRCCOPY);

    if(m_ShowAncillary)
    {
        SetBkMode(pDC->m_hDC, TRANSPARENT); 
        SetTextColor(pDC->m_hDC,RGB(255, 0, 0));
        SelectObject(pDC->m_hDC,m_Font);
        ::DrawText(pDC->m_hDC,m_Ancillary,-1,&Rect,DT_CALCRECT | DT_SINGLELINE);
        Rect.OffsetRect(2,2);
        ::DrawText(pDC->m_hDC,m_Ancillary,-1,&Rect,DT_SINGLELINE);
    }

}

/*
 * Method:    OnEraseBkgnd()
 * Purpose:   called by the framework when the client area of
 *            the window should be erased
 * Comments:  none
 */
BOOL CImageView::OnEraseBkgnd(CDC* pDC) 
{
    if(!m_Scale)
    {
	    RECT		wndRect;
	    CGdiObject* pOldObject;
        CRect       rect(m_Left,m_Top,m_Left + m_Width,m_Top + m_Height);
        CRgn        region;

	    GetClientRect(&wndRect);
        region.CreateRectRgnIndirect(&rect);
	    
        // simply fill the client area with the black color
	    pOldObject = pDC->SelectStockObject(BLACK_BRUSH);
        pDC->SelectClipRgn(&region,RGN_XOR);
	    pDC->PatBlt(0, 0, wndRect.right, wndRect.bottom, PATCOPY);
        pDC->SelectClipRgn(&region,RGN_OR);
	    pDC->SelectObject(pOldObject);
    }

    return TRUE;
}

#ifdef _ZOOM
void CImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
    {
        case 0x5A: // zoom
        {
            m_Mode = 1;
            break;
        }
        case 0x46: // focus
        {
            m_Mode = 2;
            break;
        }
        case 0x49: // iris
        {
            m_Mode = 3;
            break;
        }
        case VK_ADD:
        {
            if(!m_Down)
            {
                switch(m_Mode)
                {
                    case 1:
                    {
                        OutputDebugString("start zoom\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",16);
                        break;
                    }
                    case 2:
                    {
                        OutputDebugString("start focus\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",4);
                        break;
                    }
                    case 3:
                    {
                        OutputDebugString("start iris\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",1);
                        break;
                    }
                }

                m_Down = true;
            }
        }
        case VK_SUBTRACT:
        {
            if(!m_Down)
            {
                switch(m_Mode)
                {
                    case 1:
                    {
                        OutputDebugString("start zoom\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",32);
                        break;
                    }
                    case 2:
                    {
                        OutputDebugString("start focus\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",8);
                        break;
                    }
                    case 3:
                    {
                        OutputDebugString("start iris\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",2);
                        break;
                    }
                }

                m_Down = true;
            }
         }
    }
}

void CImageView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(m_Down)
    {
        switch(nChar)
        {
            case 0x5A: // zoom
            case 0x46: // focus
            case 0x49: // iris
            {
                switch(m_Mode)
                {
                    case 1:
                    {
                        OutputDebugString("stop zoom\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",0);
                        break;
                    }
                    case 2:
                    {
                        OutputDebugString("stop focus\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",0);
                        break;
                    }
                    case 3:
                    {
                        OutputDebugString("stop iris\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",0);
                        break;
                    }
                    default:
                        break;
                }

                m_Mode = 0;
                break;
            }
            case VK_ADD:
            case VK_SUBTRACT:
            {
                switch(m_Mode)
                {
                    case 1:
                    {
                        OutputDebugString("stop zoom\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",0);
                        break;
                    }
                    case 2:
                    {
                        OutputDebugString("stop focus\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",0);
                        break;
                    }
                    case 3:
                    {
                        OutputDebugString("stop iris\n");
                        PvAttrUint32Set(m_Camera,"EngineeringTest3",0);
                        break;
                    }
                    default:
                        break;
                }

                m_Down = false;

                break;
            }
        }  
    }
}

#endif

/*
 * Method:    OnSize()
 * Purpose:   called by the framework when the window was resized
 * Comments:  none
 */
void CImageView::OnSize(UINT nType, int cx, int cy)
{
    m_VWidth  = cx;
    m_VHeight = cy;

    AdaptToSize();

    Invalidate(TRUE);
        
    CScrollView::OnSize(nType, cx, cy);
}

void  CImageView::SetGain(ULONG Value)
{
    m_Gain = Value;
}

ULONG  CImageView::GetGain()
{
    return m_Gain;
}

/*
 * Method:    Update()
 * Purpose:   update the rendering with a given frame
 * Comments:  none
 */ 
BOOL CImageView::Update(tPvFrame* pFrame)
{
	BOOL  isBgr = false;
    ULONG pixelSize;
    BOOL  bClear = false;
    ULONG remainder;

	// color, or monochrome?
	if ((pFrame->Format == ePvFmtBayer8) || (pFrame->Format == ePvFmtBayer16) ||
		(pFrame->Format == ePvFmtBgr24)  || (pFrame->Format == ePvFmtRgb24)   ||
        (pFrame->Format == ePvFmtBgra32) || (pFrame->Format == ePvFmtRgba32)  ||
        (pFrame->Format == ePvFmtRgb48)  || (pFrame->Format == ePvFmtYuv411)  ||
        (pFrame->Format == ePvFmtYuv422) || (pFrame->Format == ePvFmtYuv444)  || (pFrame->Format == ePvFmtBayer12Packed))
		isBgr = true;
	else
	    isBgr = false;

    pixelSize = (isBgr) ? 3 : 1;

    // if we don't have a buffer yet, or if the frame size have changed, we need
    // to allocate a new image buffer
    if(!m_Buffer || m_Width != pFrame->Width || m_Height != pFrame->Height || (isBgr != m_IsBgr))
    {
		delete [] m_Buffer;

		m_Width      = pFrame->Width;
		m_Height     = pFrame->Height;
        m_BufferSize = (((m_Width * pixelSize) + 3) & ~3) * m_Height;
		m_Buffer     = new BYTE[m_BufferSize];
        m_BufferEnd  = m_Buffer + m_BufferSize;
        
        m_IsBgr  = isBgr;

	    m_BitmapInfo.bmiHeader.biWidth	   = m_Width;
	    m_BitmapInfo.bmiHeader.biHeight	   = 0 - m_Height;
	    m_BitmapInfo.bmiHeader.biBitCount  = m_IsBgr ? 24 : 8;  
        m_BitmapInfo.bmiHeader.biSizeImage = m_BufferSize;
        
        AdaptToSize();

        bClear = true;
    }

	trackColor tc(pFrame);
	tc.sendStr("got to top");
	// Copy the frame in our buffer.  (Windows requires that each line be ULONG aligned.)

	switch(pFrame->Format)
    {
        case ePvFmtMono8:
	    {
		    const BYTE*			pSrc = (BYTE*)pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + (m_Width * m_Height);
		    BYTE*				pDest = m_Buffer;
		    
            remainder = ULONG_PADDING(m_Width);

		    while (pSrc < pSrcEnd)
		    {
			    memcpy(pDest, pSrc, m_Width);
			    pSrc  += m_Width;
			    pDest += m_Width + remainder;
		    }

			tc.sendStr("Mono8");
            break;
	    }
        case ePvFmtMono16:
	    {
		    const USHORT*		pSrc = (USHORT*)pFrame->ImageBuffer;
		    const USHORT*		pSrcEnd = pSrc + (m_Width * m_Height);
		    BYTE*				pDest = m_Buffer;		    
		    const BYTE			bitshift = (BYTE)pFrame->BitDepth - 8;

            remainder = ULONG_PADDING(m_Width);

		    while (pSrc < pSrcEnd)
		    {
			    for (ULONG i = 0; i < m_Width; i++)
				    *(pDest++) = *(pSrc++) >> (bitshift - m_Gain);
             
			    pDest += remainder;
		    }

			tc.sendStr("Mono16");
            break;
	    }

		//BAYER 8
		//HUMAN PAINTBRUSH CAMERA============================================================================
		//===================================================================================================
		//===================================================================================================
		//===================================================================================================
		//===================================================================================================
		//===================================================================================================
		//===================================================================================================
        case ePvFmtBayer8:
	    {
            if(m_Interpolate)
            {
		        remainder = ULONG_PADDING(m_Width * pixelSize);

		        PvUtilityColorInterpolate(pFrame, &m_Buffer[2], &m_Buffer[1], &m_Buffer[0], 2, remainder);

				tc.TrackSetup(m_Buffer);
				
				//tc.normalize(m_Buffer);
				//tc.extractBlue(m_Buffer);
				//for(int i=0; i<pFrame->ImageBufferSize*3; i+=3){
				//	//tc.sendDouble(m_Buffer[i]);
				//	if(m_Buffer[i] < m_Buffer[i+1] || m_Buffer[i] < m_Buffer[i+2]){
				//		//m_Buffer[i] = 0;
				//	}else{
				//		//m_Buffer[i] = 255;
				//	}
				//	//m_Buffer[i+1] = 0;
				//	//m_Buffer[i+2] = 0;
				//}
            }
            else
            {
		        const BYTE*			pSrc = (BYTE*)pFrame->ImageBuffer;
		        const BYTE*			pSrcEnd = pSrc + pFrame->ImageBufferSize;
		        BYTE*				pDest = m_Buffer;
    		    
                remainder = ULONG_PADDING(m_Width * 3);

		        while (pSrc < pSrcEnd)
		        {
			        for (ULONG i = 0; i < m_Width; i++)
			        {
				        *(pDest++) = *pSrc;
				        *(pDest++) = *pSrc;
				        *(pDest++) = *(pSrc++);
			        }

			        pDest += remainder;
		        }
            }

			tc.sendStr("Bayer8");
            break;
	    }
        case ePvFmtBayer16:
	    {
            if(m_Interpolate)
            {
		        //
		        // Convert to 8 bits.
		        //
		        const USHORT*		pSrc = (USHORT*)pFrame->ImageBuffer;
		        const USHORT*		pSrcEnd = pSrc + (m_Width * m_Height);
		        BYTE*				pDest = (BYTE*)pFrame->ImageBuffer;		    
		        const BYTE			bitshift = (BYTE)pFrame->BitDepth - 8;

                remainder = ULONG_PADDING(m_Width * pixelSize);

		        while (pSrc < pSrcEnd)
			        *(pDest++) = *(pSrc++) >> (bitshift - m_Gain);

		        pFrame->Format = ePvFmtBayer8;

		        PvUtilityColorInterpolate(pFrame, &m_Buffer[2], &m_Buffer[1], &m_Buffer[0], 2, remainder);          

                pFrame->Format = ePvFmtBayer16;
            }
            else
            {
		        const USHORT*		pSrc = (USHORT*)pFrame->ImageBuffer;
		        const USHORT*		pSrcEnd = pSrc + (m_Width * m_Height);
		        BYTE*				pDest = m_Buffer;
                const BYTE			bitshift = (BYTE)pFrame->BitDepth - 8;
    		    
                remainder = ULONG_PADDING(m_Width * 3);

		        while (pSrc < pSrcEnd)
		        {
			        for (ULONG i = 0; i < m_Width; i++)
			        {
                        pDest[0] = pDest[1] = pDest[2] = *(pSrc++) >> (bitshift - m_Gain);
                        pDest += 3;
			        }

			        pDest += remainder;
		        }
            }

			tc.sendStr("Bayer16");
            break;
	    }
        case ePvFmtRgb24:
	    {
		    const BYTE*			pSrc = (BYTE*)pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + pFrame->ImageBufferSize;
		    BYTE*				pDest = m_Buffer;
		    
            remainder = ULONG_PADDING(m_Width * 3);

		    while (pSrc < pSrcEnd)
		    {
			    for (ULONG i = 0; i < m_Width; i++)
			    {
				    const BYTE	red     = *(pSrc++);
				    const BYTE	green   = *(pSrc++);
				    const BYTE	blue    = *(pSrc++);

				    *(pDest++) = blue;
				    *(pDest++) = green;
				    *(pDest++) = red;
			    }
			    pDest += remainder;
		    }

			tc.sendStr("rgb24");

            break;
	    }
        case ePvFmtBgr24:
	    {
		    const BYTE*			pSrc = (BYTE*)pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + pFrame->ImageBufferSize;
		    BYTE*				pDest = m_Buffer;		   
            const ULONG         rowlen = m_Width * 3;
            
            remainder = ULONG_PADDING(rowlen);

		    while (pSrc < pSrcEnd)
		    {                        
                memcpy(pDest,pSrc,rowlen);
                pSrc += rowlen;
			    pDest += rowlen + remainder;  
		    }


			tc.sendStr("bgr24");

            break;
	    }
        case ePvFmtRgba32:
	    {
		    const BYTE*			pSrc = (BYTE*)pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + pFrame->ImageBufferSize;
		    BYTE*				pDest = m_Buffer;
		    
            remainder = ULONG_PADDING(m_Width * 3);

		    while (pSrc < pSrcEnd && pDest < m_BufferEnd)
		    {
			    for (ULONG i = 0; i < m_Width && pDest < m_BufferEnd; i++)
			    {
				    const BYTE	red     = *(pSrc++);
				    const BYTE	green   = *(pSrc++);
				    const BYTE	blue    = *(pSrc++);

                    // skip the alpha
                    pSrc++;

                    if(pDest < m_BufferEnd)
                    {
				        *(pDest++) = blue;
                        if(pDest < m_BufferEnd)
                        {
				            *(pDest++) = green;
                            if(pDest < m_BufferEnd)
				                *(pDest++) = red;
                        }
                    }
			    }
			    pDest += remainder;
		    }

			tc.sendStr("RGBA32");

            break;
	    }
        case ePvFmtBgra32:
	    {
		    const BYTE*			pSrc = (BYTE*)pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + pFrame->ImageBufferSize;
		    BYTE*				pDest = m_Buffer;
		    
            remainder = ULONG_PADDING(m_Width * 3);

		    while (pSrc < pSrcEnd && pDest < m_BufferEnd)
		    {
			    for (ULONG i = 0; i < m_Width && pDest < m_BufferEnd; i++)
			    {
                    if(pDest < m_BufferEnd)
                    {
				        *(pDest++) = *(pSrc++);
				        if(pDest < m_BufferEnd)
                        {
                            *(pDest++) = *(pSrc++);
                            if(pDest < m_BufferEnd)
				                *(pDest++) = *(pSrc++);
                        }
                    }

                    // skip the alpha
                    pSrc++;
			    }
			    pDest += remainder;
		    }

			tc.sendStr("BGRA32");
            break;
	    }
        case ePvFmtRgb48:
	    {
		    const USHORT*		pSrc = (USHORT*)pFrame->ImageBuffer;
		    const USHORT*		pSrcEnd = (const USHORT*)((const BYTE*)pSrc + pFrame->ImageBufferSize);
		    BYTE*				pDest = m_Buffer;		    
		    const BYTE			bitshift = (BYTE)pFrame->BitDepth - 8;

            remainder = ULONG_PADDING(m_Width * 3);

		    while (pSrc < pSrcEnd && pDest < m_BufferEnd)
		    {
			    for (ULONG i = 0; i < m_Width && pDest < m_BufferEnd; i++)
			    {
				    const BYTE	red     = *(pSrc++) >> (bitshift - m_Gain);
				    const BYTE	green   = *(pSrc++) >> (bitshift - m_Gain);
				    const BYTE	blue    = *(pSrc++) >> (bitshift - m_Gain);

                    if(pDest < m_BufferEnd)
                    {
				        *(pDest++) = blue;
                        if(pDest < m_BufferEnd)
                        {
				            *(pDest++) = green;
				            if(pDest < m_BufferEnd)
                                *(pDest++) = red;
                        }
                    }
			    }
			    pDest += remainder;
		    }

			tc.sendStr("RGB48");

            break;
	    }
        case ePvFmtYuv411:
        {
		    const BYTE*			pSrc = (BYTE*) pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + (unsigned int)(m_Width * m_Height * 1.5);
		    BYTE*				pDest = m_Buffer;
		    
            if(m_Interpolate)
            {
                int y1,y2,y3,y4,u,v;
                int r,g,b;

		        while (pSrc < pSrcEnd)
		        {
                    u  = pSrc[0];
                    y1 = pSrc[1];
                    y2 = pSrc[2];
                    v  = pSrc[3];
                    y3 = pSrc[4];
                    y4 = pSrc[5]; 
                    pSrc+=6;

                    YUV2RGB(y1,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
                    YUV2RGB(y2,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
                    YUV2RGB(y3,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
                    YUV2RGB(y4,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
		        } 
            }
            else
            {
                int y1,y2,y3,y4;

		        while (pSrc < pSrcEnd)
		        {                 
                    y1 = pSrc[1];
                    y2 = pSrc[2];
                    y3 = pSrc[4];
                    y4 = pSrc[5]; 
                    pSrc+=6;

                    pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y1,255));
                    pDest+=3;
                    if(pDest < m_BufferEnd)
                    {
                        pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y2,255));
                        pDest+=3;
                        if(pDest < m_BufferEnd)
                        {
                            pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y3,255));
                            pDest+=3;
                            if(pDest < m_BufferEnd)
                            {
                                pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y4,255));
                                pDest+=3;
                            }
                        }
                    }
		        } 
            }

			tc.sendStr("yuv411");
            break;
        }
        case ePvFmtYuv422:
        {
		    const BYTE*			pSrc = (BYTE*) pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + (m_Width * m_Height * 2);
		    BYTE*				pDest = m_Buffer;
		    
            if(m_Interpolate)
            {
                int y1,y2,u,v;
                int r,g,b;

		        while (pSrc < pSrcEnd)
		        {
                    u  = pSrc[0];
                    y1 = pSrc[1];
                    v  = pSrc[2];
                    y2 = pSrc[3];
                    pSrc+=4;

                    YUV2RGB(y1,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
                    YUV2RGB(y2,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
		        }
            }
            else
            {
                int y1,y2;

		        while (pSrc < pSrcEnd && pDest < m_BufferEnd)
		        {                 
                    y1 = pSrc[1];
                    y2 = pSrc[3];
                    pSrc+=4;

                    pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y1,255));
                    pDest+=3;

                    if(pDest < m_BufferEnd)
                    {
                        pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y2,255));
                        pDest+=3;
                    }
		        }
            }

			tc.sendStr("yuv422");            
            break;
        }
        case ePvFmtYuv444:
        {
		    const BYTE*			pSrc = (BYTE*) pFrame->ImageBuffer;
		    const BYTE*			pSrcEnd = pSrc + (m_Width * m_Height * 3);
		    BYTE*				pDest = m_Buffer;
		    
            if(m_Interpolate)
            {
                int y2,y1,u,v;
                int r,g,b;

                r = b = g = 0;

		        while (pSrc < pSrcEnd)
		        {
                    u  = pSrc[0];
                    y1 = pSrc[1];
                    v  = pSrc[2];
                    y2 = pSrc[4];
                    pSrc+=6;

                    YUV2RGB(y1,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
                    YUV2RGB(y2,u,v,r,g,b);
                    *(pDest++) = (BYTE)b;
			        *(pDest++) = (BYTE)g;
		            *(pDest++) = (BYTE)r;
		        }
            }
            else
            {
                int y2,y1;

		        while (pSrc < pSrcEnd && pDest < m_BufferEnd)
		        {
                    y1 = pSrc[1];
                    y2 = pSrc[4];
                    pSrc+=6;

                    pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y1,255));
                    pDest+=3;

                    if(pDest < m_BufferEnd)
                    {
                        pDest[0] = pDest[1] = pDest[2] = (unsigned char)max(0,min(y2,255));
                        pDest+=3;
                    }
		        }
            }

			tc.sendStr("yuv444");
            break;
        }
        case ePvFmtMono12Packed:
        {
		    const Packed12BitsPixel_t*  pSrc = (const Packed12BitsPixel_t*)pFrame->ImageBuffer;
            const Packed12BitsPixel_t*  pSrcEnd = (const Packed12BitsPixel_t*)((unsigned char*)pFrame->ImageBuffer + pFrame->ImageSize);
		    BYTE*				        pDest = m_Buffer;		    
		    const BYTE			        bitshift = (BYTE)pFrame->BitDepth - 8;
            USHORT                      pixel;

            remainder = ULONG_PADDING(m_Width);

		    while (pSrc < pSrcEnd)
		    {
			    for (ULONG i = 0; i < m_Width && pSrc < pSrcEnd; i+=2)
                {
                    if(pDest < m_BufferEnd)
                    {
                        pixel = (USHORT)pSrc->LByte << 4;
                        pixel += ((USHORT)pSrc->MByte & 0xF0) >> 4;
                        *(pDest++) = pixel >> (bitshift - m_Gain);
                        
                        if(pDest < m_BufferEnd)
                        {
                            pixel = (USHORT)pSrc->UByte << 4;
                            pixel += ((USHORT)pSrc->MByte & 0x0F) >> 4;
                            *(pDest++) = pixel >> (bitshift - m_Gain);
                        }
                    }

                    pSrc++;
                }
             
			    pDest += remainder;
		    }

			tc.sendStr("mono12packed");
            break;    
        }
        case ePvFmtBayer12Packed:
        {
            if(!m_Interpolate)
            {
		        const Packed12BitsPixel_t*  pSrc = (const Packed12BitsPixel_t*)pFrame->ImageBuffer;
                const Packed12BitsPixel_t*  pSrcEnd = (const Packed12BitsPixel_t*)((unsigned char*)pFrame->ImageBuffer + pFrame->ImageSize);
		        BYTE*				        pDest = m_Buffer;		    
		        const BYTE			        bitshift = (BYTE)pFrame->BitDepth - 8;
                USHORT                      pixel;

                remainder = ULONG_PADDING(m_Width * 3);

		        while (pSrc < pSrcEnd && pDest < m_BufferEnd)
		        {
			        for (ULONG i = 0; i < m_Width && pSrc < pSrcEnd; i+=2)
                    {
                        if(pDest < m_BufferEnd)
                        {
                            pixel = (USHORT)pSrc->LByte << 4;
                            pixel += ((USHORT)pSrc->MByte & 0xF0) >> 4;
                            pDest[0] = pDest[1] = pDest[2] = pixel >> (bitshift - m_Gain);
                            pDest += 3;
                            
                            if(pDest < m_BufferEnd)
                            {
                                pixel = (USHORT)pSrc->UByte << 4;
                                pixel += ((USHORT)pSrc->MByte & 0x0F) >> 4;
                                pDest[0] = pDest[1] = pDest[2] = pixel >> (bitshift - m_Gain);
                                pDest += 3;
                            }
                        }

                        pSrc++;
                    }
                 
			        pDest += remainder;
		        }
            }
            else
            {
		        const Packed12BitsPixel_t*  pSrc = (const Packed12BitsPixel_t*)pFrame->ImageBuffer;
                const Packed12BitsPixel_t*  pSrcEnd = (const Packed12BitsPixel_t*)((unsigned char*)pFrame->ImageBuffer + pFrame->ImageSize);
		        BYTE*				        pDest = (BYTE*)pFrame->ImageBuffer;	  
                BYTE*                       pDestEnd = pDest + pFrame->ImageBufferSize;
		        const BYTE			        bitshift = (BYTE)pFrame->BitDepth - 8;
                USHORT                      pixel1,pixel2;

                remainder = ULONG_PADDING(m_Width * 3);

		        while (pSrc < pSrcEnd && pDest < pDestEnd)
		        {
			        for (ULONG i = 0; i < m_Width && pSrc < pSrcEnd; i+=2)
                    {
                        pixel1 = (USHORT)pSrc->LByte << 4;
                        pixel1 += ((USHORT)pSrc->MByte & 0xF0) >> 4;
                        
                        pixel2 = (USHORT)pSrc->UByte << 4;
                        pixel2 += ((USHORT)pSrc->MByte & 0x0F) >> 4;

                        if(pDest < pDestEnd)
                        {
                            *(pDest++) = pixel1 >> (bitshift - m_Gain);
                            if(pDest < pDestEnd)
                                *(pDest++) = pixel2 >> (bitshift - m_Gain);
                        }

                        pSrc++;
                    }
		        }

                if(pDest < pDestEnd)
                {
		            pFrame->Format = ePvFmtBayer8;

		            PvUtilityColorInterpolate(pFrame, &m_Buffer[2], &m_Buffer[1], &m_Buffer[0], 2, remainder);  
                }
            }

			tc.sendStr("bayer12packed");
            break;
        }
    }

    if(pFrame->AncillarySize && m_ShowAncillary)
    {
        const UCHAR* lData = (const UCHAR*)pFrame->AncillaryBuffer;
		CString lNumber;

        m_Ancillary.Empty();

        for(UINT i=0;i<pFrame->AncillarySize;i++)
		{
			lNumber.Format("%02X",lData[i]);
            m_Ancillary += lNumber;
		}
			//m_Ancillary.AppendFormat("%02X",lData[i]);
    }
               
    // and invalidate the view
	Invalidate(bClear);   
	tc.sendStr("got to the end");

    return true;
}
