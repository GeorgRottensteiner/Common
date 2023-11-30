#include <Grafik\ContextDescriptor.h>
#include <Grafik/Image.h>
#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUITreeCtrl.h"



GUITreeCtrl::GUITreeCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 lbType, GR::u32 dwId ) :
  AbstractTreeCtrl<GUIComponent,GUIScrollbar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{
  m_ItemHeight = 16;
  m_Offset     = 0;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  m_pScrollBar->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollbar::SB_BUTTON_LEFT_UP )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollbar::SB_BUTTON_RIGHT_DOWN )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollbar::SB_SLIDER )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );

  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );
}



void GUITreeCtrl::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  GR::Graphic::ContextDescriptor    cdPage( pPage );

  GR::tRect   rectItem;
  
  tTree::iterator    it( m_itFirstVisibleItem );
  while ( it != m_TreeItems.end() )
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
      if ( GetExtraIndentRect( it, rectExtraIdent ) )
      {
        if ( ( it->CtrlItemData >= 0 )
        &&   ( it->CtrlItemData < m_VectImages.size() ) )
        {
          GR::Graphic::Image*    pImage = m_VectImages[it->CtrlItemData];

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
      GUIComponent::LocalToScreen( rcClient );

      GR::tRect   rcTemp( rectItem );
      GUIComponent::LocalToScreen( rcTemp );
      rcTemp = rcTemp.intersection( rcClient );
      pPage->SetRange( rcTemp.Left, rcTemp.Top, 
                        rcTemp.Right, rcTemp.Bottom );

      DrawText( pPage, Item.Text.c_str(), rectItem, GUI::AF_LEFT | GUI::AF_VCENTER );

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



size_t GUITreeCtrl::AddImage( GR::Graphic::Image* pImage )
{
  m_VectImages.push_back( pImage );

  return m_VectImages.size() - 1;
}



void GUITreeCtrl::SetItemImage( TREEITEM hItem, DWORD dwImage )
{
  if ( hItem == m_TreeItems.end() )
  {
    return;
  }
  hItem->CtrlItemData = dwImage;
}



GR::u32 GUITreeCtrl::GetItemImage( TREEITEM hItem ) const
{
  if ( hItem == m_TreeItems.end() )
  {
    return 0;
  }
  return hItem->CtrlItemData;
}



TREEITEM GUITreeCtrl::InsertItem( const char* szString, DWORD dwItemImage, DWORD dwItemData )
{
  TreeIterator  itNew = m_TreeItems.insert( tTreeItem() );

  tTreeItem&    newItem = *itNew;

  newItem.Text          = szString;
  newItem.ItemData      = dwItemData;
  newItem.CtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_TreeItems.end() )
  {
    m_itFirstVisibleItem = m_TreeItems.begin();
  }

  UpdateScrollBar();

  return itNew;
}



TREEITEM GUITreeCtrl::InsertItem( TREEITEM hItemParent, const char* szString, DWORD dwItemImage, DWORD dwItemData )
{
  if ( hItemParent == TreeIterator() )
  {
    return InsertItem( szString, dwItemData );
  }

  TreeIterator it = m_TreeItems.insert_child( hItemParent, tTreeItem() );

  tTreeItem&   newItem = *it;

  newItem.Text          = szString;
  newItem.ItemData      = dwItemData;
  newItem.CtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_TreeItems.end() )
  {
    m_itFirstVisibleItem = m_TreeItems.begin();
  }

  UpdateScrollBar();
  return it;
}



