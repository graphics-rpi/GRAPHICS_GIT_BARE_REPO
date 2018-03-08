/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        AboutDlg.cpp		
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the aboutbox dialog class	
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

#include "AboutDlg.h"

//===== LOCAL DATA ============================================================

// message map
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================

/*
 * Method:    CAboutDlg()
 * Purpose:   class constructor
 * Comments:  none
 */
CAboutDlg::CAboutDlg(CWnd* pParentWnd /* = NULL */ ) 
    : CDialog(CAboutDlg::IDD,pParentWnd)
{
}

/*
 * Method:    DoDataExchange()
 * Purpose:   called by the framework to validate the contents of
 *            the dialog
 * Comments:  none
 */
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    // nothing special to do, just call the parent's method
	CDialog::DoDataExchange(pDX);
}