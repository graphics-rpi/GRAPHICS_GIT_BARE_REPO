/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        SerialFrame.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the RS232 SerialIO window	
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
#include "SerialFrame.h"
#include <PvRegIO.h>

//===== DEFINES ===============================================================

#define WIDTH  610
#define HEIGHT 420

// Camera registers dealing with Serial IO
#define REG_SIO_INQUIRY			0x16000
#define REG_SIO_MODE_INQUIRY	0x16100
#define REG_SIO_MODE			0x16104
#define REG_SIO_TX_INQUIRY		0x16120
#define REG_SIO_TX_STATUS		0x16124
#define REG_SIO_TX_CONTROL		0x16128
#define REG_SIO_TX_LENGTH		0x1612C
#define REG_SIO_RX_INQUIRY		0x16140
#define REG_SIO_RX_STATUS		0x16144
#define REG_SIO_RX_CONTROL		0x16148
#define REG_SIO_RX_LENGTH		0x1614C
#define REG_SIO_TX_BUFFER		0x16400
#define REG_SIO_RX_BUFFER		0x16800

//===== INLINES ===============================================================

inline void SetBits(unsigned int& aValue,unsigned int aData,unsigned int aPosition,unsigned int aLength)
{
    unsigned int datamask;   // data mask, before aPosition shift

    if (aLength == 32)
        datamask = 0xFFFFFFFF;
    else
        datamask = (1L << aLength) - 1;

    aValue &= ~(datamask << aPosition);             // Clear bits
    aValue |= (aData & datamask) << aPosition;      // Set value
}

inline void SetBits(unsigned long& aValue,unsigned int aData,unsigned int aPosition,unsigned int aLength)
{
    unsigned int datamask;   // data mask, before aPosition shift

    if (aLength == 32)
        datamask = 0xFFFFFFFF;
    else
        datamask = (1L << aLength) - 1;

    aValue &= ~(datamask << aPosition);             // Clear bits
    aValue |= (aData & datamask) << aPosition;      // Set value
}

inline unsigned int GetBits(unsigned int aValue,unsigned int aPosition,unsigned int aLength)
{
    unsigned int datamask;   // data mask, before aPosition shift

    if (aLength == 32)
        datamask = 0xFFFFFFFF;
    else
        datamask = (1L << aLength) - 1;

    return (aValue >> aPosition) & datamask;
}

void _OutputDebugString(const char* format, ...)
{
	static char output[1024];
	va_list     arg;

	va_start(arg, format);

	_vsnprintf(output, sizeof(output), format, arg);

	va_end(arg);     

	OutputDebugString(output);
}

bool F_WriteMem(tPvHandle camera,unsigned long address,const unsigned char*	buffer,unsigned long length)
{
	const unsigned long	numRegs = (length + 3) / 4;
	unsigned long*		pAddressArray = new unsigned long[numRegs];
	unsigned long*		pDataArray = new unsigned long[numRegs];
	bool				result;
	unsigned long		i;


	//
	// We want to write an array of bytes from the camera.  To do this, we
	// write sequential registers with the data array.  The register MSB
	// is the first byte of the array.
	//

	// 1.  Generate write addresses, and convert from byte array to MSB-packed
	// registers.
	for (i = 0; i < numRegs; i++)
	{
		pAddressArray[i] = address + (i*4);

		pDataArray[i] = (unsigned long)*(buffer++) << 24;
		pDataArray[i] |= (unsigned long)*(buffer++) << 16;
		pDataArray[i] |= (unsigned long)*(buffer++) << 8;
		pDataArray[i] |= (unsigned long)*(buffer++);
	}

	// 2.  Execute write.
	if (PvRegisterWrite(camera, numRegs, pAddressArray, pDataArray, NULL) == ePvErrSuccess)
		result = true;
	else
		result = false;

	delete [] pAddressArray;
	delete [] pDataArray;

	return result;
}

bool F_ReadMem(tPvHandle camera,unsigned long address,unsigned char* buffer,unsigned long length)
{
	const unsigned long	numRegs = (length + 3) / 4;
	unsigned long*		pAddressArray = new unsigned long[numRegs];
	unsigned long*		pDataArray = new unsigned long[numRegs];
	bool				result;
	unsigned long		i;


	//
	// We want to read an array of bytes from the camera.  To do this, we
	// read sequential registers which contain the data array.  The register
	// MSB is the first byte of the array.
	//

	// 1.  Generate read addresses
	for (i = 0; i < numRegs; i++)
		pAddressArray[i] = address + (i*4);

	// 2.  Execute read.
	if (PvRegisterRead(camera, numRegs, pAddressArray, pDataArray, NULL) == ePvErrSuccess)
	{
        unsigned long data = 0;

		// 3.  Convert from MSB-packed registers to byte array
		for (i = 0; i < length; i++)
		{
			if (i % 4 == 0)
				data = pDataArray[i/4];

			buffer[i] = (unsigned char)((data >> 24) & 0xFF);
			data <<= 8;
		}

		result = true;
	}
	else
		result = false;

	delete [] pAddressArray;
	delete [] pDataArray;

	return result;
}

bool WriteStringToSerialIO(tPvHandle camera,const char* string,unsigned int length)
{
	unsigned long		regAddress;
	unsigned long		regValue;

	// Wait for transmitter ready.
	do
	{
		regAddress = REG_SIO_TX_STATUS;
		if (PvRegisterRead(camera, 1, &regAddress, &regValue, NULL) != ePvErrSuccess)
			return false;
	}
	while (!(regValue & 1));  // Waiting for transmitter-ready bit

	// Write the buffer.
	if (!F_WriteMem(camera, REG_SIO_TX_BUFFER, (const unsigned char*)string, length))
		return false;

	// Write the buffer length.  This triggers transmission.
	regAddress = REG_SIO_TX_LENGTH;
	regValue = length;
	if (PvRegisterWrite(camera, 1, &regAddress, &regValue, NULL) != ePvErrSuccess)
		return false;

	return true;
}

bool ReadStringFromSerialIO(tPvHandle camera,unsigned char*	buffer,unsigned long bufferLength,unsigned long* pReceiveLength)
{
	unsigned long		regAddress;
	unsigned long		dataLength;

	// How many characters to read?
	regAddress = REG_SIO_RX_LENGTH;
	if (PvRegisterRead(camera, 1, &regAddress, &dataLength, NULL) != ePvErrSuccess)
		return false;

	// It must fit in the user's buffer.
	if (dataLength > bufferLength)
		dataLength = bufferLength;

	if (dataLength > 0)
	{
		// Read the data.
		if (!F_ReadMem(camera, REG_SIO_RX_BUFFER, buffer, dataLength))
			return false;

		// Decrement the camera's read index.
		regAddress = REG_SIO_RX_LENGTH;
		if (PvRegisterWrite(camera, 1, &regAddress, &dataLength, NULL) != ePvErrSuccess)
			return false;
	}

	*pReceiveLength = dataLength;

	return true;
}

//===== CONSTANTS =============================================================

static const char* KBaudRates[] = {

    "300",
    "600",
    "1200",
    "2400",
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "115200",
    "230400"

};

static const char* KParities[] = {
    
    "None",
    "Odd",
    "Even"

};

static const char* KCharLens[] = {

    "5 bits",
    "6 bits",
    "7 bits",
    "8 bits"

};

static const char* KStopBits[] = {

    "1 bit",
    "1.5 bits",
    "2 bits"

};

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CSerialFrame, CChildFrame)

// message map
BEGIN_MESSAGE_MAP(CSerialFrame, CChildFrame)
    ON_WM_TIMER()
END_MESSAGE_MAP()


//===== CLASS DEFINITION ======================================================

/*
 * Class:    CSerialView
 * Purpose:  Derive the FormView to create a view to hold the various fields
 *           to be displayed by the window
 * Comments: none
 */
class CSerialView : public CFormView
{
    DECLARE_DYNCREATE(CSerialView)

    public:

        /*
         * Method:    CSerialView()
         * Purpose:   class constructor
         * Arguments: none
         * Return:    none
         * Comments:  none
         */
        CSerialView();

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

        void setupInitial(unsigned long aInquiry,unsigned long aModes);
        void setupConnected();
        void setupDisconnected();

        afx_msg void OnRateChange();
        afx_msg void OnParityChange();
        afx_msg void OnCharLenChange();
        afx_msg void OnStopBitChange();

        afx_msg void OnSend();
        afx_msg void OnConnect();
        afx_msg void OnDisconnect();

        unsigned int getLine(char* Buffer,unsigned int Max);

        void clearLine();

        void addTxString(const char* String);
        void addRxString(const char* String);

    protected:

        // message map
	    DECLARE_MESSAGE_MAP()

};

//===== CLASS IMPLEMENTATIONS =================================================

IMPLEMENT_DYNCREATE(CSerialView, CFormView)

// message map
BEGIN_MESSAGE_MAP(CSerialView, CFormView)
    ON_CBN_SELCHANGE(IDC_COMBO3,OnRateChange)
    ON_CBN_SELCHANGE(IDC_COMBO4,OnParityChange)
    ON_CBN_SELCHANGE(IDC_COMBO5,OnCharLenChange)
    ON_CBN_SELCHANGE(IDC_COMBO6,OnStopBitChange)
    ON_BN_CLICKED(IDC_BUTTON1,OnSend)
    ON_BN_CLICKED(IDC_BUTTON3,OnConnect)
    ON_BN_CLICKED(IDC_BUTTON2,OnDisconnect)
END_MESSAGE_MAP()

/*
 * Method:    CSerialView()
 * Purpose:   class constructor
 * Comments:  none
 */
CSerialView::CSerialView()
    : CFormView(IDD_SERIALVIEW)
{
}

/*
 * Method:    Create()
 * Purpose:   create the view
 * Comments:  none
 */
BOOL CSerialView::Create(const RECT& Rect, CWnd* pParent)
{
    if(CFormView::Create(NULL,_T("SerialView"),WS_CHILD | WS_VISIBLE,Rect,pParent,0,NULL))
        return true;
    else
        return false;
}

void CSerialView::setupInitial(unsigned long aInquiry,unsigned long aModes)
{
    CComboBox* lRate    = (CComboBox*)GetDlgItem(IDC_COMBO3);
    CComboBox* lParity  = (CComboBox*)GetDlgItem(IDC_COMBO4);
    CComboBox* lCharLen = (CComboBox*)GetDlgItem(IDC_COMBO5);
    CComboBox* lStopBit = (CComboBox*)GetDlgItem(IDC_COMBO6);

    // enable the controls

    ((CButton*)GetDlgItem(IDC_BUTTON3))->EnableWindow();
    lRate->EnableWindow();
    lParity->EnableWindow();
    lCharLen->EnableWindow();
    lStopBit->EnableWindow();

    // set all the possible values

    for(int i=0;i<10;i++)
    {
        if(aInquiry & (0x1 * (i + 1)))
            lRate->SetItemData(lRate->AddString(KBaudRates[i]),i);
    }

    for(int i=0;i<3;i++)
    {
        if(aInquiry & (0x10000 * (i + 1)))
            lParity->SetItemData(lParity->AddString(KParities[i]),i);        
    }

    for(int i=0;i<4;i++)
    {
        if(aInquiry & (0x100000 * (i + 1)))
            lCharLen->SetItemData(lCharLen->AddString(KCharLens[i]),i);        
    }    

    for(int i=0;i<3;i++)
    {
        if(aInquiry & (0x1000000 * (i + 1)))
            lStopBit->SetItemData(lStopBit->AddString(KStopBits[i]),i);        
    }

    // set what's currently on the camera

    lRate->SelectString(-1,KBaudRates[GetBits(aModes,0,8)]);
    lParity->SelectString(-1,KParities[GetBits(aModes,8,2)]);
    lCharLen->SelectString(-1,KCharLens[GetBits(aModes,10,2)]);
    lStopBit->SelectString(-1,KStopBits[GetBits(aModes,12,2)]);
}

void CSerialView::setupConnected()
{
    CComboBox* lRate        = (CComboBox*)GetDlgItem(IDC_COMBO3);
    CComboBox* lParity      = (CComboBox*)GetDlgItem(IDC_COMBO4);
    CComboBox* lCharLen     = (CComboBox*)GetDlgItem(IDC_COMBO5);
    CComboBox* lStopBit     = (CComboBox*)GetDlgItem(IDC_COMBO6);
    CButton*   lConnect     = (CButton*)GetDlgItem(IDC_BUTTON3);
    CButton*   lDisconnect  = (CButton*)GetDlgItem(IDC_BUTTON2);
    CButton*   lSend        = (CButton*)GetDlgItem(IDC_BUTTON1);
    CEdit*     lEntry       = (CEdit*)GetDlgItem(IDC_EDIT1);

    lRate->EnableWindow(false);
    lParity->EnableWindow(false);
    lCharLen->EnableWindow(false);
    lStopBit->EnableWindow(false);
    lConnect->EnableWindow(false);
    lDisconnect->EnableWindow();
    lSend->EnableWindow();
    lEntry->EnableWindow();
}

void CSerialView::setupDisconnected()
{
    CComboBox* lRate        = (CComboBox*)GetDlgItem(IDC_COMBO3);
    CComboBox* lParity      = (CComboBox*)GetDlgItem(IDC_COMBO4);
    CComboBox* lCharLen     = (CComboBox*)GetDlgItem(IDC_COMBO5);
    CComboBox* lStopBit     = (CComboBox*)GetDlgItem(IDC_COMBO6);
    CButton*   lConnect     = (CButton*)GetDlgItem(IDC_BUTTON3);
    CButton*   lDisconnect  = (CButton*)GetDlgItem(IDC_BUTTON2);
    CButton*   lSend        = (CButton*)GetDlgItem(IDC_BUTTON1);
    CEdit*     lEntry       = (CEdit*)GetDlgItem(IDC_EDIT1);

    lRate->EnableWindow();
    lParity->EnableWindow();
    lCharLen->EnableWindow();
    lStopBit->EnableWindow();
    lConnect->EnableWindow();
    lDisconnect->EnableWindow(false);
    lSend->EnableWindow(false);
    lEntry->EnableWindow(false);
}

unsigned int CSerialView::getLine(char* Buffer,unsigned int Max)
{
    CEdit* lEntry = (CEdit*)GetDlgItem(IDC_EDIT1);

    return lEntry->GetLine(0,Buffer,Max);
}

void CSerialView::clearLine() 
{
    CEdit* lEntry = (CEdit*)GetDlgItem(IDC_EDIT1);

    lEntry->SetWindowText("");
}

void CSerialView::OnRateChange()
{
    CComboBox* lBox = (CComboBox*)GetDlgItem(IDC_COMBO3);

    ((CSerialFrame*)this->GetParent())->OnRateChange(lBox->GetItemData(lBox->GetCurSel()));
}

void CSerialView::OnParityChange()
{
    CComboBox* lBox = (CComboBox*)GetDlgItem(IDC_COMBO4);

    ((CSerialFrame*)this->GetParent())->OnParityChange(lBox->GetItemData(lBox->GetCurSel()));
}

void CSerialView::OnCharLenChange()
{
    CComboBox* lBox = (CComboBox*)GetDlgItem(IDC_COMBO5);

    ((CSerialFrame*)this->GetParent())->OnCharLenChange(lBox->GetItemData(lBox->GetCurSel()));
}

void CSerialView::OnStopBitChange()
{
    CComboBox* lBox = (CComboBox*)GetDlgItem(IDC_COMBO6);

    ((CSerialFrame*)this->GetParent())->OnStopBitChange(lBox->GetItemData(lBox->GetCurSel()));
}

void CSerialView::OnSend()
{
    ((CSerialFrame*)this->GetParent())->OnSendClicked();
}

void CSerialView::OnConnect()
{
    ((CSerialFrame*)this->GetParent())->OnConnectClicked();
}

void CSerialView::OnDisconnect()
{
    ((CSerialFrame*)this->GetParent())->OnDisconnectClicked();
}

void CSerialView::addTxString(const char* String)
{
    CListBox* lList = (CListBox*)GetDlgItem(IDC_LIST1);
    CString   lString = "> ";

    lString += String;

    lList->AddString(lString);

    int nCount = lList->GetCount();
    if (nCount > 0)
        lList->SetCurSel(nCount-1);
}

void CSerialView::addRxString(const char* String)
{
    CListBox* lList = (CListBox*)GetDlgItem(IDC_LIST1);
    CString   lString = "< ";

    lString += String;

    lList->AddString(lString);

    int nCount = lList->GetCount();
    if (nCount > 0)
        lList->SetCurSel(nCount-1);
}

/*
 * Method:    CSerialFrame()
 * Purpose:   class constructor
 * Comments:  none
 */
CSerialFrame::CSerialFrame()
{
}

/*
 * Method:    ~CSerialFrame()
 * Purpose:   class destructor
 * Comments:  none
 */  
CSerialFrame::~CSerialFrame()
{
}

/*
 * Method:    Create()
 * Purpose:   create the window
 * Comments:  none
 */ 
BOOL CSerialFrame::Create(tPvHandle aHandle,tPvCameraInfoEx* pInfo)
{
    BOOL result;

    if((result = CChildFrame::Create(pInfo,CChildFrame::eSerial)))
    {
        m_Handle = aHandle;

        // create the view
        m_View = new CSerialView;
        if(m_View)
            result = m_View->Create(CRect(0,0,WIDTH,HEIGHT),this);
        else
            result = false;

        if(result)
        {
            unsigned long lValue[3];
            unsigned long lRegister[3] = {REG_SIO_INQUIRY,REG_SIO_MODE_INQUIRY,REG_SIO_MODE};

            if(PvRegisterRead(m_Handle,3,lRegister,lValue,NULL) == ePvErrSuccess && (lValue[0] & 3))
                m_View->setupInitial(lValue[1],lValue[2]);
            else
            {
                OutputDebugString("Camera don't support serial?\n");   
            }
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
BOOL CSerialFrame::PreCreateWindow(CREATESTRUCT& cs)
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
void CSerialFrame::OnDestroy()
{
    CChildFrame::OnDestroy();
}

/*
 * Method:    OnTimer()
 * Purpose:   called by the framework when a timer event
 *            occurs
 * Comments:  none
 */
void CSerialFrame::OnTimer(UINT_PTR uId)
{
    unsigned long lLength;
  
    if(ReadStringFromSerialIO(m_Handle,(unsigned char*)&m_String[m_Index],512 - m_Index,&lLength) && lLength)
    {
        m_Index += lLength;
        if(m_String[m_Index - 1] == '\0' || m_String[m_Index - 1] == '\r')
        {
            m_String[m_Index] = '\0';
            m_View->addRxString(m_String);
            m_Index = 0;
        }
    }    
}

/*
 * Method:    OnCommandReceived()
 * Purpose:   called when the window has received a command
 * Comments:  none
 */
void CSerialFrame::OnCommandReceived(const tCommand* pCommand)
{
}

void CSerialFrame::OnRateChange(unsigned int Value)
{
    unsigned long lValue;
    unsigned long lRegister = REG_SIO_MODE;

    if(PvRegisterRead(m_Handle,1,&lRegister,&lValue,NULL) == ePvErrSuccess)
    {
        SetBits(lValue,Value,0,8);
        PvRegisterWrite(m_Handle,1,&lRegister,&lValue,NULL);
    }
}

void CSerialFrame::OnParityChange(unsigned int Value)
{
    unsigned long lValue;
    unsigned long lRegister = REG_SIO_MODE;

    if(PvRegisterRead(m_Handle,1,&lRegister,&lValue,NULL) == ePvErrSuccess)
    {
        SetBits(lValue,Value,8,2);
        PvRegisterWrite(m_Handle,1,&lRegister,&lValue,NULL);
    }    
}

void CSerialFrame::OnCharLenChange(unsigned int Value)
{
    unsigned long lValue;
    unsigned long lRegister = REG_SIO_MODE;

    if(PvRegisterRead(m_Handle,1,&lRegister,&lValue,NULL) == ePvErrSuccess)
    {
        SetBits(lValue,Value,10,2);
        PvRegisterWrite(m_Handle,1,&lRegister,&lValue,NULL);
    }
}

void CSerialFrame::OnStopBitChange(unsigned int Value)
{
    unsigned long lValue;
    unsigned long lRegister = REG_SIO_MODE;

    if(PvRegisterRead(m_Handle,1,&lRegister,&lValue,NULL) == ePvErrSuccess)
    {
        SetBits(lValue,Value,12,2);
        PvRegisterWrite(m_Handle,1,&lRegister,&lValue,NULL);
    }
}

void CSerialFrame::OnSendClicked()
{
    char         lString[512];
    unsigned int lLength;

    if((lLength = m_View->getLine(lString,512)))
    {
        lString[lLength++] = '\r';
        lString[lLength] = '\0';

        if(WriteStringToSerialIO(m_Handle,lString,lLength))
        {
            lString[lLength-1] = '\0';
            m_View->addTxString(lString);  
            m_View->clearLine();
        }
    }
}

void CSerialFrame::OnConnectClicked()
{
    unsigned long lRegisters[3];
	unsigned long lValues[3];

	lRegisters[0]   = REG_SIO_TX_CONTROL;
	lValues[0]	    = 3;  // Reset & enable transmitter
	lRegisters[1]   = REG_SIO_RX_CONTROL;
	lValues[1]	    = 3;  // Reset & enable receiver
	lRegisters[2]   = REG_SIO_RX_STATUS;
	lValues[2]	    = 0xFFFFFFFF;  // Clear status bits

	if(PvRegisterWrite(m_Handle,3,lRegisters, lValues, NULL) == ePvErrSuccess)
    {
        m_Index = 0;
        m_String[0] = '\0';
        m_Timer = SetTimer(1,250,NULL);
        m_View->setupConnected();  
    }
    else
    {
        _OutputDebugString("FAILED!\n");
    }
}

void CSerialFrame::OnDisconnectClicked()
{
    unsigned long lRegisters[2];
	unsigned long lValues[2];

	lRegisters[0]   = REG_SIO_TX_CONTROL;
	lValues[0]	    = 0;  // Reset & enable transmitter
	lRegisters[1]   = REG_SIO_RX_CONTROL;
	lValues[1]	    = 0;  // Reset & enable receiver

	PvRegisterWrite(m_Handle,2,lRegisters, lValues, NULL);

    KillTimer(m_Timer);

    m_View->setupDisconnected();
}

