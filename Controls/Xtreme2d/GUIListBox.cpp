#include "GUIComponentDisplayer.h"
#include "GUIListBox.h"



GUI_IMPLEMENT_CLONEABLE( GUIListBox, "ListBox" )



GUIListBox::GUIListBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 lbType ) :
  AbstractListBox<GUIComponent,GUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{
  m_ItemHeight       = 16;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  RecalcClientRect();

  SetSize( iNewWidth, iNewHeight );
}



void GUIListBox::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  if ( m_pFont )
  {
    GR::tRect   rcItem;

    int   iYOffset = -(int)m_FirstVisibleItem * m_ItemHeight;

    size_t iItem = GetSelectedItem();
    if ( iItem != -1 )
    {
      Displayer.DrawQuad( 0, iYOffset + (int)iItem * m_ItemHeight,
                           m_ClientRect.width() - m_pScrollBar->Width(), m_ItemHeight,
                           GetSysColor( GUI::COL_HIGHLIGHT ) );
    }

    if ( ( m_MouseOverItem != -1 )
    &&   ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) )
    {
      Displayer.DrawQuad( 0, iYOffset + (int)m_MouseOverItem * m_ItemHeight,
                           m_ClientRect.width() - m_pScrollBar->Width(), m_ItemHeight,
                           GetSysColor( GUI::COL_HOTLIGHT ) );
    }

    iItem = 0;
    tAbstractListBoxItemList::iterator    it( m_Items.begin() );
    while ( it != m_Items.end() )
    {
      tListBoxItem&   Item = *it;

      if ( iYOffset >= m_Height )
      {
        break;
      }
      if ( iYOffset >= 0.0f )
      {
        DWORD   dwColor = GetSysColor( GUI::COL_WINDOWTEXT );
        if ( ( iItem == m_SelectedItem )
        ||   ( ( iItem == m_MouseOverItem ) 
        &&     ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) ) )
        {
          dwColor = GetSysColor( GUI::COL_HIGHLIGHTTEXT );
        }
        if ( GetItemRect( iItem, rcItem ) )
        {
          Displayer.DrawText( m_pFont, 
                              Item.Text,
                              rcItem,
                              GUI::AF_LEFT | GUI::AF_VCENTER,
                              dwColor );
        }
      }

      iYOffset += m_ItemHeight;

      ++iItem;
      ++it;
    }
  }
}



