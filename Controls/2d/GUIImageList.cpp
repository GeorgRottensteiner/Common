#include <Grafik/ContextDescriptor.h>
#include <Grafik/Image.h>

#include "GUIComponentDisplayer.h"
#include "GUIImageList.h"



IMPLEMENT_CLONEABLE( CGUIImageList, "GUIImageList" )

CGUIImageList::CGUIImageList( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 lbType ) :
  CAbstractListBox<CGUIComponent,CGUIScrollbar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{

  m_iItemHeight = 12;
  m_iItemWidth  = m_Width;

  ModifyEdge( GUI::GET_SUNKEN_BORDER );

  m_pScrollBar->GetComponent( CGUIScrollbar::SB_BUTTON_LEFT_UP )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollbar::SB_BUTTON_RIGHT_DOWN )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollbar::SB_SLIDER )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );

}



void CGUIImageList::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

  if  ( m_listItems.empty() )
  {
    return;
  }

  int   iPageXOffset = pPage->GetOffsetX();
  int   iPageYOffset = pPage->GetOffsetY();

  size_t     iItemNr = m_iFirstVisibleItem;
  tAbstractListBoxItemList::iterator    it( m_listItems.begin() );
  std::advance( it, m_iFirstVisibleItem );
  GR::tRect   rectItem;

  while ( it != m_listItems.end() )
  {
    tListBoxItem&   Item = *it;

    if ( !GetItemRect( iItemNr, rectItem ) )
    {
      break;
    }

    GR::Graphic::Image*    pImage = (GR::Graphic::Image*)_atoi64( Item.m_strText.c_str() );
    if ( pImage )
    {
      pImage->PutImage( pPage, rectItem.position().x, rectItem.position().y );
    }

    if ( iItemNr == m_iSelectedItem )
    {
      GR::Graphic::ContextDescriptor    cdPage( pPage );

      cdPage.Rectangle( rectItem.position().x, 
                        rectItem.position().y,
                        rectItem.size().x, rectItem.size().y,
                        0xffffffff );
    }
    if ( iItemNr == m_iMouseOverItem )
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



void CGUIImageList::AddString( GR::Graphic::Image* pImage, GR::up dwItemData )
{

  m_listItems.push_back( tListBoxItem() );

  tListBoxItem&   newItem = m_listItems.back();

  char    szDummy[200];

  wsprintf( szDummy, "%d", pImage );

  newItem.m_strText     = szDummy;
  newItem.m_dwItemData  = dwItemData;

  UpdateScrollBar();

}



