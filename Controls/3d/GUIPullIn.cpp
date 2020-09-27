/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Debug\debugclient.h>

#include <DX8/DX8Shader.h>

#include "GUIComponentDisplayer.h"
#include "GUIPullIn.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIPullIn::CGUIPullIn( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dftType, GR::u32 dwId ) :
  CAbstractPullIn<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId, dftType )
{

  ModifyEdge( GUI::GET_RAISED_BORDER );

}



void CGUIPullIn::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIPullIn::DisplayNonClientOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::tRect   rc;

  if ( GetCaptionRect( rc ) )
  {
    GR::CDX8ShaderFlat::Apply( *pViewer );

    DWORD   dwCol1 = GetSysColor( GUI::COL_ACTIVECAPTION );
    DWORD   dwCol2 = GetSysColor( GUI::COL_GRADIENTACTIVECAPTION );
    pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), 
                      dwCol1, 
                      dwCol2, 
                      dwCol1, 
                      dwCol2 );
  }

  DisplayText( pViewer, iOffsetX, iOffsetY, m_strCaption.c_str(), 
               GUI::AF_DEFAULT, 
               GetSysColor( GUI::COL_CAPTIONTEXT ), &rc );

  CGUIComponent::DisplayNonClientOnPage( pViewer, iOffsetX, iOffsetY );

}




ICloneAble* CGUIPullIn::Clone()
{

  return new CGUIPullIn( *this );

}