// P:\Common\MFC\MultiTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MultiTreeCtrl.h"

#include <debug/debugclient.h>
#include ".\multitreectrl.h"


// CMultiTreeCtrl

IMPLEMENT_DYNAMIC(CMultiTreeCtrl, CTreeCtrl)
CMultiTreeCtrl::CMultiTreeCtrl() :
  m_pDragImage( NULL ),
  m_bLDragging( FALSE ),
  m_hitemDrag( NULL ),
  m_hitemDrop( NULL )
{
}

CMultiTreeCtrl::~CMultiTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CMultiTreeCtrl, CTreeCtrl)
  ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnTvnSelchanging)
  ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
  ON_WM_LBUTTONDOWN()
  ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CMultiTreeCtrl message handlers



BOOL CMultiTreeCtrl::SelectItem( HTREEITEM hItem )
{

  if ( SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED ) )
  {
    CWnd* pWndParent = GetParent();

    if ( pWndParent )
    {
      NMTREEVIEW    nmTV;

      ZeroMemory( &nmTV, sizeof( nmTV ) );
      nmTV.hdr.code     = TVN_SELCHANGED;
      nmTV.hdr.hwndFrom = GetSafeHwnd();
      nmTV.hdr.idFrom   = GetDlgCtrlID();

      nmTV.itemNew.hItem  = hItem;
      nmTV.itemNew.mask   = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
      nmTV.itemNew.stateMask  = TVIS_STATEIMAGEMASK | TVIS_OVERLAYMASK;
      if ( !GetItem( &nmTV.itemNew ) )
      {
        dh::Log( "GetItem failed\n" );
      }
      nmTV.action       = TVC_UNKNOWN;
      GetCursorPos( &nmTV.ptDrag );
      ScreenToClient( &nmTV.ptDrag );
      pWndParent->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmTV );
    }
    return TRUE;
  }
  return FALSE;

  //return CTreeCtrl::SelectItem( hItem );

}



BOOL CMultiTreeCtrl::UnselectItem( HTREEITEM hItem )
{

  if ( ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
  &&   ( SetItemState( hItem, 0, TVIS_SELECTED ) ) )
  {
    CWnd* pWndParent = GetParent();

    if ( pWndParent )
    {
      NMTREEVIEW    nmTV;

      ZeroMemory( &nmTV, sizeof( nmTV ) );
      nmTV.hdr.code     = TVN_SELCHANGED;
      nmTV.hdr.hwndFrom = GetSafeHwnd();
      nmTV.hdr.idFrom   = GetDlgCtrlID();

      nmTV.itemNew.hItem  = hItem;
      nmTV.itemNew.mask   = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
      nmTV.itemNew.stateMask  = TVIS_STATEIMAGEMASK | TVIS_OVERLAYMASK;
      if ( !GetItem( &nmTV.itemNew ) )
      {
        dh::Log( "GetItem failed\n" );
      }
      nmTV.action       = TVC_UNKNOWN;
      GetCursorPos( &nmTV.ptDrag );
      ScreenToClient( &nmTV.ptDrag );
      pWndParent->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmTV );
    }
    return TRUE;
  }
  return FALSE;

}



bool CMultiTreeCtrl::IsItemSelected( HTREEITEM hItem )
{

  if ( hItem == NULL )
  {
    return false;
  }
  return !!( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED );

}



void CMultiTreeCtrl::RemoveSelection()
{

  HTREEITEM   hItem = GetRootItem();

  while ( hItem )
  {
    SetItemState( hItem, 0, TVIS_SELECTED );

    hItem = GetNextItem( hItem );
  }

}



HTREEITEM CMultiTreeCtrl::GetNextSelectedItem( HTREEITEM hItem )
{

  if ( hItem == NULL )
  {
    hItem = GetRootItem();
  }
  else
  {
    hItem = GetNextItem( hItem );
  }

  while ( hItem )
  {
    if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
    {
      return hItem;
    }
    hItem = GetNextItem( hItem );
  }

  return NULL;

}



void CMultiTreeCtrl::OnTvnSelchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
/*
  LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

  if ( pNMTreeView->itemNew.state & TVIS_SELECTED )
  {
    if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
    {
      
    }
    else
    {
      RemoveSelection();
    }

    SelectItem( pNMTreeView->itemNew.hItem );
  }
  else
  {
    UnselectItem( pNMTreeView->itemNew.hItem );
  }
  */


  *pResult = TRUE;
}



void CMultiTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{

  POINT   pt;

  GetCursorPos( &pt );
  ScreenToClient( &pt );
  OnLButtonUp( 0, pt );
  *pResult = 0;

}

void CMultiTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{

  //SetFocus();

  CWnd::OnLButtonDown( nFlags, point );
  return;

  POINT   pt;

  GetCursorPos( &pt );

  ScreenToClient( &pt );

  UINT    uFlags;

  HTREEITEM hItem = HitTest( pt, &uFlags );

  SetFocus();

  if ( ( hItem )
  &&   ( uFlags & TVHT_ONITEM ) )
  {
    if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
    {
    }
    else
    {
      RemoveSelection();
      SelectItem( hItem );
      CTreeCtrl::OnLButtonDown(nFlags, point);
      return;
    }
    if ( IsItemSelected( hItem ) )
    {
      UnselectItem( hItem );
    }
    else
    {
      SelectItem( hItem );
    }
  }
  else
  {
    CTreeCtrl::OnLButtonDown(nFlags, point);
  }

  return;

}



size_t CMultiTreeCtrl::GetItemLevel( HTREEITEM hItem )
{

  size_t    iLevel = 0;

  while ( hItem = GetParentItem( hItem ) )
  {
    iLevel++;
  }

  return iLevel;

}



/*-GetNextItem----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HTREEITEM CMultiTreeCtrl::GetNextItem( HTREEITEM hItem )
{

  HTREEITEM hti = NULL;

  if ( ItemHasChildren( hItem ) )
  {
    hti = CTreeCtrl::GetNextItem( hItem, TVGN_CHILD );
  }

  if ( hti == NULL ) 
  {
    while ( ( hti = GetNextSiblingItem( hItem ) ) == NULL ) 
    {
	    if ( ( hItem = GetParentItem( hItem ) ) == NULL )
      {
        return NULL;
	    }
    }
  }
  return hti;
}




void CMultiTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{

  NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	m_hitemDrag = pNMTreeView->itemNew.hItem;
	m_hitemDrop = NULL;

	m_pDragImage = CreateDragImageEx( m_hitemDrag );  // get the image list for dragging
	// CreateDragImage() returns NULL if no image list
	// associated with the tree view control
	if ( !m_pDragImage )
  {
		return;
  }

	m_bLDragging = TRUE;
	m_pDragImage->BeginDrag(0, CPoint(-15,-15));
	POINT pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	m_pDragImage->DragEnter(NULL, pt);
	SetCapture();

}

void CMultiTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{

  HTREEITEM	hitem;
	UINT		flags;

	if ( m_bLDragging )
	{
		POINT pt = point;
		ClientToScreen( &pt );
		CImageList::DragMove(pt);
		if ((hitem = HitTest(point, &flags)) != NULL)
		{
			CImageList::DragShowNolock(FALSE);
			SelectDropTarget(hitem);
			m_hitemDrop = hitem;

      HTREEITEM prev = GetPrevVisibleItem(hitem);
			HTREEITEM next = GetNextVisibleItem(hitem);
		
			EnsureVisible(prev);		
			EnsureVisible(next);

			CImageList::DragShowNolock(TRUE);
		}
	}

  CTreeCtrl::OnMouseMove(nFlags, point);
}

void CMultiTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
  //CTreeCtrl::OnLButtonUp(nFlags, point);

  CWnd::OnLButtonUp(nFlags, point);

  if (m_bLDragging)
	{
		m_bLDragging = FALSE;
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		ReleaseCapture();

		delete m_pDragImage;

		// Remove drop target highlighting
		SelectDropTarget(NULL);

		if ( m_hitemDrag == m_hitemDrop )
    {
			return;
    }

		// If Drag item is an ancestor of Drop item then return
		HTREEITEM htiParent = m_hitemDrop;
		while ( (htiParent = GetParentItem( htiParent ) ) != NULL )
		{
			if ( htiParent == m_hitemDrag )
      {
        return;
      }
		}

		Expand( m_hitemDrop, TVE_EXPAND );

		HTREEITEM htiNew = CopyBranch( m_hitemDrag, m_hitemDrop, TVI_LAST );
		DeleteItem( m_hitemDrag );

    CWnd* pWndParent = GetParent();

    if ( pWndParent )
    {
      NMTREEVIEW    nmTV;

      ZeroMemory( &nmTV, sizeof( nmTV ) );
      nmTV.hdr.code     = TVN_LAST;
      nmTV.hdr.hwndFrom = GetSafeHwnd();
      nmTV.hdr.idFrom   = GetDlgCtrlID();

      nmTV.itemNew.hItem  = htiNew;
      nmTV.itemNew.mask   = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
      nmTV.itemNew.stateMask  = TVIS_STATEIMAGEMASK | TVIS_OVERLAYMASK;
      if ( !GetItem( &nmTV.itemNew ) )
      {
        dh::Log( "GetItem failed\n" );
      }
      nmTV.action       = TVC_BYMOUSE;
      GetCursorPos( &nmTV.ptDrag );
      ScreenToClient( &nmTV.ptDrag );
      pWndParent->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmTV );
    }

		SelectItem( htiNew );
	}
  else
  {
    POINT   pt;

    GetCursorPos( &pt );

    ScreenToClient( &pt );

    UINT    uFlags;

    HTREEITEM hItem = HitTest( pt, &uFlags );

    SetFocus();

    if ( ( hItem )
    &&   ( uFlags & TVHT_ONITEM ) )
    {
      if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
      {
      }
      else
      {
        RemoveSelection();
        SelectItem( hItem );
        //CTreeCtrl::OnLButtonDown(nFlags, point);
        return;
      }
      if ( IsItemSelected( hItem ) )
      {
        UnselectItem( hItem );
      }
      else
      {
        SelectItem( hItem );
      }
    }
    else
    {
      //CTreeCtrl::OnLButtonDown(nFlags, point);
    }
  }

}



HTREEITEM CMultiTreeCtrl::CopyItem( HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter )
{

  TV_INSERTSTRUCT         tvstruct;
  HTREEITEM               hNewItem;
  CString                 sText;


  // get information of the source item
  tvstruct.item.hItem = hItem;
  tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | 
                          TVIF_IMAGE | TVIF_SELECTEDIMAGE;
  GetItem( &tvstruct.item );  
  sText = GetItemText( hItem );
  
  tvstruct.item.cchTextMax = sText.GetLength();
  tvstruct.item.pszText = sText.LockBuffer();

  // Insert the item at proper location
  tvstruct.hParent = htiNewParent;
  tvstruct.hInsertAfter = htiAfter;
  tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  hNewItem = InsertItem(&tvstruct);
  sText.ReleaseBuffer();

  // Now copy item data and item state.
  SetItemData( hNewItem, GetItemData( hItem ) );
  SetItemState( hNewItem, GetItemState( hItem, TVIS_STATEIMAGEMASK ), TVIS_STATEIMAGEMASK );

  return hNewItem;

}



HTREEITEM CMultiTreeCtrl::CopyBranch( HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter )
{

  HTREEITEM hChild;

  HTREEITEM hNewItem = CopyItem( htiBranch, htiNewParent, htiAfter );
  hChild = GetChildItem( htiBranch );
  while ( hChild != NULL )
  {
    // recursively transfer all the items
    CopyBranch( hChild, hNewItem );  
    hChild = GetNextSiblingItem( hChild );
  }
  return hNewItem;

}

LRESULT CMultiTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  // TODO: Fügen Sie hier Ihren spezialisierten Code ein, und/oder rufen Sie die Basisklasse auf.

  return CTreeCtrl::WindowProc(message, wParam, lParam);
}



size_t CMultiTreeCtrl::GetSelectedItemCount()
{

  size_t    iItems = 0;

  HTREEITEM   hItem = GetNextSelectedItem();
  while ( hItem )
  {
    ++iItems;
    hItem = GetNextSelectedItem( hItem );
  }
  return iItems;

}



CImageList* CMultiTreeCtrl::CreateDragImageEx( HTREEITEM hItem )
{

  /*
	if ( GetImageList(TVSIL_NORMAL) != NULL)
		return CreateDragImage(hItem);
    */

	CRect rect;
	GetItemRect( hItem, rect, TRUE );
	rect.top = rect.left = 0;

	// Create bitmap
	CClientDC	dc( this );
	CDC 		memDC;	

	if ( !memDC.CreateCompatibleDC( &dc ) )
  {
		return NULL;
  }

	CBitmap bitmap;

	if ( !bitmap.CreateCompatibleBitmap( &dc, rect.Width(), rect.Height() ) )
  {
		return NULL;
  }

  int   iSelectedItems = GetSelectedItemCount();

	CBitmap* pOldMemDCBitmap = memDC.SelectObject( &bitmap );
	CFont* pOldFont = memDC.SelectObject( GetFont() );

	memDC.FillSolidRect( &rect, RGB( 0, 255, 0 ) ); // Here green is used as mask color
	memDC.SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );

  if ( iSelectedItems <= 1 )
  {
    CImageList*   pIcons = GetImageList( TVSIL_NORMAL );
    if ( pIcons )
    {
      int   iImage, iDummy;
      POINT ptPos;

      ptPos.x = 0;
      ptPos.y = 0;
      GetItemImage( hItem, iImage, iDummy );

      pIcons->Draw( &memDC, iImage, ptPos, ILD_NORMAL );

      IMAGEINFO   ii;
      pIcons->GetImageInfo( iImage, &ii );

      rect.left += ii.rcImage.right;
    }
	  memDC.TextOut( rect.left, rect.top, GetItemText( hItem ) );
  }
  else
  {
    memDC.TextOut( rect.left, rect.top, "Multiple Items" );
  }

	memDC.SelectObject( pOldFont );
	memDC.SelectObject( pOldMemDCBitmap );

	CImageList* pImageList = new CImageList;
	pImageList->Create( rect.Width(), rect.Height(), ILC_COLOR | ILC_MASK, 0, 1 );
	pImageList->Add( &bitmap, RGB( 0, 255, 0 ) );

	return pImageList;
}
