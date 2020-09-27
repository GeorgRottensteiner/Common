#include "GUIComponentDisplayer.h"
#include "GUIListControl.h"



GUI_IMPLEMENT_CLONEABLE( GUIListControl, "ListCtrl" )



GUIListControl::GUIListControl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwStyles, GR::u32 dwId ) :
  AbstractListCtrl<GUIComponent, GUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, dwStyles, dwId )
{
  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
  SetFont( m_pFont );
}



void GUIListControl::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::u32   dwColor     = 0xff808080;

  GR::tRect   rc;


  // Header darstellen
  if ( Style() & LCS_SHOW_HEADER )
  {
    for ( size_t i = 0; i < Columns(); ++i )
    {
      GetHeaderRect( i, rc );

      Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), 0xffa0a0a0 );
      
      if ( m_pFont )
      {
        Displayer.DrawText( m_pFont, m_vectColumns[i].Description.c_str(), rc, m_vectColumns[i].TextAlignment );
      }
    }
  }

  // Selection
  if ( GetLineRect( m_SelectedItem, rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), 0xff8080ff );
  }
  if ( GetLineRect( m_MouseOverItem, rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), 0xffff00ff );
  }


  size_t    iItem = m_Offset;

  bool      bDone = false;

  if ( Columns() == 0 )
  {
    return;
  }
  do
  {
    for ( size_t iColumn = 0; iColumn < Columns(); ++iColumn )
    {
      if ( !GetItemRect( iItem, iColumn, rc ) )
      {
        bDone = true;
        break;
      }
      if ( m_pFont )
      {
        Displayer.DrawText( m_pFont, GetItemText( iItem, iColumn ).c_str(), rc, m_vectColumns[iColumn].TextAlignment, GetColor( GUI::COL_WINDOWTEXT ) );
      }
    }
    ++iItem;
  }
  while ( !bDone );
}



void GUIListControl::SetTextureSection( const GUI::eBorderType eType, const XTextureSection& TexSection )
{
  AbstractListCtrl<GUIComponent, GUIScrollBar>::SetTextureSection( eType, TexSection );

  UpdateScrollBar();
}