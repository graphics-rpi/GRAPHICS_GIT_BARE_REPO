/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        ValueView.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the Attribute value view, at bottom of camera
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

//===== INCLUDE FILES =========================================================

#include "stdafx.h"
#include "SampleViewer.h"
#include "ValueView.h"

#include <math.h>
#include <limits.h>

//===== CONSTANTS =============================================================

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CValueView, CFormView)
// message map
BEGIN_MESSAGE_MAP(CValueView, CFormView)
    ON_BN_CLICKED(IDC_VALUEVIEW_VALIDATE,OnValidate)
    ON_BN_CLICKED(IDC_VALUEVIEW_RELOAD,OnReset)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CEnumValueView, CValueView)
// message map
BEGIN_MESSAGE_MAP(CEnumValueView, CValueView)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CUintValueView, CValueView)
// message map
BEGIN_MESSAGE_MAP(CUintValueView, CValueView)
    ON_WM_HSCROLL()
    ON_EN_CHANGE(IDC_UINTVIEW_VALUE,OnEditChange)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CSintValueView, CValueView)
// message map
BEGIN_MESSAGE_MAP(CSintValueView, CValueView)
    ON_WM_HSCROLL()
    ON_EN_CHANGE(IDC_SINTVIEW_VALUE,OnEditChange)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CFloatValueView, CValueView)
// message map
BEGIN_MESSAGE_MAP(CFloatValueView, CValueView)
    ON_EN_CHANGE(IDC_FLOATVIEW_VALUE,OnEditChange)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CCmdValueView, CValueView)
// message map
BEGIN_MESSAGE_MAP(CCmdValueView, CValueView)
    ON_BN_CLICKED(IDC_CMDVIEW_BUTTON,OnPushed)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CStringValueView, CValueView)
// message map
BEGIN_MESSAGE_MAP(CStringValueView, CValueView)
    ON_EN_CHANGE(IDC_STRINGVIEW_VALUE,OnEditChange)
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CValueView::CValueView()
    : CFormView(IDD_ENUMVIEW)
{
    m_Observer = NULL;
}

/*
 * Method:    CValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CValueView::CValueView(int nID)
    : CFormView(nID)
{
    m_Observer = NULL;
}

/*
 * Method:    ~CValueView()
 * Purpose:   class destructor
 * Comments:  none
 */
CValueView::~CValueView()
{
}

/*
 * Method:    Create()
 * Purpose:   create the view
 * Comments:  none
 */
BOOL CValueView::Create(PValueObserver* pObserver,const RECT& Rect, CWnd* pParent)
{
    if(m_BitValid.LoadBitmap(IDB_BITMAP5) && m_BitReload.LoadBitmap(IDB_BITMAP6))
    {
        if(CFormView::Create(NULL,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,Rect,pParent,0,NULL))
        {
            CButton* pButton;

            pButton = (CButton*)GetDlgItem(IDC_VALUEVIEW_VALIDATE);
            if(pButton)
                pButton->SetBitmap(HBITMAP(m_BitValid));

            pButton = (CButton*)GetDlgItem(IDC_VALUEVIEW_RELOAD);
            if(pButton)
                pButton->SetBitmap(HBITMAP(m_BitReload));

            m_Observer = pObserver;

            return true;
        }
        else
            return false;
    }
    else
        return false;
}

/*
 * Method:    SetLabel()
 * Purpose:   set the label to be displayed
 * Comments:  none
 */
void CValueView::SetLabel(CString& String)
{
    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_VALUEVIEW_LABEL);
    if(pStatic)
        pStatic->SetWindowText(const_cast<char *>((LPCTSTR)String));
}

/*
 * Method:    InvokeValid()
 * Purpose:   invoke the change(s) validation
 * Comments:  none
 */
void CValueView::InvokeValid()
{
    if(m_Observer)
        m_Observer->OnValueChanged(true);
}

/*
 * Method:    InvokeReset()
 * Purpose:   invoke the change(s) reset
 * Comments:  none
 */
void CValueView::InvokeReset()
{
    if(m_Observer)
        m_Observer->OnValueChanged(false);
}

/*
 * Method:    OnValidate()
 * Purpose:   called when the user press the "Validate" button
 * Comments:  none
 */
void CValueView::OnValidate()
{
    if(m_Observer)
        m_Observer->OnValueChanged(true);
}

/*
 * Method:    OnReset()
 * Purpose:   called when the user press the "Reload" button
 * Comments:  none
 */
void CValueView::OnReset()
{
    if(m_Observer)
        m_Observer->OnValueChanged(false);
}

/*
 * Method:    CEnumValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CEnumValueView::CEnumValueView()
    : CValueView(IDD_ENUMVIEW)
{
}

/*
 * Method:    SetRange()
 * Purpose:   set the range
 * Comments:  none
 */
BOOL CEnumValueView::SetRange(LPCTSTR pRange)
{
    CComboBox* pCBox = (CComboBox*)GetDlgItem(IDC_ENUMVIEW_COMBOBOX);

    int nIndex = 0;
    CString Enums = pRange;
    int i,j = 0;

    pCBox->ResetContent();

    for(i = 0;i<Enums.GetLength() && nIndex >= 0;i++)
    {
        if(Enums[i] == ',')
        {
            nIndex = pCBox->AddString(Enums.Mid(j,i-j));
            j = i+1;
        }
    }

    if(j<i && nIndex >= 0)
        nIndex = pCBox->AddString(Enums.Mid(j,i-j));

    return nIndex >= 0;
}

/*
 * Method:    SetValue()
 * Purpose:   set the current value
 * Comments:  none
 */
BOOL CEnumValueView::SetValue(LPCTSTR pValue)
{
    int nIndex = 0;
    CComboBox* pCBox = (CComboBox*)GetDlgItem(IDC_ENUMVIEW_COMBOBOX);

    if((nIndex = pCBox->FindStringExact(0,pValue)) != LB_ERR)
    {
        return pCBox->SetCurSel(nIndex) != CB_ERR;
    }
    else
        return false;
}

/*
 * Method:    GetValue()
 * Purpose:   get the value
 * Comments:  none
 */
BOOL CEnumValueView::GetValue(CString& Value)
{
    CComboBox* pCBox = (CComboBox*)GetDlgItem(IDC_ENUMVIEW_COMBOBOX);
    int nIndex;

    nIndex = pCBox->GetCurSel();
    if(nIndex != LB_ERR)
    {
        pCBox->GetLBText(nIndex,Value);
        return true;
    }
    else
        return false;
}

/*
 * Method:    CUintValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CUintValueView::CUintValueView()
    : CValueView(IDD_UINTVIEW)
{
    m_Skip = false;
}

/*
 * Method:    SetRange()
 * Purpose:   set the range
 * Comments:  none
 */
BOOL CUintValueView::SetRange(tPvUint32 nMin,tPvUint32 nMax)
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_UINTVIEW_SLIDER);

    if(pSlider)
    {
        int lMin,lMax;

        m_Min = nMin;
        m_Max = nMax;

        if(nMax - nMin> 0xFFFF)
        {
            lMin    = 0;
            lMax    = 0xFFFF;
            m_Step  = (tPvFloat32)(nMax - nMin) / (tPvFloat32)0x10000;
        }
        else
        {
            m_Step  = 1;
            lMin    = nMin;
            lMax    = nMax;
        }

        pSlider->SetRange(lMin,lMax);
        
        return true;
    }
    else
        return false;
}

/*
 * Method:    SetValue()
 * Purpose:   set the value
 * Comments:  none
 */
BOOL CUintValueView::SetValue(tPvUint32 nValue)
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_UINTVIEW_SLIDER);
    CEditView* pStatic = (CEditView*)GetDlgItem(IDC_UINTVIEW_VALUE);

    if(pStatic && pSlider)
    {
        CString String;

        String.Format("%u",nValue);
        m_Skip = true;
        pStatic->SetWindowText(String);
        m_Skip = false;

        pSlider->SetPos((int)ceil((tPvFloat32)nValue / m_Step));

        return true;
    }
    else
        return false; 
}

/*
 * Method:    GetValue()
 * Purpose:   get the value
 * Comments:  none
 */
BOOL CUintValueView::GetValue(tPvUint32& nValue)
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_UINTVIEW_SLIDER);
	CEditView* pStatic = (CEditView*)GetDlgItem(IDC_UINTVIEW_VALUE);

    if(pSlider && pStatic)
    {
        CString   String;
       
        pStatic->GetWindowText(String);

        nValue = strtoul((LPCTSTR)String,NULL,10);	

        if(!(nValue >= m_Min && nValue <= m_Max))
        {
            if(nValue < m_Min)
                nValue = m_Min;
            else
                nValue = m_Max;

            String.Format("%u",nValue);
            pStatic->SetWindowText(String);
            pSlider->SetPos((int)ceil(nValue / m_Step));
        }

        return true;
    }
    else
        return false;
}

/*
 * Method:    OnHScroll()
 * Purpose:   called when the slider is touched
 * Comments:  none
 */
void CUintValueView::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_UINTVIEW_SLIDER);
    CEditView* pStatic = (CEditView*)GetDlgItem(IDC_UINTVIEW_VALUE);

    if(pStatic && pSlider)
    {
	    CString String;
        int lPos,lMin,lMax;

        pSlider->GetRange(lMin,lMax);
        lPos = pSlider->GetPos();

        if(lPos == lMax)
            String.Format("%u",m_Max);
        else
        if(lPos == lMin)
            String.Format("%u",m_Min);
        else
	        String.Format("%u",(tPvUint32)ceil((tPvFloat32)lPos * m_Step));

	    m_Skip = true;
	    pStatic->SetWindowText(String);
	    m_Skip = false;
    }

    CValueView::OnHScroll(nSBCode,nPos,pScrollBar);
}

/*
 * Method:    OnEditChange()
 * Purpose:   called when the user change the value displayed in the entry field
 * Comments:  none
 */
void CUintValueView::OnEditChange()
{
    // after display
    if(!m_Skip)
    {
        CEditView* pStatic = (CEditView*)GetDlgItem(IDC_UINTVIEW_VALUE);
        CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_UINTVIEW_SLIDER);

        if(pStatic && pSlider)
        {
            CString   String;
            tPvUint32 nValue;

            pStatic->GetWindowText(String);

            nValue = strtoul((LPCTSTR)String,NULL,10);	

            pSlider->SetPos((int)ceil((tPvFloat32)nValue / m_Step));
        }
    }
}

/*
 * Method:    CSintValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CSintValueView::CSintValueView()
    : CValueView(IDD_SINTVIEW)
{
    m_Skip = false;
}

/*
 * Method:    SetRange()
 * Purpose:   set the range
 * Comments:  none
 */
BOOL CSintValueView::SetRange(tPvInt64& nMin,tPvInt64& nMax)
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SINTVIEW_SLIDER);

    if(pSlider)
    {
        int lMin,lMax;

        m_Min = nMin;
        m_Max = nMax;

        if(nMax - nMin> 0xFFFF)
        {
            lMin    = 0;
            lMax    = 0xFFFF;
            m_Step  = (double)(nMax - nMin) / (double)0x10000;
        }
        else
        {
            m_Step  = 1;
            lMin    = nMin;
            lMax    = nMax;
        }

        pSlider->SetRange(lMin,lMax);
        
        return true;
    }
    else
        return false;
}

/*
 * Method:    SetValue()
 * Purpose:   set the value
 * Comments:  none
 */
BOOL CSintValueView::SetValue(tPvInt64& nValue)
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SINTVIEW_SLIDER);
    CEditView* pStatic = (CEditView*)GetDlgItem(IDC_SINTVIEW_VALUE);

    if(pStatic && pSlider)
    {
        CString String;

        String.Format("%I64d",nValue);
        m_Skip = true;
        pStatic->SetWindowText(String);
        m_Skip = false;

        pSlider->SetPos((int)ceil((double)nValue / m_Step));

        return true;
    }
    else
        return false; 
}

/*
 * Method:    GetValue()
 * Purpose:   get the value
 * Comments:  none
 */
BOOL CSintValueView::GetValue(tPvInt64& nValue)
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SINTVIEW_SLIDER);
	CEditView* pStatic = (CEditView*)GetDlgItem(IDC_SINTVIEW_VALUE);

    if(pSlider && pStatic)
    {
        CString   String;
       
        pStatic->GetWindowText(String);

        nValue = atol((LPCTSTR)String);	

        if(!(nValue >= m_Min && nValue <= m_Max))
        {
            if(nValue < m_Min)
                nValue = m_Min;
            else
                nValue = m_Max;

            String.Format("%I64d",nValue);
            pStatic->SetWindowText(String);
            pSlider->SetPos((int)ceil((double)nValue / m_Step));
        }

        return true;
    }
    else
        return false;
}

/*
 * Method:    OnHScroll()
 * Purpose:   called when the slider is touched
 * Comments:  none
 */
void CSintValueView::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
    CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SINTVIEW_SLIDER);
    CEditView* pStatic = (CEditView*)GetDlgItem(IDC_SINTVIEW_VALUE);

    if(pStatic && pSlider)
    {
	    CString String;
        int lPos,lMin,lMax;

        pSlider->GetRange(lMin,lMax);
        lPos = pSlider->GetPos();

        if(lPos == lMax)
            String.Format("%I64d",m_Max);
        else
        if(lPos == lMin)
            String.Format("%I64d",m_Min);
        else
	        String.Format("%I64d",(tPvInt64)ceil((double)lPos * m_Step));

	    m_Skip = true;
	    pStatic->SetWindowText(String);
	    m_Skip = false;
    }

    CValueView::OnHScroll(nSBCode,nPos,pScrollBar);
}

/*
 * Method:    OnEditChange()
 * Purpose:   called when the user change the value displayed in the entry field
 * Comments:  none
 */
void CSintValueView::OnEditChange()
{
    // after display
    if(!m_Skip)
    {
        CEditView* pStatic = (CEditView*)GetDlgItem(IDC_SINTVIEW_VALUE);
        CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SINTVIEW_SLIDER);

        if(pStatic && pSlider)
        {
            CString   String;
            tPvInt64  nValue;

            pStatic->GetWindowText(String);

            nValue = atol((LPCTSTR)String);		

            pSlider->SetPos((int)ceil((double)nValue / m_Step));
        }
    }
}

/*
 * Method:    CFloatValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CFloatValueView::CFloatValueView()
    : CValueView(IDD_FLOATVIEW)
{
    m_Skip = false;
}

/*
 * Method:    SetRange()
 * Purpose:   set the range
 * Comments:  none
 */
BOOL CFloatValueView::SetRange(tPvFloat32 fMin,tPvFloat32 fMax)
{
    m_Min = fMin;
    m_Max = fMax;
    return true;
}

/*
 * Method:    SetValue()
 * Purpose:   set the value
 * Comments:  none
 */
BOOL CFloatValueView::SetValue(tPvFloat32 fValue)
{
    CEditView* pEdit = (CEditView*)GetDlgItem(IDC_FLOATVIEW_VALUE);

    if(pEdit)
    {
        CString String;

        String.Format("%.3f",fValue);
        m_Skip = true;
        pEdit->SetWindowText(String);
        m_Skip = false;

        m_Value = fValue;

        return true;
    }
    else
        return false; 
}

/*
 * Method:    GetValue()
 * Purpose:   get the value
 * Comments:  none
 */
BOOL CFloatValueView::GetValue(tPvFloat32& fValue)
{
    CEditView* pEdit = (CEditView*)GetDlgItem(IDC_FLOATVIEW_VALUE);

    if(pEdit)
    {
        CString String;
        CString Cleaned;

        pEdit->GetWindowText(String);

        Cleaned = String.SpanIncluding("0123456789.-");

        fValue = (tPvFloat32)atof((LPCTSTR)Cleaned);

        if(fValue<m_Min || fValue>m_Max)
        {
            if(fValue<m_Min)
                fValue = m_Min;
            else
                fValue = m_Max;  
    
            Cleaned.Format("%.3f",fValue);
            m_Skip = true;
            pEdit->SetWindowText(Cleaned);
            m_Skip = false;
        }
        else
        if(String != Cleaned)
        {
            m_Skip = true;
            pEdit->SetWindowText(Cleaned);
            m_Skip = false;
        }

        return true;
    }
    else
        return false;
}

/*
 * Method:    OnEditChange()
 * Purpose:   called when the user change the value displayed in the entry field
 * Comments:  none
 */
void CFloatValueView::OnEditChange()
{
}

/*
 * Method:    CCmdValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CCmdValueView::CCmdValueView()
    : CValueView(IDD_CMDVIEW)
{
}

/*
 * Method:    SetLabel()
 * Purpose:   set the label to be displayed
 * Comments:  none
 */
void CCmdValueView::SetLabel(CString& String)
{
    CButton* pButton = (CButton*)GetDlgItem(IDC_CMDVIEW_BUTTON);
    if(pButton)
        pButton->SetWindowText(const_cast<char *>((LPCTSTR)String));
}

/*
 * Method:    OnPushed()
 * Purpose:   called when the user push the button
 * Comments:  none
 */
void CCmdValueView::OnPushed()
{
    InvokeValid();
}

/*
 * Method:    CStringValueView()
 * Purpose:   class constructor
 * Comments:  none
 */
CStringValueView::CStringValueView()
    : CValueView(IDD_STRINGVIEW)
{
    m_Skip = false;
}

/*
 * Method:    SetValue()
 * Purpose:   set the value
 * Comments:  none
 */
BOOL CStringValueView::SetValue(const CString& sValue)
{
    CEditView* pEdit = (CEditView*)GetDlgItem(IDC_STRINGVIEW_VALUE);

    if(pEdit)
    {
        m_Skip = true;
        pEdit->SetWindowText(sValue);
        m_Skip = false;

        return true;
    }
    else
        return false; 
}

/*
 * Method:    GetValue()
 * Purpose:   get the value
 * Comments:  none
 */
BOOL CStringValueView::GetValue(CString& sValue)
{
    CEditView* pEdit = (CEditView*)GetDlgItem(IDC_STRINGVIEW_VALUE);

    if(pEdit)
    {
        pEdit->GetWindowText(sValue); 

        return true;
    }
    else
        return false;
}

/*
 * Method:    OnEditChange()
 * Purpose:   called when the user change the value displayed in the entry field
 * Comments:  none
 */
void CStringValueView::OnEditChange()
{
}