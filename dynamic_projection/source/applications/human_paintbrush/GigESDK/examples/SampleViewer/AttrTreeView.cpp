/*
| ==============================================================================
| Copyright (C) 2006-2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| File:	        AttrTreeView.cpp	
|
| Project/lib:  SampleViewer	
|
| Target:		
|
| Description:  Implement the Camera attributes tree view	
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
#include "AttrTreeView.h"

#include <math.h>

//===== CONSTANTS =============================================================

// percent of the width that should be allocated to the 1st column
const double KMainColumnPercent = 0.6;

//===== LOCAL DATA ============================================================

IMPLEMENT_DYNCREATE(CAttrTreeView, CColumnTreeView)

// message map
BEGIN_MESSAGE_MAP(CAttrTreeView, CColumnTreeView)
	ON_NOTIFY(TVN_SELCHANGED, TreeID, OnItemSelection)
    ON_WM_SIZE()
END_MESSAGE_MAP()

//===== CLASS IMPLEMENTATIONS =================================================
 
/*
 * Method:    CAttrTreeView()
 * Purpose:   constructor
 * Arguments: none
 * Return:    none
 * Comments:  none
 */       
CAttrTreeView::CAttrTreeView()
{
    m_Observer = NULL;
}

/*
 * Method:    ~CAttrTreeView()
 * Purpose:   destructor
 * Arguments: none
 * Return:    none
 * Comments:  none
 */
CAttrTreeView::~CAttrTreeView()
{
}

/*
 * Method:    Create()
 * Purpose:   create the view
 * Comments:  none
 */ 
BOOL CAttrTreeView::Create(PAttrTreeViewObserver* pObserver,ULONG nWidth,ULONG nHeight,CWnd* pParentWnd,UINT nID)
{
    m_Observer = pObserver;

    if(CColumnTreeView::Create(NULL,_T("tree"),WS_VISIBLE,CRect(0,0,nWidth-1,nHeight-1),pParentWnd,nID))
    {
        HDITEM hditem;

        // create the two columns
        hditem.mask = HDI_TEXT | HDI_FORMAT;
        hditem.fmt = HDF_CENTER | HDF_STRING;
        hditem.pszText = "Attributes";
        m_Header.InsertItem(0, &hditem);
        hditem.pszText = "Values";
        m_Header.InsertItem(1, &hditem);

        // increase the height
        //m_Tree.SetItemHeight(m_Tree.GetItemHeight() + 4);
   
        // add the root item
        if(m_Tree.InsertItem(_T("/"),NULL,TVI_FIRST))
            return true;
        else
            return false;
    }
    else
        return false;
}

/*
void CAttrTreeView::GetSelectedItemBox(CRect& rect)
{
    HTREEITEM hItem;
    
    if((hItem = m_Tree.GetSelectedItem()))
        if(m_Tree.GetItemRect(hItem,rect,true))
        {
            HDITEM hditem;
            CRect hrect;

            m_Header.GetWindowRect(&hrect);

            rect.top    += hrect.Height();
            rect.bottom += hrect.Height();

            hditem.mask = HDI_WIDTH;
            m_Header.GetItem(0,&hditem);
            rect.left  = hditem.cxy;
            m_Header.GetItem(1,&hditem);
            rect.right = rect.left + hditem.cxy - 1;
        }
}
*/

/*
 * Method:    OnItemSelection()
 * Purpose:   called when a item from the tree was selected
 * Comments:  none
 */ 
void CAttrTreeView::OnItemSelection(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;

	// find the label of the selected item
	CString strLabel;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if (hItem && !m_Tree.ItemHasChildren(hItem))
	{
		strLabel = m_Tree.GetItemText(hItem);
		int nPos = strLabel.Find('\t');
		if (nPos >= 0)
			strLabel = strLabel.Left(nPos);

        if(m_Observer)
            m_Observer->AttrSelected(strLabel);
	}

	*pResult = 0;
}

/*
 * Method:    OnSize()
 * Purpose:   called by the framework when the view was resized
 * Comments:  none
 */
void CAttrTreeView::OnSize(UINT nType, int cx, int cy)
{
    HDITEM hditem;

    // save space for the scrollbar
    cx -= 15;
        
    hditem.mask = HDI_WIDTH;

    // we resize the column to fit in the new width of the
    // view

    m_Header.GetItem(0,&hditem);
    hditem.cxy = (int)floor((float)cx * KMainColumnPercent);
    m_Header.SetItem(0,&hditem);
    m_Header.GetItem(1,&hditem);
    hditem.cxy = cx - (int)floor((float)cx * KMainColumnPercent);
    m_Header.SetItem(1,&hditem);

    UpdateColumns();

    CColumnTreeView::OnSize(nType,cx,cy);
}
