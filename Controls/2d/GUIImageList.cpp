#include <Grafik/ContextDescriptor.h>
#include <Grafik/Image.h>

#include "GUIComponentDisplayer.h"
#include "GUIImageList.h"



IMPLEMENT_CLONEABLE( GUIImageList, "GUIImageList" )

GUIImageList::GUIImageList( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 lbType ) :
  AbstractListBox<GUIComponent,GUIScrollbar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{
  m_ItemHeight = 12;
  m_ItemWidth  = m_Width;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  m_pScrollBar->GetComponent( GUIScrollbar::SB_BUTTON_LEFT_UP )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollbar::SB_BUTTON_RIGHT_DOWN )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollbar::SB_SLIDER )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );
}



void GUIImageList::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  if  ( m_Items.empty() )
  {
    return;
  }

  int   iPageXOffset = pPage->GetOffsetX();
  int   iPageYOffset = pPage->GetOffsetY();

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

    GR::Graphic::Image*    pImage = (GR::Graphic::Image*)_atoi64( Item.Text.c_str() );
    if ( pImage )
    {
      pImage->PutImage( pPage, rectItem.position().x, rectItem.position().y );
    }

    if ( iItemNr == m_SelectedItem )
    {
      GR::Graphic::ContextDescriptor    cdPage( pPage );

      cdPage.Rectangle( rectItem.position().x, 
                        rectItem.position().y,
                        rectItem.size().x, rectItem.size().y,
                        0xffffffff );
    }
    if ( iItemNr == m_MouseOverItem )
    {
      GR::Graphic::ContextDescriptor    cdPage( pPage );

      cdPage.AlphaBox( rectItem.position().x, 
                       rectItem.position().y,
                       rectItem.size().x, rectItem.size().y, 0xff8080ff, 128 );
    }

    ++iItemNr;
    ++it;
  }

}



void GUIImageList::AddString( GR::Graphic::Image* pImage, GR::up dwItemData )
{
  m_Items.push_back( tListBoxItem() );

  tListBoxItem&   newItem = m_Items.back();

  char    szDummy[200];

  wsprintf( szDummy, "%d", pImage );

  newItem.Text      = szDummy;
  newItem.ItemData  = dwItemData;

  UpdateScrollBar();
}



