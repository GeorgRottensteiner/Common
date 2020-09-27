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
#include "GUIListBox.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIListBox::CGUIListBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 lbType ) :
  CAbstractListBox<CGUIComponent,CGUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{

  m_iItemHeight       = 16;

  ModifyEdge( GUI::GET_SUNKEN_BORDER );

  RecalcClientRect();

  SetSize( iNewWidth, iNewHeight );

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIListBox::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  pViewer->SetTexture( 0, NULL );

  if ( !( m_Edge & GUI::GET_TRANSPARENT_BKGND ) )
  {
    pViewer->DrawQuad( iOffsetX, iOffsetY,
                       m_iWidth, m_iHeight,
                       GetSysColor( GUI::COL_WINDOW ) );
  }

  if ( m_pFont )
  {
    int   iYOffset = -(int)m_iFirstVisibleItem * m_iItemHeight;

    int   iItem = GetSelectedItem();
    if ( iItem != -1 )
    {
      pViewer->DrawBox( iOffsetX, iOffsetY + iYOffset + iItem * m_iItemHeight,
                        m_ClientRect.size().x - m_pScrollBar->Width(), m_iItemHeight,
                        GetSysColor( GUI::COL_HIGHLIGHT ) );
    }

    if ( ( m_iMouseOverItem != -1 )
    &&   ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) )
    {
      pViewer->DrawBox( iOffsetX, iOffsetY + iYOffset + m_iMouseOverItem * m_iItemHeight,
                        m_ClientRect.size().x - m_pScrollBar->Width(), m_iItemHeight,
                        GetSysColor( GUI::COL_HOTLIGHT ) );
    }

    GR::CDX8ShaderAlphaTest::Apply( *pViewer );

    iItem = 0;
    tAbstractListBoxItemList::iterator    it( m_listItems.begin() );
    while ( it != m_listItems.end() )
    {
      tListBoxItem&   Item = *it;

      if ( iYOffset >= m_iHeight )
      {
        break;
      }
      if ( iYOffset >= 0.0f )
      {
        DWORD   dwColor = GetSysColor( GUI::COL_WINDOWTEXT );
        if ( ( iItem == m_iSelectedItem )
        ||   ( ( iItem == m_iMouseOverItem ) 
        &&     ( !( Style() & LCS_HIDE_HOVER_ITEM ) ) ) )
        {
          dwColor = GetSysColor( GUI::COL_HIGHLIGHTTEXT );
        }
        pViewer->DrawText( (CDX8Font*)m_pFont,
                        iOffsetX,
                        iOffsetY + iYOffset,
                        Item.m_strText.c_str(),
                        dwColor );
      }

      iYOffset += m_iItemHeight;

      ++iItem;
      ++it;
    }
  }

}



ICloneAble* CGUIListBox::Clone()
{

  return new CGUIListBox( *this );

}