/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "GUIComponentDisplayer.h"
#include "GUIButton.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIButton::CGUIButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwId ) :
  CAbstractButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId )
{
  ModifyEdge( GUI::GET_RAISED_BORDER );
}



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIButton::CGUIButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  CAbstractButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
{

  ModifyEdge( GUI::GET_RAISED_BORDER );
}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIButton::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::tRect   rc;

  GetClientRect( rc );

  if ( IsPushed() )
  {
    ModifyEdge( GUI::GET_SUNKEN_BORDER, GUI::GET_RAISED_BORDER );
  }
  else
  {
    ModifyEdge( GUI::GET_RAISED_BORDER, GUI::GET_SUNKEN_BORDER );
  }

  if ( m_pFont )
  {
    if ( IsPushed() )
    {
      rc.offset( 1, 1 );
    }
    DisplayText( pViewer, iOffsetX, iOffsetY, m_strCaption.c_str(), GUI::AF_DEFAULT, GetColor( GUI::COL_WINDOWTEXT ), &rc );
  }
  
  if ( IsFocused() )
  {
    GetClientRect( rc );
    rc.inflate( -1, -1 );
    DrawFocusRect( pViewer, iOffsetX, iOffsetY, rc );
  }

}



