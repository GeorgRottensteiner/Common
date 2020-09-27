/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8/DX8Shader.h>

#include <Misc/Misc.h>

#include "GUIComponentDisplayer.h"
#include "GUIImageList.h"



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIImageList::CGUIImageList( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 lbType ) :
  CAbstractListBox<CGUIComponent,CGUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, lbType, dwId )
{

  m_iItemHeight = 12;
  m_iItemWidth  = m_iWidth;

  ModifyEdge( GUI::GET_SUNKEN_BORDER );

  m_pScrollBar->GetComponent( CGUIScrollBar::SB_BUTTON_LEFT_UP )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollBar::SB_BUTTON_RIGHT_DOWN )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( CGUIScrollBar::SB_SLIDER )->ModifyEdge( GUI::GET_TRANSPARENT_BKGND );
  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIImageList::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  if  ( m_listItems.empty() )
  {
    return;
  }

  size_t     iItemNr = m_iFirstVisibleItem;
  tAbstractListBoxItemList::iterator    it( m_listItems.begin() );
  std::advance( it, m_iFirstVisibleItem );
  GR::tRect   rectItem;

  GR::CDX8ShaderFlat::Apply( *pViewer );

  while ( it != m_listItems.end() )
  {
    tListBoxItem&   Item = *it;

    if ( !GetItemRect( iItemNr, rectItem ) )
    {
      break;
    }

    tTextureSection*    pTexSec = (tTextureSection*)_atoi64( Item.m_strText.c_str() );
    if ( pTexSec )
    {
      pViewer->DrawTextureSection( iOffsetX + rectItem.position().x, iOffsetY + rectItem.position().y, *pTexSec, 0xffffffff,
                                   rectItem.width(), rectItem.height() );
    }

    ++iItemNr;
    ++it;
  }

  if ( GetItemRect( m_iSelectedItem, rectItem ) )
  {
    GR::CDX8ShaderAlphaBlend::Apply( *pViewer );

    pViewer->DrawRect( iOffsetX + rectItem.position().x, 
                       iOffsetY + rectItem.position().y,
                       rectItem.size().x, rectItem.size().y,
                       0xffffffff );
  }
  if ( GetItemRect( m_iMouseOverItem, rectItem ) )
  {
    GR::CDX8ShaderAlphaBlend::Apply( *pViewer );

    pViewer->DrawBox( iOffsetX + rectItem.position().x, 
                      iOffsetY + rectItem.position().y,
                      rectItem.size().x, rectItem.size().y, 0x808080ff );
  }



}



/*-AddString------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIImageList::AddString( const tTextureSection& TexSec, GR::u32 dwItemData )
{

  m_listItems.push_back( tListBoxItem() );

  tListBoxItem&   newItem = m_listItems.back();

  newItem.m_strText     = CMisc::printf( "%d", &TexSec );
  newItem.m_dwItemData  = dwItemData;

  UpdateScrollBar();

}



void CGUIImageList::SetItemImage( const size_t iIndex, const tTextureSection& TexSec )
{

  if ( iIndex >= m_listItems.size() )
  {
    return;
  }
  SetItemText( iIndex, CMisc::printf( "%d", &TexSec ) );

}




ICloneAble* CGUIImageList::Clone()
{

  return new CGUIImageList( *this );

}