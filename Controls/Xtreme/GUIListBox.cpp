#include "GUIComponentDisplayer.h"
#include "GUIListBox.h"

GUI_IMPLEMENT_CLONEABLE( GUIListBox, "Listbox" )



GUIListBox::GUIListBox( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id, GR::u32 Type ) :
  AbstractListBox<GUIComponent,GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, Type, Id )
{
  m_ItemHeight       = 16;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
  RecalcClientRect();
  SetSize( NewWidth, NewHeight );
}



void GUIListBox::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetTexture( 0, NULL );

  if ( m_pFont )
  {
    GR::tRect   rcItem;

    int   yOffset = -(int)m_FirstVisibleItem * m_ItemHeight;

    size_t item = GetSelectedItem();
    if ( item != -1 )
    {
      Displayer.DrawQuad( 0, yOffset + (int)item * m_ItemHeight,
                          m_ClientRect.width() - m_pScrollBar->Width(), m_ItemHeight,
                          GetColor( GUI::COL_HIGHLIGHT ) );
    }

    if ( ( m_MouseOverItem != -1 )
    &&   ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) )
    {
      Displayer.DrawQuad( 0, yOffset + (int)m_MouseOverItem * m_ItemHeight,
                          m_ClientRect.width() - m_pScrollBar->Width(), m_ItemHeight,
                          GetColor( GUI::COL_HOTLIGHT ) );
    }

    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

    item = 0;
    tAbstractListBoxItemList::iterator    it( m_Items.begin() );
    while ( it != m_Items.end() )
    {
      tListBoxItem&   Item = *it;

      if ( yOffset >= m_Height )
      {
        break;
      }

      if ( Flags() & GUI::COMPFT_OWNER_DRAW_ITEM )
      {
        if ( GetItemRect( item, rcItem ) )
        {
          GUI::OwnerDrawInfo    odInfo;

          odInfo.pComponent = this;
          odInfo.ItemIndex  = (int)item;
          odInfo.ItemOffset = rcItem.position();
          odInfo.ItemSize   = rcItem.size();
          odInfo.pDisplayer = &Displayer;

          GenerateEvent( OET_OWNER_DRAW_ITEM, ( GR::up ) & odInfo );
        }
      }
      else

      if ( yOffset >= 0.0f )
      {
        GR::u32 dwColor = GetColor( GUI::COL_WINDOWTEXT );
        if ( ( item == m_SelectedItem )
        ||   ( ( item == m_MouseOverItem )
        &&     ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) ) )
        {
          dwColor = GetColor( GUI::COL_HIGHLIGHTTEXT );
        }
        if ( GetItemRect( item, rcItem ) )
        {
          Displayer.DrawText( m_pFont, 0, 0,
                              Item.Text,
                              m_TextAlignment,
                              dwColor,
                              &rcItem );
        }
      }

      yOffset += m_ItemHeight;

      ++item;
      ++it;
    }
  }
}



