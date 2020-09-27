/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8/DX8Shader.h>

#include "GUIComponentDisplayer.h"
#include "GUICheckbox.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUICheckBox::CGUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId ) :
  CAbstractCheckBox<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId ),
  m_pTexture( NULL )
{
}



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUICheckBox::CGUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId ) :
  CAbstractCheckBox<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwId ),
  m_pTexture( NULL )
{

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUICheckBox::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::CDX8ShaderAlphaTest::Apply( *pViewer );

  int     iBoxSize = 20;

  if ( m_iHeight - 4 < iBoxSize )
  {
    iBoxSize = m_iHeight - 4;
  }
  
  if ( m_pFont )
  {
    GR::tRect   rcText( iOffsetX + ( m_Height - iBoxSize ) / 2 + iBoxSize + 4, iOffsetY,
                        m_ClientRect.width() - ( m_Height - iBoxSize ) / 2 - iBoxSize - 2, m_ClientRect.height() );
                        
    DisplayText( pViewer, 0, 0, m_strCaption.c_str(), GUI::AF_LEFT | GUI::AF_VCENTER, GetSysColor( GUI::COL_WINDOWTEXT ), &rcText );
  }

  pViewer->DrawRect( iOffsetX + ( m_Height - iBoxSize ) / 2, iOffsetY + ( m_Height - iBoxSize ) / 2,
                     iBoxSize, iBoxSize,
                     GetSysColor( GUI::COL_WINDOW ) );
  if ( IsChecked() )
  {
    pViewer->DrawLine( iOffsetX + ( m_Height - iBoxSize ) / 2, iOffsetY + ( m_Height - iBoxSize ) / 2,
                       iOffsetX + ( m_Height - iBoxSize ) / 2 + iBoxSize - 1, iOffsetY + ( m_Height - iBoxSize ) / 2 + iBoxSize - 1,
                       GetSysColor( GUI::COL_WINDOW ) );
    pViewer->DrawLine( iOffsetX + ( m_Height - iBoxSize ) / 2 + iBoxSize - 1, iOffsetY + ( m_Height - iBoxSize ) / 2,
                       iOffsetX + ( m_Height - iBoxSize ) / 2, iOffsetY + ( m_Height - iBoxSize ) / 2 + iBoxSize - 1,
                       GetSysColor( GUI::COL_WINDOW ) );
  }

}



void CGUICheckBox::SetTexture( CDX8Texture* pTexture )
{

  m_pTexture = pTexture;

}



ICloneAble* CGUICheckBox::Clone()
{

  return new CGUICheckBox( *this );

}