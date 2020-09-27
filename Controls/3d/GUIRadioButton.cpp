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
#include "GUIRadioButton.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIRadioButton::CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId ) :
  CAbstractRadioButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId ),
  m_dwPushedColor( 0xffffffff ),
  m_dwMouseOverColor( 0xffffa000 )
{
}



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIRadioButton::CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId ) :
  CAbstractRadioButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwId ),
  m_dwPushedColor( 0xffffffff ),
  m_dwMouseOverColor( 0xffffa000 )
{

  SetColor( GUI::COL_WINDOWTEXT, 0xffffffff );

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIRadioButton::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  DWORD   dwColor = GetColor( GUI::COL_WINDOWTEXT );

  if ( !IsEnabled() )
  {
    dwColor = 0xff606060;
  }
  else
  {
    if ( Style() & RBFT_PUSHED )
    {
      dwColor = m_dwPushedColor;
    }
    else if ( IsMouseInside() )
    {
      dwColor = m_dwMouseOverColor;
    }
  }

  PrepareStatesForAlpha( pViewer, dwColor );

  //DrawFrame( pViewer, m_pTexture, dwColor, fOffsetX, fOffsetY );
  int     iBoxSize = 20;

  if ( m_iHeight - 4 < iBoxSize )
  {
    iBoxSize = m_iHeight - 4;
  }
  
  if ( m_pFont )
  {
    int   iTextWidth  = m_pFont->TextLength( m_strCaption.c_str() );
    int   iTextHeight = m_pFont->TextHeight( m_strCaption.c_str() );

    pViewer->DrawText( (CDX8Font*)m_pFont, 
                    iOffsetX + iBoxSize + ( m_iWidth - iTextWidth - iBoxSize ) / 2, 
                    iOffsetY + ( m_iHeight - iTextHeight ) / 2, 
                    m_strCaption.c_str(),
                    dwColor );
  }

  pViewer->DrawRect( iOffsetX + ( m_iHeight - iBoxSize ) / 2, iOffsetY + ( m_iHeight - iBoxSize ) / 2,
                     iBoxSize, iBoxSize,
                     0xffffffff );
  if ( IsChecked() )
  {
    pViewer->DrawLine( iOffsetX + ( m_iHeight - iBoxSize ) / 2, iOffsetY + ( m_iHeight - iBoxSize ) / 2,
                       iOffsetX + ( m_iHeight - iBoxSize ) / 2 + iBoxSize - 1, iOffsetY + ( m_iHeight - iBoxSize ) / 2 + iBoxSize - 1,
                       dwColor );
    pViewer->DrawLine( iOffsetX + ( m_iHeight - iBoxSize ) / 2 + iBoxSize - 1, iOffsetY + ( m_iHeight - iBoxSize ) / 2,
                       iOffsetX + ( m_iHeight - iBoxSize ) / 2, iOffsetY + ( m_iHeight - iBoxSize ) / 2 + iBoxSize - 1,
                       dwColor );
  }

  pViewer->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  pViewer->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
  pViewer->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

  pViewer->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
  pViewer->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
  pViewer->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

}



ICloneAble* CGUIRadioButton::Clone()
{

  return new CGUIRadioButton( *this );

}