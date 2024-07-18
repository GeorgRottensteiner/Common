#include <Misc/Misc.h>
#include <String/Convert.h>

#include "GUIComponentDisplayer.h"
#include "GUIImageList.h"
#include "GUIImage.h"



GUI_IMPLEMENT_CLONEABLE( GUIImageList, "ImageList" )



GUIImageList::GUIImageList( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id, GR::u32 Type ) :
  AbstractListBox<GUIComponent,GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, Type, Id )
{
  m_ItemHeight = 12;
  m_ItemWidth  = m_Width;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  m_pScrollBar->GetComponent( GUIScrollBar::SB_BUTTON_LEFT_UP )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollBar::SB_BUTTON_RIGHT_DOWN )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->GetComponent( GUIScrollBar::SB_SLIDER )->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->SetLocation( m_ClientRect.Width() - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.Height() );
}



GUIImageList::~GUIImageList()
{
  ResetContent();
}



void GUIImageList::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  if  ( m_Items.empty() )
  {
    return;
  }

  size_t     iItemNr = m_FirstVisibleItem;
  tAbstractListBoxItemList::iterator    it( m_Items.begin() );
  std::advance( it, m_FirstVisibleItem );
  GR::tRect   rectItem;

  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

  while ( it != m_Items.end() )
  {
    tListBoxItem&   Item = *it;

    if ( !GetItemRect( iItemNr, rectItem ) )
    {
      break;
    }

    XTextureSection*    pTexSec = (XTextureSection*)GR::Convert::ToUP( Item.Text, 16 );
    if ( pTexSec )
    {
      Displayer.DrawTextureSection( rectItem.Left, rectItem.Top, *pTexSec, 0xffffffff,
                                    rectItem.Width(), rectItem.Height() );
    }

    ++iItemNr;
    ++it;
  }

  if ( GetItemRect( m_SelectedItem, rectItem ) )
  {
    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );

    Displayer.DrawQuad( rectItem.Left,
                        rectItem.Top,
                        rectItem.Width(),
                        rectItem.Height(),
                        0x80ffffff );
  }
  if ( GetItemRect( m_MouseOverItem, rectItem ) )
  {
    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );

    Displayer.DrawQuad( rectItem.Left, rectItem.Top,
                        rectItem.Width(), rectItem.Height(),
                        0x808080ff );
  }
}



void GUIImageList::AddString( const XTextureSection& TexSec, GR::u32 ItemData )
{
  m_Items.push_back( tListBoxItem() );

  tListBoxItem&   newItem = m_Items.back();

  newItem.Text      = CMisc::printf( "%p", new XTextureSection( TexSec ) );
  newItem.ItemData  = ItemData;

  UpdateScrollBar();
}



void GUIImageList::SetItemImage( const size_t ItemIndex, const XTextureSection& TexSec )
{
  if ( ItemIndex >= m_Items.size() )
  {
    return;
  }

  GR::String  item = GetItemText( ItemIndex );

  XTextureSection*    pTexSec = (XTextureSection*)GR::Convert::ToUP( item, 16 );
  delete pTexSec;

  SetItemText( ItemIndex, CMisc::printf( "%d", new XTextureSection( TexSec ) ) );
}




void GUIImageList::DeleteString( GR::u32 ItemIndex )
{
  GR::String  item = GetItemText( ItemIndex );

  XTextureSection*    pTexSec = (XTextureSection*)GR::Convert::ToUP( item, 16 );
  delete pTexSec;

  AbstractListBox<GUIComponent,GUIScrollBar>::DeleteString( ItemIndex );
}



void GUIImageList::ResetContent()
{
  tAbstractListBoxItemList::iterator   it( m_Items.begin() );
  while ( it != m_Items.end() )
  {
    tListBoxItem&   Item( *it );

    XTextureSection*    pTexSec = (XTextureSection*)GR::Convert::ToUP( Item.Text, 16 );
    delete pTexSec;

    ++it;
  }
  
  AbstractListBox<GUIComponent,GUIScrollBar>::ResetContent();
}



bool GUIImageList::ProcessEvent( const GUI::ComponentEvent& Event )
{
  switch ( Event.Type )
  {
    case CET_DRAG_CONTENT_START:
      if ( m_SelectedItem != -1 )
      {
        GUI::DragContentInfo&   dragInfo( *( GUI::DragContentInfo* )Event.Value );

        dragInfo.DragUserData = m_SelectedItem;

        // TODO - query if this item can be dragged!
        GUI::QueryEvent   qEvent( GUI::QueryEventType::QET_CAN_DRAG_CONTENT, this );
        qEvent.ItemIndex = m_SelectedItem;

        QueryEvent( qEvent );
        if ( !qEvent.ResultValue )
        {
          auto dragImage = new GUIImage();
          dragImage->SetImage( *(XTextureSection*)GR::Convert::ToUP( GetItemText( m_SelectedItem ), 16 ) );
          dragInfo.pComponentDragContent = dragImage;
          dragInfo.pComponentDragContent->SetUserData( m_SelectedItem );
          return true;
        }
      }
      return false;
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
  return AbstractListBox<GUIComponent,GUIScrollBar>::ProcessEvent( Event );
}



void GUIImageList::UpdateCustomSection( GR::u32 SectionID, GUIComponent* pComponent, GR::u32 CustomSectionID )
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
    pComponent->SetCustomTextureSection( GUI::CTS_BUTTON, XTextureSection() );
  }
}



