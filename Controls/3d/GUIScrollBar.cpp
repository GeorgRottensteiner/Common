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
#include "GUIScrollBar.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIScrollBar::CGUIScrollBar( int iNewX, int iNewY, int iNewWidth, int iNewHeight, ScrollbarFlagType sfType, DWORD dwId ) :
  CAbstractScrollbar<CGUIComponent, CGUIButton, CGUISlider>( iNewX, iNewY, iNewWidth, iNewHeight, sfType, dwId )
{

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIScrollBar::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  DWORD   dwColor     = 0xff606060;

  GR::tRect   rc;

  GetClientRect( rc );

  pViewer->DrawBox( D3DXVECTOR3( (float)iOffsetX, (float)iOffsetY, 0 ), D3DXVECTOR3( (float)rc.size().x, (float)rc.size().y, 0 ), dwColor );

}



