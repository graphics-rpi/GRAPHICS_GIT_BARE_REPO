/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ContainerView.h		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Define the Attribute value view, at bottom of camera
|               controls window.	
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

#ifndef _VALUEVIEW_INCLUDED_
#define _VALUEVIEW_INCLUDED_

#include <PvAPI.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//===== CLASS DEFINITION ======================================================

/*
 * Class:    PValueObserver
 * Purpose:  Prototype for a value change observer
 * Comments: none
 */
class PValueObserver
{
    public:

        /*
         * Method:    ~PValueObserver()
         * Purpose:   destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        virtual ~PValueObserver() {};

    protected:

        /*
         * Method:    OnValueChanged()
         * Purpose:   called when the value of the attribute have changed
         * Arguments:
         *
         * [i] BOOL bValid, true when the change must be validated
         *
         * Return:    none
         * Comments:  none
         */
        virtual void OnValueChanged(BOOL bValid) = 0;

    friend class CValueView;
};

/*
 * Class:    CValueView
 * Purpose:  Derive the CFormView to create a view to hold the attribute value
 *           control
 * Comments: none
 */
class CValueView : public CFormView
{
    DECLARE_DYNCREATE(CValueView)

    public:

        /*
         * Method:    CValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CValueView();

        /*
         * Method:    CValueView()
         * Purpose:   class constructor
         * Arguments: 
         *
         * [i] int nRes, resource ID
         *
         * Return:    none
         * Comments:  none
         */
        CValueView(int nRes);

        /*
         * Method:    ~CValueView()
         * Purpose:   class destructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        virtual ~CValueView();

        /*
         * Method:    Create()
         * Purpose:   create the view
         * Arguments: 
         *
         * [i] PValueObserver* pObserver,   observer
         * [i] const RECT& Rect,            rect of the view
         * [i] CWnd* pParent,               view's parent
         *
         * Return:    true is no error
         * Comments:  none
         */
	    BOOL Create(PValueObserver* pObserver,const RECT& Rect, CWnd* pParent);

        /*
         * Method:    SetLabel()
         * Purpose:   set the label to be displayed
         * Arguments: 
         *
         * [i] CString& String, string to display
         *
         * Return:    none
         * Comments:  none
         */
        virtual void SetLabel(CString& String);

    protected:

        /*
         * Method:    OnValidate()
         * Purpose:   called when the user press the "Validate" button
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnValidate();

        /*
         * Method:    OnReset()
         * Purpose:   called when the user press the "Reload" button
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnReset( );

    protected:

        /*
         * Method:    InvokeValid()
         * Purpose:   invoke the change(s) validation
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void InvokeValid();

        /*
         * Method:    InvokeReset()
         * Purpose:   invoke the change(s) reset
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        void InvokeReset();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:

        // bitmaps for the buttons
        CBitmap         m_BitValid;
        CBitmap         m_BitReload;

        // change observer
        PValueObserver* m_Observer;
};

/*
 * Class:    CEnumValueView
 * Purpose:  Derive the CValueView to create a view to hold the attribute value when
 *           it's an enumerate
 * Comments: none
 */
class CEnumValueView : public CValueView
{
    DECLARE_DYNCREATE(CEnumValueView)

    public:

        /*
         * Method:    CEnumValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CEnumValueView();

        /*
         * Method:    SetRange()
         * Purpose:   set the range
         * Arguments: 
         *
         * [i] LPCTSTR pRange, range string
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetRange(LPCTSTR pRange);

        /*
         * Method:    SetValue()
         * Purpose:   set the current value
         * Arguments: 
         *
         * [i] LPCTSTR pValue, current value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetValue(LPCTSTR pValue);

        /*
         * Method:    GetValue()
         * Purpose:   get the value
         * Arguments: 
         *
         * [i] CString& Value, value string
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL GetValue(CString& Value);

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()
};

/*
 * Class:    CUintValueView
 * Purpose:  Derive the CValueView to create a view to hold the attribute value when
 *           it's an unint32
 * Comments: none
 */
class CUintValueView : public CValueView
{
    DECLARE_DYNCREATE(CUintValueView)

    public:

        /*
         * Method:    CUintValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CUintValueView();

        /*
         * Method:    SetRange()
         * Purpose:   set the range
         * Arguments: 
         *
         * [i] tPvUint32 nMin, minimum value
         * [i] tPvUint32 nMax, maximum value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetRange(tPvUint32 nMin,tPvUint32 nMax);

        /*
         * Method:    SetValue()
         * Purpose:   set the value
         * Arguments: 
         *
         * [i] tPvUint32 nValue, current value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetValue(tPvUint32 nValue);

        /*
         * Method:    GetValue()
         * Purpose:   get the value
         * Arguments: 
         *
         * [i] tPvUint32& nValue, value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL GetValue(tPvUint32& nValue);

    protected:

        /*
         * Method:    OnHScroll()
         * Purpose:   called when the slider is touched
         * Arguments: 
         *
         * [i] UINT nSBCode,            indicates the scrolling request of the user
         * [i] UINT nPos,               Specifies the scroll-box position    
         * [i] CScrollBar* pScrollBar,  (not used)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );

        /*
         * Method:    OnEditChange()
         * Purpose:   called when the user change the value displayed in the entry field
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnEditChange();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:
        BOOL        m_Skip;
        tPvUint32   m_Min;
        tPvUint32   m_Max;
        tPvFloat32  m_Step;
};

/*
 * Class:    CSintValueView
 * Purpose:  Derive the CValueView to create a view to hold the attribute value when
 *           it's an sint64
 * Comments: none
 */
class CSintValueView : public CValueView
{
    DECLARE_DYNCREATE(CSintValueView)

    public:

        /*
         * Method:    CSintValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CSintValueView();

        /*
         * Method:    SetRange()
         * Purpose:   set the range
         * Arguments: 
         *
         * [i] tPvInt64& nMin, minimum value
         * [i] tPvInt64& nMax, maximum value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetRange(tPvInt64& nMin,tPvInt64& nMax);

        /*
         * Method:    SetValue()
         * Purpose:   set the value
         * Arguments: 
         *
         * [i] tPvInt64& nValue, current value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetValue(tPvInt64& nValue);

        /*
         * Method:    GetValue()
         * Purpose:   get the value
         * Arguments: 
         *
         * [i] tPvInt64& nValue, value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL GetValue(tPvInt64& nValue);

    protected:

        /*
         * Method:    OnHScroll()
         * Purpose:   called when the slider is touched
         * Arguments: 
         *
         * [i] UINT nSBCode,            indicates the scrolling request of the user
         * [i] UINT nPos,               Specifies the scroll-box position    
         * [i] CScrollBar* pScrollBar,  (not used)
         *
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );

        /*
         * Method:    OnEditChange()
         * Purpose:   called when the user change the value displayed in the entry field
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnEditChange();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:
        BOOL        m_Skip;
        tPvInt64    m_Min;
        tPvInt64    m_Max;
        double      m_Step;
};

/*
 * Class:    CFloatValueView
 * Purpose:  Derive the CValueView to create a view to hold the attribute value when
 *           it's a float32
 * Comments: none
 */
class CFloatValueView : public CValueView
{
    DECLARE_DYNCREATE(CFloatValueView)

    public:

        /*
         * Method:    CFloatValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CFloatValueView();

        /*
         * Method:    SetRange()
         * Purpose:   set the range
         * Arguments: 
         *
         * [i] tPvFloat32 fMin, minimum value
         * [i] tPvFloat32 fMax, maximum value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetRange(tPvFloat32 fMin,tPvFloat32 fMax);

        /*
         * Method:    SetValue()
         * Purpose:   set the value
         * Arguments: 
         *
         * [i] tPvFloat32 fValue, current value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetValue(tPvFloat32 fValue);

        /*
         * Method:    GetValue()
         * Purpose:   get the value
         * Arguments: 
         *
         * [o] tPvFloat32& fValue, value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL GetValue(tPvFloat32& fValue);

    protected:

        /*
         * Method:    OnEditChange()
         * Purpose:   called when the user change the value displayed in the entry field
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnEditChange();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:
        
        // min & max & value
        tPvFloat32 m_Min;
        tPvFloat32 m_Max;
        tPvFloat32 m_Value;

        BOOL m_Skip;
};

/*
 * Class:    CCmdValueView
 * Purpose:  Derive the CValueView to create a view to hold the attribute value when
 *           it's a command
 * Comments: none
 */
class CCmdValueView : public CValueView
{
    DECLARE_DYNCREATE(CCmdValueView)

    public:

        /*
         * Method:    CCmdValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CCmdValueView();

        /*
         * Method:    SetLabel()
         * Purpose:   set the label to be displayed
         * Arguments: 
         *
         * [i] CString& String, string to display
         *
         * Return:    none
         * Comments:  none
         */
        void SetLabel(CString& String);

    protected:

        /*
         * Method:    OnPushed()
         * Purpose:   called when the user push the button
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnPushed( );      

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()
};

/*
 * Class:    CStringValueView
 * Purpose:  Derive the CValueView to create a view to hold the attribute value when
 *           it's a string
 * Comments: none
 */
class CStringValueView : public CValueView
{
    DECLARE_DYNCREATE(CStringValueView)

    public:

        /*
         * Method:    CStringValueView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CStringValueView();

        /*
         * Method:    SetValue()
         * Purpose:   set the value
         * Arguments: 
         *
         * [i] const CString& sValue, current value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL SetValue(const CString& sValue);

        /*
         * Method:    GetValue()
         * Purpose:   get the value
         * Arguments: 
         *
         * [o] CString& sValue, value
         *
         * Return:    true if no error
         * Comments:  none
         */
        BOOL GetValue(CString& sValue);

    protected:

        /*
         * Method:    OnEditChange()
         * Purpose:   called when the user change the value displayed in the entry field
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        afx_msg void OnEditChange();

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

    private:
        
        // value
        char m_Value[28];

        BOOL m_Skip;
};


#endif // _VALUEVIEW_INCLUDED_
