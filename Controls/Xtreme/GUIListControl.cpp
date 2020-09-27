#include "GUIComponentDisplayer.h"
#include "GUIListControl.h"

GUI_IMPLEMENT_CLONEABLE( GUIListControl, "ListControl" )



GUIListControl::GUIListControl( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Styles, GR::u32 Id ) :
  AbstractListCtrl<GUIComponent, GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, Styles, Id )
{
  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
}



void GUIListControl::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  // Header darstellen
  if ( Style() & LCS_SHOW_HEADER )
  {
    XViewport vpOrig = Displayer.Viewport();
    for ( size_t i = 0; i < Columns(); ++i )
    {
      GetHeaderRect( i, rc );

      Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );
      Displayer.m_pActualRenderer->SetTexture( 0, NULL );
      Displayer.DrawEdge( GUI::VFT_RAISED_BORDER, rc );

      if ( m_pFont )
      {
        rc.inflate( -2, -2 );

        XViewport vpTemp = vpOrig;

        vpTemp.X      += rc.Left;
        vpTemp.Width  = rc.width();
        Displayer.SetViewport( vpTemp );

        Displayer.DrawText( m_pFont, 0, 0, m_vectColumns[i].Description, m_vectColumns[i].TextAlignment, 
                            GetColor( GUI::COL_WINDOWTEXT ),
                            &rc );
        Displayer.SetViewport( vpOrig );
      }
    }
  }

  // Selection
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );
  if ( GetLineRect( m_SelectedItem, rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_HIGHLIGHT ) );
  }
  if ( GetLineRect( m_MouseOverItem, rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_HOTLIGHT ) );
  }


  if ( Columns() )
  {
    size_t    iItem = m_Offset;

    bool      bDone = false;

    XViewport vpOrig = Displayer.Viewport();
    XViewport vpTemp = vpOrig;
    do
    {
      if ( Flags() & GUI::COMPFT_OWNER_DRAW_ITEM )
      {
        if ( ( iItem >= m_Items.NumRows() )
        ||   ( iItem < m_Offset ) )
        {
          break;
        }
        if ( !GetItemRect( iItem, -1, rc ) )
        {
          bDone = true;
          break;
        }

        GUI::OwnerDrawInfo    odInfo;

        odInfo.pComponent = this;
        odInfo.ItemIndex  = (int)iItem;
        odInfo.ItemOffset = rc.position();
        odInfo.ItemSize   = rc.size();
        odInfo.pDisplayer = &Displayer;

        GenerateEvent( OET_OWNER_DRAW_ITEM, (GR::up)&odInfo );
      }
      else
      {
        GR::tRect   rcLine;

        GetLineRect( iItem, rcLine );

        for ( size_t iColumn = 0; iColumn < Columns(); ++iColumn )
        {
          if ( !GetItemRect( iItem, iColumn, rc ) )
          {
            bDone = true;
            break;
          }

          vpTemp.X = vpOrig.X + rc.Left;
          vpTemp.Y = vpOrig.Y + rc.Top;
          vpTemp.Width  = rc.width();
          vpTemp.Height = rc.height();

          if ( rc.Bottom > m_ClientRect.height() )
          {
            vpTemp.Height -= rc.Bottom - ( m_ClientRect.height() );
          }

          Displayer.SetViewport( vpTemp );

          size_t itemData = GetInternalItemData( iItem, iColumn );
          std::map<int,XTextureSection>::iterator   itI( m_ItemImages.find( (int)itemData ) );
          if ( itI != m_ItemImages.end() )
          {
            // an image!
            Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

            Displayer.DrawTextureSection( rc.Left, rc.Top, itI->second );
          }
          else if ( m_pFont )
          {
            if ( GetSelectedItem() == iItem )
            {
              Displayer.DrawText( m_pFont, 0, 0, 
                                  GetItemText( iItem, iColumn ),
                                  m_vectColumns[iColumn].TextAlignment,
                                  GetColor( GUI::COL_HIGHLIGHTTEXT ),
                                  &rc );
            }
            else if ( iItem == m_MouseOverItem )
            {
              Displayer.DrawText( m_pFont, 0, 0, 
                                  GetItemText( iItem, iColumn ),
                                  m_vectColumns[iColumn].TextAlignment,
                                  GetColor( GUI::COL_HOTLIGHTTEXT ),
                                  &rc );
            }
            else
            {
              Displayer.DrawText( m_pFont, 0, 0, 
                                  GetItemText( iItem, iColumn ),
                                  m_vectColumns[iColumn].TextAlignment,
                                  GetColor( GUI::COL_WINDOWTEXT ),
                                  &rc );
            }
          }
        }
      }
      ++iItem;
    }
    while ( !bDone );
    Displayer.SetViewport( vpOrig );
  }
}



void GUIListControl::SetTextureSection( const GUI::eBorderType eType, const XTextureSection& TexSection )
{
  AbstractListCtrl<GUIComponent, GUIScrollBar>::SetTextureSection( eType, TexSection );

  UpdateScrollBar();
}



bool GUIListControl::ProcessEvent( const GUI::ComponentEvent& Event )
{
  switch ( Event.Type )
  {
    case CET_CUSTOM_SECTION_CHANGED:
      if ( Event.Value == GUI::CTS_ARROW_UP )
      {
        UpdateCustomSection( ( GR::u32 )Event.Value, m_pScrollBar->TopLeftButton(), GUI::CTS_BUTTON );
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
  return AbstractListCtrl<GUIComponent, GUIScrollBar>::ProcessEvent( Event );
}



void GUIListControl::UpdateCustomSection( GR::u32 SectionID, GUIComponent* pComponent, GR::u32 CustomSectionID )
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



void GUIListControl::SetItemImage( size_t ItemIndex, size_t SubItemIndex, size_t ImageIndex )
{
  SetInternalItemData( ItemIndex, SubItemIndex, ImageIndex );
}



void GUIListControl::SetImageListImage( size_t ImageIndex, XTextureSection Section )
{
  if ( Section.m_pTexture == NULL )
  {
    m_ItemImages.erase( (int)ImageIndex );
  }
  else
  {
    m_ItemImages[(int)ImageIndex] = Section;
  }
}