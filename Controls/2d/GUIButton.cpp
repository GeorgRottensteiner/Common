#include <Misc/Misc.h>

#include <Grafik/GfxHelper.h>
#include <Grafik/ContextDescriptor.h>

#include <Controls/Component.h>
#include "GUIButton.h"
#include "GUIComponentDisplayer.h"



GUI_IMPLEMENT_CLONEABLE( GUIButton, "Button" )



GUIButton::GUIButton( GR::u32 ID ) 
  : AbstractButton<GUIComponent>( ID ),
    m_ptPushedTextOffset( 1, 1 )
{
  m_pImage            = NULL;
  m_pImageMouseOver   = NULL;
  m_pImagePushed      = NULL;
  m_pImageDisabled    = NULL;
}



GUIButton::GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 ID ) 
  : AbstractButton<GUIComponent>( NewX, NewY, NewWidth, NewHeight, ID ),
    m_ptPushedTextOffset( 1, 1 )
{
  m_pImage            = NULL;
  m_pImageMouseOver   = NULL;
  m_pImagePushed      = NULL;
  m_pImageDisabled    = NULL;

  RecalcClientRect();
}



GUIButton::GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, const char* pCaption, GR::u32 ID )
  : AbstractButton<GUIComponent>( NewX, NewY, NewWidth, NewHeight, pCaption, ID ),
    m_ptPushedTextOffset( 1, 1 )
{
  m_pImage            = NULL;
  m_pImageMouseOver   = NULL;
  m_pImagePushed      = NULL;
  m_pImageDisabled    = NULL;

  RecalcClientRect();
}



GUIButton::GUIButton( int NewX, int NewY, GR::Graphic::Image* pImage, GR::Graphic::Image* pImageMO, GR::Graphic::Image* pImageP, GR::u32 Id ) :
  AbstractButton<GUIComponent>( NewX, NewY, 100, 30, Id ),
  m_ptPushedTextOffset( 1, 1 )
{
  m_pImage            = pImage;
  m_pImageMouseOver   = pImageMO;
  m_pImagePushed      = pImageP;
  m_pImageDisabled    = NULL;

  if ( m_pImage )
  {
    SetSize( m_pImage->GetWidth(), m_pImage->GetHeight() );
  }

  RecalcClientRect();
}



void GUIButton::DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage )
{
  if ( !( VisualStyle() & GUI::VFT_TRANSPARENT_BKGND ) )
  {
    if ( !IsEnabled() )
    {
      if ( m_pImageDisabled )
      {
        m_pImageDisabled->PutImage( pPage, 0, 0 );
      }
      else
      {
        pPage->Box( 0, 0, Width() - 1, Height() - 1, pPage->GetRGB256( GetColor( GUI::COL_BTNFACE ) ) );
      }
    }
    else if ( m_pImageBackground )
    {
      for ( int X = 0; X < Width() / m_pImageBackground->GetWidth() + 1; ++X )
      {
        for ( int Y = 0; Y < Height() / m_pImageBackground->GetHeight() + 1; ++Y )
        {
          m_pImageBackground->PutImage( pPage, X * m_pImageBackground->GetWidth(), Y * m_pImageBackground->GetHeight() );
        }
      }
    }
    else
    {
      pPage->Box( 0, 0, Width() - 1, Height() - 1, pPage->GetRGB256( GetColor( GUI::COL_BTNFACE ) ) );
    }
  }
  

  GR::tRect   rectComp;
  GetComponentRect( rectComp );
  DrawEdge( pPage, m_VisualStyle, rectComp );
}



void GUIButton::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  if ( !IsVisible() )
  {
    return;
  }

  GR::Graphic::ContextDescriptor    cdPage( pPage );

  GR::tRect   rcClient;

  GetClientRect( rcClient );

  if ( !IsEnabled() )
  {
    if ( m_pImageDisabled )
    {
      m_pImageDisabled->PutImage( pPage, 0, 0 );
    }
  }
  else if ( IsPushed() )
  {
    if ( m_pImagePushed )
    {
      m_pImagePushed->PutImage( pPage, 0, 0 );
    }
    else if ( !( Style() & BCS_NO_BORDER ) )
    {
      DrawEdge( pPage, GUI::VFT_SUNKEN_BORDER, rcClient );
    }
  }
  else if ( GUI::Component::IsMouseInside() )
  {
    if ( m_pImageMouseOver )
    {
      m_pImageMouseOver->PutImage( pPage, 0, 0 );
    }
    else if ( !( Style() & BCS_NO_BORDER ) )
    {
      DrawEdge( pPage, GUI::VFT_RAISED_BORDER, rcClient );
    }
  }
  else
  {
    if ( m_pImage )
    {
      m_pImage->PutImage( pPage, 0, 0 );
    }
    else if ( !( Style() & BCS_NO_BORDER ) )
    {
      DrawEdge( pPage, GUI::VFT_RAISED_BORDER, rcClient );
    }
  }

  rcClient.offset( m_ptTextDisplayOffset );
  if ( IsPushed() )
  {
    rcClient.offset( m_ptPushedTextOffset );
  }
  DrawText( pPage, m_Caption.c_str(), rcClient, TextAlignment() );

  rcClient.offset( -m_ptTextDisplayOffset );
  if ( IsFocused() )
  {
    rcClient.inflate( -4, -4 );
    DrawFocusRect( pPage, rcClient );
  }
}



void GUIButton::SetImages( GR::Graphic::Image* pImage, GR::Graphic::Image* pImageMO, GR::Graphic::Image* pImageP, GR::Graphic::Image* pImageDisabled )
{
  m_pImage          = pImage;
  m_pImageMouseOver = pImageMO;
  m_pImagePushed    = pImageP;
  m_pImageDisabled  = pImageDisabled;

  if ( m_pImage )
  {
    ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
    SetSize( m_pImage->GetWidth(), m_pImage->GetHeight() );
  }
  RecalcClientRect();
}



bool GUIButton::IsMouseInside( const GR::tPoint& ptMousePos )
{
  if ( m_pImage == NULL )
  {
    return GUI::Component::IsMouseInside( ptMousePos );
  }

  if ( ( ptMousePos.x < 0 )
  ||   ( ptMousePos.y < 0 )
  ||   ( ptMousePos.x >= m_pImage->GetWidth() )
  ||   ( ptMousePos.y >= m_pImage->GetHeight() ) )
  {
    return false;
  }
  if ( m_pImage->GetPixel( ptMousePos.x, ptMousePos.y ) != m_pImage->GetTransparentColor() )
  {
    return true;
  }
  return false;
}



bool GUIButton::IsMouseInsideNonClientArea( const GR::tPoint& ptMousePos )
{
  if ( ( ptMousePos.x >= 0 )
  &&   ( ptMousePos.y >= 0 )
  &&   ( ptMousePos.x < m_Width )
  &&   ( ptMousePos.y < m_Height ) )
  {
    GR::tPoint    PT = ptMousePos - GetClientOffset();

    return IsMouseInside( PT );
  }
  return false;
}



void GUIButton::SetPushedTextOffset( const GR::tPoint& ptOffset )
{
  m_ptPushedTextOffset = ptOffset;
}



void GUIButton::SetDisplayTextOffset( const GR::tPoint& ptOffset )
{
  m_ptTextDisplayOffset = ptOffset;
}