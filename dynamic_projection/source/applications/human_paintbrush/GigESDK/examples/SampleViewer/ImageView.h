/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ImageView.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define a view class to render live view
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

#ifndef _IMAGEVIEW_INCLUDED_
#define _IMAGEVIEW_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <PvAPI.h>

//===== CLASS DEFINITION ======================================================

/*
 * Class:    CImageView
 * Purpose:  render the image
 * Comments: none
 */
class CImageView : public CScrollView
{
	DECLARE_DYNCREATE(CImageView)

    public:

        /*
         * Method:    CImageView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
	    CImageView();

        /*
         * Method:    ~CImageView()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */        
        virtual ~CImageView();

        /*
         * Method:    Create()
         * Purpose:   create the window
         * Arguments: 
         *
         * [i] CWnd* pParentWnd,        parent
         * [i] tPvInterface Interface,  interface used by the camera
         *
         * Return:    FALSE if failed
         * Comments:  none
         */ 
        BOOL Create(CWnd* pParentWnd,tPvInterface Interface);

        /*
         * Method:    Update()
         * Purpose:   update the rendering with a given frame
         * Arguments: 
         *
         * [i] tPvFrame* pFrame, frame
         *
         * Return:    true if rendered, false otherwise
         * Comments:  none
         */ 
        BOOL Update(tPvFrame* pFrame);

        /*
         * Method:    SetToScale()
         * Purpose:   instruct the view to scale the image to fit
         *            in the window
         * Arguments: 
         *
         * [i] BOOL aScale, true if to scale, false otherwise
         *
         * Return:    none
         * Comments:  none
         */ 
        void SetToScale(BOOL aScale);

        /*
         * Method:    IsScaling()
         * Purpose:   check if the image is scaled or not
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        BOOL IsScaling() const {return m_Scale;};

        /*
         * Method:    SetShowAncillary()
         * Purpose:   instruct the view to render the ancillary data
         * Arguments: 
         *
         * [i] BOOL aShowAncillary, true or false
         *
         * Return:    none
         * Comments:  none
         */ 
        void SetShowAncillary(BOOL aShowAncillary) {m_ShowAncillary = aShowAncillary;};

        /*
         * Method:    IsShowAncillary()
         * Purpose:   check if the image is showing the ancillary data
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        BOOL IsShowAncillary() const {return m_ShowAncillary;};

        /*
         * Method:    SetInterpolate()
         * Purpose:   instruct the view to interpolate colors or not
         * Arguments: 
         *
         * [i] BOOL aMono, true if to interpolate, false otherwise
         *
         * Return:    none
         * Comments:  none
         */ 
        void SetInterpolate(BOOL aInterpolate) {m_Interpolate = aInterpolate;};

        /*
         * Method:    IsInterpolate()
         * Purpose:   check if the image is interpolated
         * Arguments: none
         * Return:    none
         * Comments:  none
         */ 
        BOOL IsInterpolate() const {return m_Interpolate;};

        /*
         * Method:    Reset()
         * Purpose:   do whatever need to be done when the streaming is re-started
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void Reset();

        /*
         * Method:    SaveAsBitmap()
         * Purpose:   Save the bitmap in a bmp file
         * Arguments:
         *
         * [i] const char* pFilename, file to save the image as
         *
         * Return:    none
         * Comments:  none
         */
        BOOL SaveAsBitmap(const char* pFilename);

        void SetGain(ULONG Value);

        ULONG GetGain();

    public: // from CScrollView

        /*
         * Method:    PreCreateWindow()
         * Purpose:   called by the framework prior to the creation of
         *            the window. 
         * Arguments:
         *
         * [b] CREATESTRUCT& cs, creation parameters
         *
         * Return:    FALSE if an error occured
         * Comments:  Can be used to alter to window size and position
         */
	    BOOL PreCreateWindow(CREATESTRUCT& cs);

        /*
         * Method:    OnInitialUpdate()
         * Purpose:   called by the framework on the first update of the view
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void OnInitialUpdate(); 

        void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL );

        /*
         * Method:    OnSize()
         * Purpose:   called by the framework when the window was resized
         * Arguments: 
         *
         * [i] UINT nType,  type of the resizing
         * [i] int cx,      new width of the client area
         * [i] int cy,      new height of the client area
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnSize(UINT nType, int cx, int cy);

        /*
         * Method:    OnDraw()
         * Purpose:   called by the framework when the client area of
         *            the window must be redraw
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnDraw( CDC* pDC );

        /*
         * Method:    OnEraseBkgnd()
         * Purpose:   called by the framework when the client area of
         *            the window should be erased
         * Arguments: 
         *
         * [i] CDC* pDC, device-context object
         *
         * Return:    TRUE if the background was erased
         * Comments:  none
         */
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

        #ifdef _ZOOM
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
        #endif

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:

        void AdaptToSize();

    private: // type

        // bitmap info
        struct tBitmapInfo
        {
	        BITMAPINFOHEADER	bmiHeader;
	        RGBQUAD				bmiColors[256];
        };

    private:

        // used to run OnInitialUpdate() only once
        BOOL m_InitDone;
        // bitmap info
	    tBitmapInfo		m_BitmapInfo;
        // internal buffer for frame rendering
		BYTE*			m_Buffer;
        BYTE*           m_BufferEnd;
        ULONG           m_BufferSize;
        // width & height of the frames
		ULONG			m_Width;
		ULONG			m_Height;
        // width & height of view
		ULONG			m_VWidth;
		ULONG			m_VHeight;
        // monochrome of color
        BOOL            m_IsBgr;
        // image must be scaled to fit
        BOOL            m_Scale;
        // force mono rendering
        BOOL            m_ShowAncillary;
        // perform color interpolation
        BOOL            m_Interpolate;
        // coord. of the left-top corner of the image in the view
        int             m_Left;
        int             m_Top;
        //
        ULONG           m_Gain;
        // formatted ancillary data
        CString         m_Ancillary;
        CFont           m_Font;

#ifdef _ZOOM

        // Camera's interface (GigE or DCAM)
        tPvInterface     m_Interface;

    public:

        tPvHandle        m_Camera;
        DWORD            m_Mode;
        bool             m_Down;
#endif

};

#endif // _IMAGEVIEW_INCLUDED_
