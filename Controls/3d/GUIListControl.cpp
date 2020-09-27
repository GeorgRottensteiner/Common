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
#include "GUIListControl.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIListControl::CGUIListControl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwStyles, GR::u32 dwId ) :
  CAbstractListCtrl<CGUIComponent, CGUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, dwStyles, dwId )
{

  ModifyEdge( GUI::GET_RAISED_BORDER );

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIListControl::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::u32   dwColor     = 0xff808080;

  GR::tRect   rc;


  // Header darstellen
  if ( Style() & LCS_SHOW_HEADER )
  {
    for ( size_t i = 0; i < Columns(); ++i )
    {
      GetHeaderRect( i, rc );

      pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), 0xffa0a0a0 );
      
      if ( m_pFont )
      {
        DisplayText( pViewer, iOffsetX, iOffsetY, m_vectColumns[i].m_strDescription.c_str(), m_vectColumns[i].m_TextAlignment, 
                     GetColor( GUI::COL_WINDOWTEXT ),
                     &rc );
      }
    }
  }

  // Selection
  if ( GetLineRect( m_iSelectedItem, rc ) )
  {
    pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), 0xff8080ff );
  }
  if ( GetLineRect( m_iMouseOverItem, rc ) )
  {
    pViewer->DrawBox( iOffsetX + rc.m_iLeft, iOffsetY + rc.m_iTop, rc.width(), rc.height(), 0xffff80ff );
  }


  size_t    iItem = m_iOffset;

  bool      bDone = false;

  do
  {
    for ( size_t iColumn = 0; iColumn < Columns(); ++iColumn )
    {
      if ( !GetItemRect( iItem, iColumn, rc ) )
      {
        bDone = true;
        break;
      }
      if ( m_pFont )
      {
        DisplayText( pViewer, iOffsetX, iOffsetY, 
                     GetItemText( iItem, iColumn ).c_str(),
                     m_vectColumns[iColumn].m_TextAlignment,
                     GetColor( GUI::COL_WINDOWTEXT ),
                     &rc );
      }
    }
    ++iItem;
  }
  while ( !bDone );

}



void CGUIListControl::SetTextureSection( const GUI::eBorderType eType, const tTextureSection& TexSection )
{

  CAbstractListCtrl<CGUIComponent, CGUIScrollBar>::SetTextureSection( eType, TexSection );

  UpdateScrollBar();

}