#include "GUIComponentDisplayer.h"
#include "GUICheckbox.h"




GUI_IMPLEMENT_CLONEABLE( GUICheckBox, "CheckBox" )



GUICheckBox::GUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::String& strCaption, GR::u32 Flags, GR::u32 Id ) :
  AbstractCheckBox<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, Flags, Id )
{
}



GUICheckBox::GUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  AbstractCheckBox<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
{

}



void GUICheckBox::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  int     boxSize = 20;

  if ( m_Height - 4 < boxSize )
  {
    boxSize = m_Height - 4;
  }

  if ( m_pFont )
  {
    GR::tRect   rcText( ( m_Height - boxSize ) / 2 + boxSize + 4, 0,
                        m_ClientRect.width() - ( m_Height - boxSize ) / 2 - boxSize - 2, m_ClientRect.height() );

    Displayer.DrawText( m_pFont, m_Caption, rcText, GUI::AF_LEFT | GUI::AF_VCENTER, GetColor( GUI::COL_WINDOWTEXT ) );
  }

  if ( IsPushed() )
  {
    Displayer.DrawQuad( ( m_Height - boxSize ) / 2, ( m_Height - boxSize ) / 2,
                         boxSize, boxSize, GetColor( GUI::COL_BTNFACE ) );
  }
  else
  {
    Displayer.DrawQuad( ( m_Height - boxSize ) / 2, ( m_Height - boxSize ) / 2,
                         boxSize, boxSize, GetSysColor( GUI::COL_WINDOW ) );
  }
  Displayer.DrawEdge( GUI::VFT_SUNKEN_BORDER,
                      GR::tRect( ( m_Height - boxSize ) / 2, ( m_Height - boxSize ) / 2,
                       boxSize, boxSize ),
                      m_TextureSection );
  if ( IsChecked() )
  {
    Displayer.DrawLine( GR::tPoint( ( m_Height - boxSize ) / 2, ( m_Height - boxSize ) / 2 ),
                        GR::tPoint( ( m_Height - boxSize ) / 2 + boxSize - 1, ( m_Height - boxSize ) / 2 + boxSize - 1 ),
                        GetColor( GUI::COL_3DDKSHADOW ) );
    Displayer.DrawLine( GR::tPoint( ( m_Height - boxSize ) / 2 + boxSize - 1, ( m_Height - boxSize ) / 2 ),
                        GR::tPoint( ( m_Height - boxSize ) / 2, ( m_Height - boxSize ) / 2 + boxSize - 1 ),
                        GetColor( GUI::COL_3DDKSHADOW ) );
  }
}



