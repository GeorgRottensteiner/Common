#include "GUIComponentDisplayer.h"
#include "GUIListBox.h"



GUI_IMPLEMENT_CLONEABLE( GUIListBox, "ListBox" )



GUIListBox::GUIListBox( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id, GR::u32 ListBoxFlags ) :
  AbstractListBox<GUIComponent,GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, ListBoxFlags, Id )
{
  m_ItemHeight       = 16;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  RecalcClientRect();

  SetSize( NewWidth, NewHeight );
}



void GUIListBox::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  if ( m_pFont )
  {
    GR::tRect   rcItem;

    int   yOffset = -(int)m_FirstVisibleItem * m_ItemHeight;

    size_t itemIndex = GetSelectedItem();
    if ( itemIndex != -1 )
    {
      Displayer.DrawQuad( 0, yOffset + (int)itemIndex * m_ItemHeight,
                           m_ClientRect.Width() - m_pScrollBar->Width(), m_ItemHeight,
                           GetSysColor( GUI::COL_HIGHLIGHT ) );
    }

    if ( ( m_MouseOverItem != -1 )
    &&   ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) )
    {
      Displayer.DrawQuad( 0, yOffset + (int)m_MouseOverItem * m_ItemHeight,
                           m_ClientRect.Width() - m_pScrollBar->Width(), m_ItemHeight,
                           GetSysColor( GUI::COL_HOTLIGHT ) );
    }

    itemIndex = 0;
    tAbstractListBoxItemList::iterator    it( m_Items.begin() );
    while ( it != m_Items.end() )
    {
      tListBoxItem&   Item = *it;

      if ( yOffset >= m_Height )
      {
        break;
      }
      if ( yOffset >= 0.0f )
      {
        DWORD   dwColor = GetSysColor( GUI::COL_WINDOWTEXT );
        if ( ( itemIndex == m_SelectedItem )
        ||   ( ( itemIndex == m_MouseOverItem ) 
        &&     ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) ) )
        {
          dwColor = GetSysColor( GUI::COL_HIGHLIGHTTEXT );
        }
        if ( GetItemRect( itemIndex, rcItem ) )
        {
          Displayer.DrawText( m_pFont, 
                              Item.Text,
                              rcItem,
                              GUI::AF_LEFT | GUI::AF_VCENTER,
                              dwColor );
        }
      }

      yOffset += m_ItemHeight;

      ++itemIndex;
      ++it;
    }
  }
}



