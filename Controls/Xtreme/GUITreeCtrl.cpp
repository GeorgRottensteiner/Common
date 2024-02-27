#include <debug/debugclient.h>

#include <Xtreme/XRenderer.h>

#include "GUIComponentDisplayer.h"
#include "GUITreeCtrl.h"



GUI_IMPLEMENT_CLONEABLE( GUITreeCtrl, "TreeControl" )



GUITreeCtrl::GUITreeCtrl( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Styles, GR::u32 Id ) :
  AbstractTreeCtrl<GUIComponent, GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, Styles, Id )
{
  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  SetSize( NewWidth, NewHeight );
}



void GUITreeCtrl::UpdateCustomSection( GR::u32 SectionID, GUIComponent* pComponent, GR::u32 CustomSectionID )
{
  XTextureSection   section( CustomTextureSection( SectionID ) );
  if ( section.m_pTexture )
  {
    pComponent->SetCustomTextureSection( CustomSectionID, section );
    pComponent->ModifyStyle( GUIButton::BCS_NO_BORDER );
    pComponent->ModifyVisualStyle( 0, GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
  }
  else
  {
    pComponent->SetCustomTextureSection( SectionID, XTextureSection() );
  }
}



bool GUITreeCtrl::ProcessEvent( const GUI::ComponentEvent& Event )
{
  switch ( Event.Type )
  {
    case CET_CUSTOM_SECTION_CHANGED:
      if ( Event.Value == GUI::CTS_ARROW_UP )
      {
        UpdateCustomSection( (GR::u32)Event.Value, m_pScrollBar->TopLeftButton(), GUI::CTS_BUTTON );
      }
      else if ( Event.Value == GUI::CTS_ARROW_UP_MOUSEOVER )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->TopLeftButton(), GUI::CTS_BUTTON_MOUSEOVER );
      }
      else if ( Event.Value == GUI::CTS_ARROW_UP_PUSHED )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->TopLeftButton(), GUI::CTS_BUTTON_PUSHED );
      }
      else if ( Event.Value == GUI::CTS_ARROW_UP_DISABLED )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->TopLeftButton(), GUI::CTS_BUTTON_DISABLED );
      }
      else if ( Event.Value == GUI::CTS_ARROW_DOWN )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->BottomRightButton(), GUI::CTS_BUTTON );
      }
      else if ( Event.Value == GUI::CTS_ARROW_DOWN_MOUSEOVER )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->BottomRightButton(), GUI::CTS_BUTTON_MOUSEOVER );
      }
      else if ( Event.Value == GUI::CTS_ARROW_DOWN_PUSHED )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->BottomRightButton(), GUI::CTS_BUTTON_PUSHED );
      }
      else if ( Event.Value == GUI::CTS_ARROW_DOWN_DISABLED )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->BottomRightButton(), GUI::CTS_BUTTON_DISABLED );
      }
      break;
  }
  return AbstractTreeCtrl<GUIComponent, GUIScrollBar>::ProcessEvent( Event );
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
          Displayer.DrawTextureSection( rectExtraIdent.Left, rectExtraIdent.Top,
                                        m_vectSections[it->CtrlItemData],
                                        0xffffffff,
                                        m_ItemHeight,
                                        m_ItemHeight );
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
              Displayer.DrawTextureSection( rectButton.Left,
                                            rectButton.Top,
                                            tsPlus,
                                            0xffffffff,
                                            m_ItemHeight, 
                                            m_ItemHeight );
            }
            else
            {
              Displayer.DrawTextureSection( rectButton.Left,
                                            rectButton.Top,
                                            tsMinus,
                                            0xffffffff,
                                            m_ItemHeight,
                                            m_ItemHeight );
            }
          }
          else
          {
            rectButton.Inflate( -2, -2 );

            Displayer.DrawRect( GR::tPoint( rectButton.Left, rectButton.Top ), 
                                GR::tPoint( rectButton.Width(), rectButton.Height() ),
                                GetSysColor( GUI::COL_WINDOWTEXT ) );
            Displayer.DrawLine( GR::tPoint( rectButton.Left + 2, 
                                            rectButton.Top + rectButton.Height() / 2 ),
                                GR::tPoint( rectButton.Left + rectButton.Width() - 2,
                                            rectButton.Top + rectButton.Height() / 2 ),
                                GetSysColor( GUI::COL_WINDOWTEXT ) );
            if ( IsItemCollapsed( it ) )
            {
              Displayer.DrawLine( GR::tPoint( rectButton.Left + rectButton.Width() / 2,
                                              rectButton.Top + 2 ),
                                  GR::tPoint( rectButton.Left + rectButton.Width() / 2,
                                              rectButton.Top + rectButton.Height() - 2 ),
                                  GetSysColor( GUI::COL_WINDOWTEXT ) );
            }
          }
        }
      }
      Displayer.DrawText( m_pFont, 
                          0, 0, Item.Text, 
                          GUI::AF_LEFT | GUI::AF_VCENTER, 
                          GetColor( GUI::COL_WINDOWTEXT ),
                          &rectItem );

      if ( it == m_itSelectedItem )
      {
        Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );

        Displayer.DrawQuad( rectItem.Left, 
                            rectItem.Top,
                            rectItem.Width(), 
                            rectItem.Height(),
                            0x80ffffff );
      }
      else if ( it == m_itMouseOverItem )
      {
        Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );

        Displayer.DrawQuad( rectItem.Left, 
                            rectItem.Top,
                            rectItem.Width(), 
                            rectItem.Height(),
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



int GUITreeCtrl::AddImage( const XTextureSection& Section )
{
  m_vectSections.push_back( Section );

  return (int)( m_vectSections.size() - 1 );
}



void GUITreeCtrl::SetItemImage( TREEITEM hItem, GR::u32 Image )
{
  if ( hItem == m_TreeItems.end() )
  {
    return;
  }
  hItem->CtrlItemData = Image;
}



GR::u32 GUITreeCtrl::ItemImage( TREEITEM hItem )
{
  if ( hItem == m_TreeItems.end() )
  {
    return -1;
  }
  return (GR::u32)hItem->CtrlItemData;
}



TREEITEM GUITreeCtrl::InsertItem( const GR::String& String, GR::u32 ItemImage, GR::up ItemData )
{
  TreeIterator  itNew = m_TreeItems.insert( tTreeItem() );

  tTreeItem&    newItem = *itNew;

  newItem.Text          = String;
  newItem.ItemData      = ItemData;
  newItem.CtrlItemData  = ItemImage;

  if ( m_itFirstVisibleItem == m_TreeItems.end() )
  {
    m_itFirstVisibleItem = m_TreeItems.begin();
  }

  UpdateScrollBar();

  return itNew;
}



TREEITEM GUITreeCtrl::InsertItem( TREEITEM hItemParent, const GR::String& String, GR::u32 ItemImage, GR::up ItemData )
{

  if ( hItemParent == TreeIterator() )
  {
    return InsertItem( String, ItemImage, ItemData );
  }

  TreeIterator it = m_TreeItems.insert_child( hItemParent, tTreeItem() );

  tTreeItem&   newItem = *it;

  newItem.Text          = String;
  newItem.ItemData      = ItemData;
  newItem.CtrlItemData  = ItemImage;

  if ( m_itFirstVisibleItem == m_TreeItems.end() )
  {
    m_itFirstVisibleItem = m_TreeItems.begin();
  }

  UpdateScrollBar();
  return it;
}



