/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Debug\debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUITexturedButton.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUITexturedButton::CGUITexturedButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId ) :
  CAbstractButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId )
{
  ModifyEdge( GUI::GET_RAISED_BORDER );
}



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUITexturedButton::CGUITexturedButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId ) :
  CAbstractButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
{
  ModifyEdge( GUI::GET_RAISED_BORDER );
}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUITexturedButton::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  DWORD   dwTextColor = 0xff000000;

  GR::tRect   rc;

  GetClientRect( rc );

  pViewer->DrawTextureSection( iOffsetX, iOffsetY, m_TextureSectionCenter, 0xffffffff, rc.width(), rc.height() );


  if ( m_pFont )
  {
    if ( IsMouseInside() )
    {
      dwTextColor = 0xff808080;
    }
    if ( IsPushed() )
    {
      rc.offset( 1, 1 );
    }
    DisplayText( pViewer, iOffsetX, iOffsetY, m_strCaption.c_str(), GUI::AF_DEFAULT, dwTextColor, &rc );
  }

}



/*-SetTextureSection----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUITexturedButton::SetTextureSection( const tTextureSection& TexSection )
{

  m_TextureSectionCenter = TexSection;

}



ICloneAble* CGUITexturedButton::Clone()
{

  return new CGUITexturedButton( *this );

}