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
#include "GUIScreen.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIScreen::CGUIScreen( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  CGUIComponent( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
{
  m_strClassName = "Screen";
  ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIScreen::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

}




ICloneAble* CGUIScreen::Clone()
{

  return new CGUIScreen( *this );

}