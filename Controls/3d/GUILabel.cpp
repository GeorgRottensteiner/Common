/*--------------------+-------------------------------------------------------+
 | Programmname       : GUILabel                                              |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8/DX8Shader.h>

#include "GUIComponentDisplayer.h"
#include "GUILabel.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUILabel::CGUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwID ) :
  CAbstractLabel<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, GUI::AF_DEFAULT, dwID )
{
}

CGUILabel::CGUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GUI::AlignmentFlagType dwFlags, const char* szCaption, GR::u32 dwID ) :
  CAbstractLabel<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwFlags, dwID )
{
}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUILabel::DisplayOnPage( CD3DViewer* pViewer, int iXOffset, int iYOffset )
{

  GR::CDX8ShaderAlphaTest::Apply( *pViewer );

  if ( m_pFont )
  {
    int   iTextWidth  = m_pFont->TextLength( m_strCaption.c_str() );
    int   iTextHeight = m_pFont->TextHeight( m_strCaption.c_str() );

    pViewer->DrawText( (CDX8Font*)m_pFont, 
                    iXOffset,
                    (int)( iYOffset + ( m_iHeight - iTextHeight ) / 2 ), 
                    m_strCaption.c_str(),
                    GetSysColor( GUI::COL_WINDOWTEXT ) );
  }

}



ICloneAble* CGUILabel::Clone()
{

  return new CGUILabel( *this );

}