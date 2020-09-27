#include <Grafik\ContextDescriptor.h>
#include <Grafik/Image.h>
#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUITreeCtrl.h"



CGUITreeCtrl::CGUITreeCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 lbType, GR::u32 dwId ) :
  CAbstractTreeCtrl<CGUIComponent,CGUIScrollbar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{

  m_iItemHeight = 16;
  m_iOffset     = 0;

  ModifyEdge( GUI::GET_SUNKEN_BORDER );

  m_pScrollBar->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollbar::SB_BUTTON_LEFT_UP )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollbar::SB_BUTTON_RIGHT_DOWN )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollbar::SB_SLIDER )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );

  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );

}



void CGUITreeCtrl::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

  GR::Graphic::ContextDescriptor    cdPage( pPage );

  GR::tRect   rectItem;
  
  tTree::iterator    it( m_itFirstVisibleItem );
  while ( it != m_treeItems.end() )
  {
    tTreeItem&   Item = *it;

    if ( GetItemRect( it, rectItem ) )
    {
      if ( ItemHasChildren( it ) )
      {
        GR::tRect   rectButton;
        if ( GetToggleButtonRect( it, rectButton ) )
        {
          rectButton.inflate( -4, -4 );
          pPage->Rectangle( rectButton.position().x,
                            rectButton.position().y,
                            rectButton.position().x + rectButton.size().x,
                            rectButton.position().y + rectButton.size().y,
                            pPage->GetRGB256( 0xc0c0c0 ) );

          pPage->Line( rectButton.position().x + 2, 
                       rectButton.position().y + rectButton.size().y / 2,
                       rectButton.position().x + rectButton.size().x - 2,
                       rectButton.position().y + rectButton.size().y / 2,
                       pPage->GetRGB256( 0xc0c0c0 ) );
          if ( IsItemCollapsed( it ) )
          {
            pPage->Line( rectButton.position().x + rectButton.size().x / 2, 
                         rectButton.position().y + 2,
                         rectButton.position().x + rectButton.size().x / 2,
                         rectButton.position().y + rectButton.size().y - 2,
                         pPage->GetRGB256( 0xc0c0c0 ) );
          }
        }
      }
      GR::tRect   rectExtraIdent;
      if ( GetExtraIdentRect( it, rectExtraIdent ) )
      {
        if ( ( it->m_dwCtrlItemData >= 0 )
        &&   ( it->m_dwCtrlItemData < m_VectImages.size() ) )
        {
          GR::Graphic::Image*    pImage = m_VectImages[it->m_dwCtrlItemData];

          if ( pImage )
          {
            pImage->PutImage( pPage, rectExtraIdent.position().x, rectExtraIdent.position().y );
          }
        }
      }

      GR::tRect   rcOldClip;

      rcOldClip.Left   = pPage->GetLeftBorder();
      rcOldClip.Top    = pPage->GetTopBorder();
      rcOldClip.Right  = pPage->GetRightBorder();
      rcOldClip.Bottom = pPage->GetBottomBorder();

      GR::tRect   rcClient;

      GetClientRect( rcClient );
      LocalToScreen( rcClient, this );

      GR::tRect   rcTemp( rectItem );
      LocalToScreen( rcTemp, this );
      rcTemp = rcTemp.intersection( rcClient );
      pPage->SetRange( rcTemp.Left, rcTemp.Top, 
                        rcTemp.Right, rcTemp.Bottom );

      DrawText( pPage, Item.m_strText.c_str(), rectItem, GUI::AF_LEFT | GUI::AF_VCENTER );

      if ( it == m_itSelectedItem )
      {
        cdPage.AlphaBox( rectItem.position().x, 
                         rectItem.position().y,
                         rectItem.size().x, 
                         rectItem.size().y,
                         0xffffff, 128 );
      }
      else if ( it == m_itMouseOverItem )
      {
        cdPage.AlphaBox( rectItem.position().x, 
                         rectItem.position().y,
                         rectItem.size().x, 
                         rectItem.size().y,
                         0x3030a0, 128 );
      }

      pPage->SetRange( rcOldClip.Left, rcOldClip.Top, 
                       rcOldClip.Right, rcOldClip.Bottom );
    }
    else
    {
      break;
    }

    it = GetNextVisibleItem( it );
  }

}



size_t CGUITreeCtrl::AddImage( GR::Graphic::Image* pImage )
{

  m_VectImages.push_back( pImage );

  return m_VectImages.size() - 1;

}



void CGUITreeCtrl::SetItemImage( TREEITEM hItem, DWORD dwImage )
{

  if ( hItem == m_treeItems.end() )
  {
    return;
  }
  hItem->m_dwCtrlItemData = dwImage;

}



GR::u32 CGUITreeCtrl::GetItemImage( TREEITEM hItem ) const
{

  if ( hItem == m_treeItems.end() )
  {
    return 0;
  }
  return hItem->m_dwCtrlItemData;

}



TREEITEM CGUITreeCtrl::InsertItem( const char* szString, DWORD dwItemImage, DWORD dwItemData )
{

  TreeIterator  itNew = m_treeItems.insert( tTreeItem() );

  tTreeItem&    newItem = *itNew;

  newItem.m_strText         = szString;
  newItem.m_dwItemData      = dwItemData;
  newItem.m_dwCtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_treeItems.end() )
  {
    m_itFirstVisibleItem = m_treeItems.begin();
  }

  UpdateScrollBar();

  return itNew;

}



TREEITEM CGUITreeCtrl::InsertItem( TREEITEM hItemParent, const char* szString, DWORD dwItemImage, DWORD dwItemData )
{

  if ( hItemParent == TreeIterator() )
  {
    return InsertItem( szString, dwItemData );
  }

  TreeIterator it = m_treeItems.insert_child( hItemParent, tTreeItem() );

  tTreeItem&   newItem = *it;

  newItem.m_strText         = szString;
  newItem.m_dwItemData      = dwItemData;
  newItem.m_dwCtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_treeItems.end() )
  {
    m_itFirstVisibleItem = m_treeItems.begin();
  }

  UpdateScrollBar();
  return it;
}



