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
#include "GUITreeCtrl.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUITreeCtrl::CGUITreeCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwStyles, GR::u32 dwId ) :
  CAbstractTreeCtrl<CGUIComponent, CGUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, dwStyles, dwId )
{

  ModifyEdge( GUI::GET_SUNKEN_BORDER );

  SetSize( iNewWidth, iNewHeight );

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUITreeCtrl::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  GR::u32   dwColor     = 0xff808080;

  GR::tRect   rc;


  GR::tRect   rectItem;
  
  tTree::iterator    it( m_itFirstVisibleItem );
  while ( it != m_treeItems.end() )
  {
    tTreeItem&   Item = *it;

    if ( GetItemRect( it, rectItem ) )
    {
      rectItem.offset( iOffsetX, iOffsetY );
      GR::tRect   rectExtraIdent;
      if ( GetExtraIdentRect( it, rectExtraIdent ) )
      {
        rectExtraIdent.offset( iOffsetX, iOffsetY );
        if ( ( it->m_dwCtrlItemData >= 0 )
        &&   ( it->m_dwCtrlItemData < m_vectSections.size() ) )
        {
          // Icon
          pViewer->DrawTextureSection( rectExtraIdent.m_iLeft, rectExtraIdent.m_iTop,
                                       m_vectSections[it->m_dwCtrlItemData] );
        }
      }
      if ( ItemHasChildren( it ) )
      {
        GR::tRect   rectButton;
        if ( GetToggleButtonRect( it, rectButton ) )
        {
          rectButton.inflate( -2, -2 );

          rectButton.offset( iOffsetX, iOffsetY );

          pViewer->DrawRect( rectButton.m_iLeft, rectButton.m_iTop, rectButton.width(), rectButton.height(), GetSysColor( GUI::COL_WINDOWTEXT ) );

          pViewer->DrawLine( rectButton.position().x + 2, 
                             rectButton.position().y + rectButton.height() / 2,
                             rectButton.position().x + rectButton.width() - 3,
                             rectButton.position().y + rectButton.height() / 2,
                             GetSysColor( GUI::COL_WINDOWTEXT ) );
          if ( IsItemCollapsed( it ) )
          {
            pViewer->DrawLine( rectButton.position().x + rectButton.width() / 2, 
                               rectButton.position().y + 2,
                               rectButton.position().x + rectButton.width() / 2,
                               rectButton.position().y + rectButton.height() - 3,
                               GetSysColor( GUI::COL_WINDOWTEXT ) );
          }
        }
      }
      DisplayText( pViewer, 0, 0, Item.m_strText.c_str(), GUI::AF_LEFT | GUI::AF_VCENTER, 
                   GetColor( GUI::COL_WINDOWTEXT ),
                   &rectItem );

      if ( it == m_itSelectedItem )
      {
        GR::CDX8ShaderAlphaBlend::Apply( *pViewer );
        pViewer->DrawBox( rectItem.position().x, 
                          rectItem.position().y,
                          rectItem.width(), 
                          rectItem.height(),
                          0x80ffffff );
      }
      else if ( it == m_itMouseOverItem )
      {
        GR::CDX8ShaderAlphaBlend::Apply( *pViewer );
        pViewer->DrawBox( rectItem.position().x, 
                          rectItem.position().y,
                          rectItem.width(), 
                          rectItem.height(),
                          0x803030a0 );
      }

    }
    else
    {
      break;
    }

    it = GetNextVisibleItem( it );
  }

}



ICloneAble* CGUITreeCtrl::Clone()
{

  return new CGUITreeCtrl( *this );

}



int CGUITreeCtrl::AddImage( const tTextureSection& Section )
{

  m_vectSections.push_back( Section );

  return m_vectSections.size() - 1;

}


/*-SetItemImage---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUITreeCtrl::SetItemImage( TREEITEM hItem, GR::u32 dwImage )
{

  if ( hItem == m_treeItems.end() )
  {
    return;
  }
  hItem->m_dwCtrlItemData = dwImage;

}



/*-InsertItem-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

TREEITEM CGUITreeCtrl::InsertItem( const char* szString, GR::u32 dwItemImage, GR::u32 dwItemData )
{

  TreeIterator  itNew = m_treeItems.insert( tTreeItem() );

  tTreeItem&    newItem = *itNew;

  newItem.m_strText         = szString;
  newItem.m_dwItemData      = dwItemData;
  newItem.m_dwCtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_treeItems.end() )
  {
    m_itFirstVisibleItem = m_treeItems.begin();
  }

  UpdateScrollBar();

  return itNew;

}



/*-InsertItem-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

TREEITEM CGUITreeCtrl::InsertItem( TREEITEM hItemParent, const char* szString, GR::u32 dwItemImage, GR::u32 dwItemData )
{

  if ( hItemParent == TreeIterator() )
  {
    return InsertItem( szString, dwItemData );
  }

  TreeIterator it = m_treeItems.insert_child( hItemParent, tTreeItem() );

  tTreeItem&   newItem = *it;

  newItem.m_strText         = szString;
  newItem.m_dwItemData      = dwItemData;
  newItem.m_dwCtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_treeItems.end() )
  {
    m_itFirstVisibleItem = m_treeItems.begin();
  }

  UpdateScrollBar();
  return it;
}



