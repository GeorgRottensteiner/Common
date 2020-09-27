#include <Debug\debugclient.h>
#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUIListBox.h"



GUIListBox::GUIListBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 lbType, GR::u32 dwId ) :
  CAbstractListBox<GUIComponent,GUIScrollbar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{

  m_iItemHeight = 12;

  ModifyEdge( GUI::GET_SUNKEN_BORDER );

  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );

}



void GUIListBox::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

  int   iYOffset = -(int)m_iFirstVisibleItem * m_iItemHeight;


  GR::tRect   rectSelectedItem;
  
  if ( GetItemRect( m_iMouseOverItem, rectSelectedItem ) )
  {
    pPage->Box( rectSelectedItem.position().x, 
                rectSelectedItem.position().y,
                rectSelectedItem.position().x + rectSelectedItem.size().x - 1, 
                rectSelectedItem.position().y + rectSelectedItem.size().y - 1,
                pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
  }

  if ( !m_listItems.empty() )
  {
    size_t      iItem = m_iFirstVisibleItem;

    GR::tRect   rcItem;


    tAbstractListBoxItemList::iterator    it( m_listItems.begin() );
    std::advance( it, m_iFirstVisibleItem );
    while ( it != m_listItems.end() )
    {
      tListBoxItem&   Item = *it;

      if ( iYOffset >= m_ClientRect.height() )
      {
        break;
      }
      if ( GetItemRect( iItem, rcItem ) )
      {
        DrawText( pPage, Item.m_strText.c_str(), rcItem, GUI::AF_VCENTER );
      }

      iYOffset += m_iItemHeight;

      ++iItem;
      ++it;
    }
  }

  if ( ( GetItemRect( m_iSelectedItem, rectSelectedItem ) )
  &&   ( IsFocused() ) )
  {
    DrawFocusRect( pPage, rectSelectedItem );
  }


}



