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
#include "GUIDialog.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIDialog::CGUIDialog( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dftType, GR::u32 dwId ) :
  CAbstractDialog<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId, dftType )
{

  ModifyEdge( GUI::GET_RAISED_BORDER | GUI::GET_TRANSPARENT_BKGND );
  CAbstractDialog<CGUIComponent>::RecalcClientRect();

  //m_ClientRect.inflate( -2, -2 );

  

}



void CGUIDialog::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  pViewer->DrawBox( iOffsetX, iOffsetY, m_ClientRect.width(), m_ClientRect.height(), GetSysColor( GUI::COL_BTNFACE ) );

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIDialog::DisplayNonClientOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::tRect   rc;

  if ( GetCaptionRect( rc ) )
  {
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




ICloneAble* CGUIDialog::Clone()
{

  return new CGUIDialog( *this );

}