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
#include "GUISlider.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUISlider::CGUISlider( int iNewX, int iNewY, int iNewWidth, int iNewHeight, SliderFlagType sftFlags, DWORD dwId ) :
  CAbstractSlider<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, sftFlags, dwId )
{

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUISlider::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  DWORD   dwColor     = 0xff606060;

  GR::tRect   rc;

  GetClientRect( rc );

  pViewer->DrawBox( iOffsetX, iOffsetY, rc.width(), rc.height(), dwColor );

  GetSliderRect( rc );

  pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), 0xff808080 );

}



