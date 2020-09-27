#include <Misc/Misc.h>

#include "GUIComponentDisplayer.h"
#include "GUIImageList.h"



GUI_IMPLEMENT_CLONEABLE( GUIImageList, "ImageList" )



GUIImageList::GUIImageList( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 lbType ) :
  AbstractListBox<GUIComponent,GUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{
  m_ItemHeight = 12;
  m_ItemWidth  = m_Width;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  m_pScrollBar->GetComponent( GUIScrollBar::SB_BUTTON_LEFT_UP )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollBar::SB_BUTTON_RIGHT_DOWN )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollBar::SB_SLIDER )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );
}



void GUIImageList::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  if  ( m_Items.empty() )
  {
    return;
  }

  size_t     iItemNr = m_FirstVisibleItem;
  tAbstractListBoxItemList::iterator    it( m_Items.begin() );
  std::advance( it, m_FirstVisibleItem );
  GR::tRect   rectItem;

  while ( it != m_Items.end() )
  {
    tListBoxItem&   Item = *it;

    if ( !GetItemRect( iItemNr, rectItem ) )
    {
      break;
    }

    XTextureSection*    pTexSec = (XTextureSection*)_atoi64( Item.Text.c_str() );
    if ( pTexSec )
    {
      Displayer.DrawTextureSection( rectItem.position().x, rectItem.position().y, *pTexSec );
    }

    ++iItemNr;
    ++it;
  }

  if ( GetItemRect( m_SelectedItem, rectItem ) )
  {
    Displayer.DrawRect( rectItem.position(),
                        rectItem.size(),
                        0xffffffff );
  }
  if ( GetItemRect( m_MouseOverItem, rectItem ) )
  {
    Displayer.DrawQuad( rectItem.Left,
                        rectItem.Top,
                        rectItem.width(), rectItem.height(), 0x808080ff );
  }
}



void GUIImageList::AddString( const XTextureSection& TexSec, GR::u32 dwItemData )
{
  m_Items.push_back( tListBoxItem() );

  tListBoxItem&   newItem = m_Items.back();

  newItem.Text      = CMisc::printf( "%d", &TexSec );
  newItem.ItemData  = dwItemData;

  UpdateScrollBar();
}



void GUIImageList::SetItemImage( const size_t iIndex, const XTextureSection& TexSec )
{
  if ( iIndex >= m_Items.size() )
  {
    return;
  }
  SetItemText( iIndex, CMisc::printf( "%d", &TexSec ) );
}




