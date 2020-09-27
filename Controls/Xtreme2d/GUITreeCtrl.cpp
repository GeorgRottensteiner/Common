#include <Debug\debugclient.h>

#include <Xtreme/X2dRenderer.h>

#include "GUIComponentDisplayer.h"
#include "GUITreeCtrl.h"



GUI_IMPLEMENT_CLONEABLE( GUITreeCtrl, "GUITreeCtrl" )



GUITreeCtrl::GUITreeCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwStyles, GR::u32 dwId ) :
  AbstractTreeCtrl<GUIComponent, GUIScrollBar>( iNewX, iNewY, iNewWidth, iNewHeight, dwStyles, dwId )
{
  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  SetSize( iNewWidth, iNewHeight );
}



void GUITreeCtrl::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;


  GR::tRect   rectItem;

  tTree::iterator    it( m_itFirstVisibleItem );
  while ( it != m_TreeItems.end() )
  {
    tTreeItem&   Item = *it;

    if ( GetItemRect( it, rectItem ) )
    {
      GR::tRect   rectExtraIdent;
      if ( GetExtraIndentRect( it, rectExtraIdent ) )
      {
        if ( ( it->CtrlItemData >= 0 )
        &&   ( it->CtrlItemData < m_vectSections.size() ) )
        {
          // Icon
          IconInfo&   II( m_vectSections[it->CtrlItemData] );

          switch ( II.DisplayMethod )
          {
            case X2dRenderer::IMG_PLAIN:
            default:
              Displayer.DrawTextureSection( rectExtraIdent.Left, rectExtraIdent.Top, II.Section );
              break;
            case X2dRenderer::IMG_COLORKEY:
              Displayer.m_pActualRenderer->RenderTextureSectionColorKeyed( rectExtraIdent.Left, rectExtraIdent.Top, II.Section, II.ColorKey );
              break;
          }
        }
      }
      if ( ItemHasChildren( it ) )
      {
        GR::tRect   rectButton;
        if ( GetToggleButtonRect( it, rectButton ) )
        {
          XTextureSection   tsMinus = CustomTextureSection( GUI::CTS_TOGGLE_MINUS );
          XTextureSection   tsPlus = CustomTextureSection( GUI::CTS_TOGGLE_PLUS );

          if ( ( tsMinus.m_pTexture )
          &&   ( tsPlus.m_pTexture ) )
          {
            if ( IsItemCollapsed( it ) )
            {
              Displayer.DrawTextureSection( rectButton.Left, rectButton.Top, tsPlus );
            }
            else
            {
              Displayer.DrawTextureSection( rectButton.Left, rectButton.Top, tsMinus );
            }
          }
          else
          {
            rectButton.inflate( -2, -2 );

            Displayer.DrawRect( rectButton.position(),
                                rectButton.size(),
                                GetColor( GUI::COL_THIN_BORDER ) );

            Displayer.DrawLine( GR::tPoint( rectButton.position().x + 2,
                                            rectButton.position().y + rectButton.height() / 2 ),
                                GR::tPoint( rectButton.position().x + rectButton.width() - 3,
                                            rectButton.position().y + rectButton.height() / 2 ),
                                GetColor( GUI::COL_THIN_BORDER ) );
            if ( IsItemCollapsed( it ) )
            {
              Displayer.DrawLine( GR::tPoint( rectButton.position().x + rectButton.width() / 2,
                                              rectButton.position().y + 2 ),
                                  GR::tPoint( rectButton.position().x + rectButton.width() / 2,
                                              rectButton.position().y + rectButton.height() - 3 ),
                                  GetColor( GUI::COL_THIN_BORDER ) );
            }
          }
        }
      }
      Displayer.DrawText( m_pFont, Item.Text, rectItem, GUI::AF_LEFT | GUI::AF_VCENTER, GetColor( GUI::COL_WINDOWTEXT ) );
      if ( it == m_itSelectedItem )
      {
        Displayer.DrawQuad( rectItem.position().x, 
                          rectItem.position().y,
                          rectItem.width(), 
                          rectItem.height(),
                          0x80ffffff );
      }
      else if ( it == m_itMouseOverItem )
      {
        Displayer.DrawQuad( rectItem.position().x, 
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



int GUITreeCtrl::AddImage( const XTextureSection& Section, X2dRenderer::eImageDisplayMethod DisplayMethod, GR::u32 ColorKey )
{
  m_vectSections.push_back( IconInfo( Section, DisplayMethod, ColorKey ) );

  return (int)( m_vectSections.size() - 1 );
}


void GUITreeCtrl::SetItemImage( TREEITEM hItem, GR::u32 dwImage )
{
  if ( hItem == m_TreeItems.end() )
  {
    return;
  }
  hItem->CtrlItemData = dwImage;
}



TREEITEM GUITreeCtrl::InsertItem( const char* szString, GR::u32 dwItemImage, GR::u32 dwItemData )
{
  TreeIterator  itNew = m_TreeItems.insert( tTreeItem() );

  tTreeItem&    newItem = *itNew;

  newItem.Text         = szString;
  newItem.ItemData      = dwItemData;
  newItem.CtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_TreeItems.end() )
  {
    m_itFirstVisibleItem = m_TreeItems.begin();
  }

  UpdateScrollBar();

  return itNew;
}



TREEITEM GUITreeCtrl::InsertItem( TREEITEM hItemParent, const char* szString, GR::u32 dwItemImage, GR::u32 dwItemData )
{
  if ( hItemParent == TreeIterator() )
  {
    return InsertItem( szString, dwItemData );
  }

  TreeIterator it = m_TreeItems.insert_child( hItemParent, tTreeItem() );

  tTreeItem&   newItem = *it;

  newItem.Text         = szString;
  newItem.ItemData      = dwItemData;
  newItem.CtrlItemData  = dwItemImage;

  if ( m_itFirstVisibleItem == m_TreeItems.end() )
  {
    m_itFirstVisibleItem = m_TreeItems.begin();
  }

  UpdateScrollBar();
  return it;
}



