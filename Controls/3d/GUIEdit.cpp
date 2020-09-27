/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "GUIComponentDisplayer.h"
#include "GUIEdit.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIEdit::CGUIEdit( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 efType, GR::u32 dwId ) :
  CAbstractEdit<CGUIComponent,CGUIScrollBar,CGUISlider,CGUIButton>( iNewX, iNewY, iNewWidth, iNewHeight, efType, dwId )
{
  m_textAlignment = GUI::AF_LEFT | GUI::AF_VCENTER;

  ModifyEdge( GUI::GET_SUNKEN_BORDER );
}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIEdit::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::tRect   rc;

  GetClientRect( rc );
  if ( IsEnabled() )
  {
    pViewer->DrawBox( iOffsetX, iOffsetY, rc.width(), rc.height(), GetSysColor( GUI::COL_WINDOW ) );
  }
  else
  {
    pViewer->DrawBox( iOffsetX, iOffsetY, rc.width(), rc.height(), GetSysColor( GUI::COL_BTNFACE ) );
  }

  /*
  if ( GetSelectionRect( rc ) )
  {
    pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), GetSysColor( GUI::COL_HIGHLIGHT ) );
  }
  */

  // Cursor
  if ( GetCursorRect( rc ) )
  {
    pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), GetSysColor( GUI::COL_WINDOWTEXT ) );
  }

  DisplayText( pViewer, iOffsetX, iOffsetY, m_strCaption.substr( m_iTextOffset ).c_str(), m_textAlignment, GetSysColor( GUI::COL_WINDOWTEXT ) );
  if ( HasSelection() )
  {
    size_t  iSelStart = m_iSelectionAnchor,
            iSelEnd = m_iCursorPosInText;

    if ( iSelStart > iSelEnd )
    {
      iSelStart = m_iCursorPosInText;
      iSelEnd   = m_iSelectionAnchor;
    }
    if ( iSelStart < m_iTextOffset )
    {
      iSelStart = m_iTextOffset;
    }
    if ( iSelStart >= m_iTextOffset )
    {
      int   iDeltaX = m_pFont->TextLength( m_strCaption.substr( m_iTextOffset, iSelStart - m_iTextOffset ).c_str() );
      if ( iDeltaX )
      {
        iDeltaX += 2;
      }

      DisplayText( pViewer, iOffsetX + iDeltaX, iOffsetY, m_strCaption.substr( iSelStart, iSelEnd - iSelStart ).c_str(), m_textAlignment, GetSysColor( GUI::COL_HIGHLIGHTTEXT ) );
    }
  }

}



/*-SetTexture-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIEdit::SetTexture( CDX8Texture* pTexture )
{

  m_pTexture = pTexture;

}




ICloneAble* CGUIEdit::Clone()
{

  return new CGUIEdit( *this );

}