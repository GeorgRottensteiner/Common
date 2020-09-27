/*--------------------+-------------------------------------------------------+
 | Programmname       : TIB-Controls                                          |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Debug\debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUIImage.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIImage::CGUIImage( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  CGUIComponent( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
{

  RecalcClientRect();

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIImage::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::tRect   rcClient;
  
  GetClientRect( rcClient );

  pViewer->DrawTextureSection( iOffsetX, iOffsetY, m_TexSection, -1, rcClient.width(), rcClient.height() );

}



/*-SetTexture-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIImage::SetImage( const tTextureSection& TexSec )
{

  m_TexSection = TexSec;

}



ICloneAble* CGUIImage::Clone()
{

  return new CGUIImage( *this );

}
