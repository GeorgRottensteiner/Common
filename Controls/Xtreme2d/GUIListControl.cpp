#include "GUIComponentDisplayer.h"
#include "GUIListControl.h"



GUI_IMPLEMENT_CLONEABLE( GUIListControl, "ListCtrl" )



GUIListControl::GUIListControl( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Styles, GR::u32 Id ) :
  AbstractListCtrl<GUIComponent, GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, Styles, Id )
{
  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
  SetFont( m_pFont );
}



void GUIListControl::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::u32     color     = 0xff808080;
  GR::tRect   rc;


  if ( Style() & LCS_SHOW_HEADER )
  {
    for ( size_t i = 0; i < Columns(); ++i )
    {
      GetHeaderRect( i, rc );

      Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), 0xffa0a0a0 );
      
      if ( m_pFont )
      {
        Displayer.DrawText( m_pFont, m_vectColumns[i].Description.c_str(), rc, m_vectColumns[i].TextAlignment );
      }
    }
  }

  // Selection
  if ( GetLineRect( m_SelectedItem, rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), 0xff8080ff );
  }
  if ( GetLineRect( m_MouseOverItem, rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), 0xffff00ff );
  }


  size_t    itemIndex = m_Offset;
  bool      done = false;

  if ( Columns() == 0 )
  {
    return;
  }
  do
  {
    for ( size_t columnIndex = 0; columnIndex < Columns(); ++columnIndex )
    {
      if ( !GetItemRect( itemIndex, columnIndex, rc ) )
      {
        done = true;
        break;
      }
      if ( m_pFont )
      {
        Displayer.DrawText( m_pFont, GetItemText( itemIndex, columnIndex ).c_str(), rc, m_vectColumns[columnIndex].TextAlignment, GetColor( GUI::COL_WINDOWTEXT ) );
      }
    }
    ++itemIndex;
  }
  while ( !done );
}



void GUIListControl::SetTextureSection( const GUI::eBorderType Border, const XTextureSection& TexSection )
{
  AbstractListCtrl<GUIComponent, GUIScrollBar>::SetTextureSection( Border, TexSection );

  UpdateScrollBar();
}