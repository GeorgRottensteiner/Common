#include <Grafik/ContextDescriptor.h>

#include <String/StringUtil.h>

#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"


GUI_IMPLEMENT_CLONEABLE( GUIComponent, "Component" )



GUIComponent::GUIComponent( GR::u32 dwId ) :
  GUI::Component( dwId ),
  m_pImageBackground( NULL )
{
  m_ClassName      = "GUI.Component";
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



GUIComponent::GUIComponent( int iX, int iY, int iWidth, int iHeight, GR::u32 dwId ) :
  GUI::Component( iX, iY, iWidth, iHeight, dwId ),
  m_pImageBackground( NULL )
{
  m_ClassName      = "GUI.Component";
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



GUIComponent::GUIComponent( int iX, int iY, int iWidth, int iHeight, const GR::String& strCaption, GR::u32 dwId ) :
  GUI::Component( iX, iY, iWidth, iHeight, strCaption, dwId ),
  m_pImageBackground( NULL )
{
  m_ClassName      = "GUI.Component";
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



void GUIComponent::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
}



void GUIComponent::DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage )
{
  if ( !( VisualStyle() & GUI::VFT_TRANSPARENT_BKGND ) )
  {
    if ( m_pImageBackground )
    {
      for ( int iX = 0; iX < Width() / m_pImageBackground->GetWidth() + 1; ++iX )
      {
        for ( int iY = 0; iY < Height() / m_pImageBackground->GetHeight() + 1; ++iY )
        {
          m_pImageBackground->PutImage( pPage, iX * m_pImageBackground->GetWidth(), iY * m_pImageBackground->GetHeight() );
        }
      }
    }
    else
    {
      pPage->Box( 0, 0, Width() - 1, Height() - 1, pPage->GetRGB256( GetColor( GUI::COL_WINDOW ) ) );
    }
  }

  GR::tRect   rectComp;
  GetComponentRect( rectComp );
  DrawEdge( pPage, m_VisualStyle, rectComp );
}



void GUIComponent::DrawEdge( GR::Graphic::GFXPage* pPage, GR::u32 edgeType, GR::tRect& rectEdge )
{
  int   iX1 = rectEdge.Left,
        iY1 = rectEdge.Top,
        iX2 = iX1 + rectEdge.Width() - 1,
        iY2 = iY1 + rectEdge.Height() - 1;

  GR::Graphic::ContextDescriptor    cdPage( pPage );


  if ( ( m_TextureSection[GUI::BT_EDGE_TOP].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_LEFT].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_RIGHT].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_BOTTOM].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].BitsProPixel() )
  ||   ( m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].BitsProPixel() ) )
  {
    if ( ( edgeType & GUI::VFT_RAISED_BORDER )
    ||   ( edgeType & GUI::VFT_FLAT_BORDER ) )
    {
      if ( m_TextureSection[GUI::BT_EDGE_TOP].Width() )
      {
        for ( int i = 0; i < ( iX2 - iX1 - m_TextureSection[GUI::BT_EDGE_TOP_LEFT].Width() - m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].Width() ) / m_TextureSection[GUI::BT_EDGE_TOP].Width() + 1; ++i )
        {
          m_TextureSection[GUI::BT_EDGE_TOP].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_EDGE_TOP].Width(),
                            m_TextureSection[GUI::BT_EDGE_TOP].Height(),
                            iX1 + m_TextureSection[GUI::BT_EDGE_TOP_LEFT].Width() + i * m_TextureSection[GUI::BT_EDGE_TOP].Width(), iY1, &cdPage );
        }
      }
      if ( m_TextureSection[GUI::BT_EDGE_BOTTOM].Width() )
      {
        for ( int i = 0; i < ( iX2 - iX1 - m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Width() - m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].Width() ) / m_TextureSection[GUI::BT_EDGE_BOTTOM].Width() + 1; ++i )
        {
          m_TextureSection[GUI::BT_EDGE_BOTTOM].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_EDGE_BOTTOM].Width(),
                            m_TextureSection[GUI::BT_EDGE_BOTTOM].Height(),
                            iX1 + m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Width() + i * m_TextureSection[GUI::BT_EDGE_BOTTOM].Width(),
                            iY2 - m_TextureSection[GUI::BT_EDGE_BOTTOM].Height() + 1, &cdPage );
        }
      }
      if ( m_TextureSection[GUI::BT_EDGE_LEFT].Height() )
      {
        for ( int i = 0; i < ( iY2 - iY1 - m_TextureSection[GUI::BT_EDGE_TOP_LEFT].Height() - m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Height() ) / m_TextureSection[GUI::BT_EDGE_LEFT].Height() + 1; ++i )
        {
          m_TextureSection[GUI::BT_EDGE_LEFT].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_EDGE_LEFT].Width(),
                            m_TextureSection[GUI::BT_EDGE_LEFT].Height(),
                            iX1,
                            iY1 + m_TextureSection[GUI::BT_EDGE_TOP_LEFT].Height() + i * m_TextureSection[GUI::BT_EDGE_LEFT].Height(),
                            &cdPage );
        }
      }
      if ( m_TextureSection[GUI::BT_EDGE_RIGHT].Height() )
      {
        for ( int i = 0; i < ( iY2 - iY1 - m_TextureSection[GUI::BT_EDGE_RIGHT].Height() - m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].Height() ) / m_TextureSection[GUI::BT_EDGE_RIGHT].Height() + 1; ++i )
        {
          m_TextureSection[GUI::BT_EDGE_RIGHT].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_EDGE_RIGHT].Width(),
                            m_TextureSection[GUI::BT_EDGE_RIGHT].Height(),
                            iX2 - m_TextureSection[GUI::BT_EDGE_RIGHT].Width() + 1,
                            iY1 + m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].Height() + i * m_TextureSection[GUI::BT_EDGE_RIGHT].Height(),
                            &cdPage );
        }
      }

      // Ecken
      if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_EDGE_TOP_LEFT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_EDGE_TOP_LEFT].Width(),
                          m_TextureSection[GUI::BT_EDGE_TOP_LEFT].Height(),
                          iX1, iY1, &cdPage );
      }
      if ( m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].Width(),
                          m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].Height(),
                          iX2 - m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].Width() + 1, iY1, &cdPage );
      }
      if ( m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Width(),
                          m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Height(),
                          iX1, iY2 - m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Height() + 1, &cdPage );
      }
      if ( m_TextureSection[GUI::BT_EDGE_TOP_RIGHT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].Width(),
                          m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].Height(),
                          iX2 - m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].Width() + 1,
                          iY2 - m_TextureSection[GUI::BT_EDGE_BOTTOM_LEFT].Height() + 1,
                          &cdPage );
      }
      return;
    }
    else if ( edgeType & GUI::VFT_SUNKEN_BORDER )
    {
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].Width() )
      {
        for ( int i = 0; i < ( iX2 - iX1 - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].Width() - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].Width() ) / m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].Width() + 1; ++i )
        {
          m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].Width(),
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].Height(),
                            iX1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].Width() + i * m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP].Width(), iY1, &cdPage );
        }
      }
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].Width() )
      {
        for ( int i = 0; i < ( iX2 - iX1 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Width() - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].Width() ) / m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].Width() + 1; ++i )
        {
          m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].Width(),
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].Height(),
                            iX1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Width() + i * m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].Width(),
                            iY2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM].Height() + 1, &cdPage );
        }
      }
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT].Height() )
      {
        for ( int i = 0; i < ( iY2 - iY1 - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].Height() - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Height() ) / m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT].Height() + 1; ++i )
        {
          m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT].Width(),
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT].Height(),
                            iX1,
                            iY1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].Height() + i * m_TextureSection[GUI::BT_SUNKEN_EDGE_LEFT].Height(),
                            &cdPage );
        }
      }
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Height() )
      {
        for ( int i = 0; i < ( iY2 - iY1 - m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Height() - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].Height() ) / m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Height() + 1; ++i )
        {
          m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].CopyArea( 0, 0,
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Width(),
                            m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Height(),
                            iX2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Width() + 1,
                            iY1 + m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].Height() + i * m_TextureSection[GUI::BT_SUNKEN_EDGE_RIGHT].Height(),
                            &cdPage );
        }
      }

      // Ecken
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].Width(),
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].Height(),
                          iX1, iY1, &cdPage );
      }
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].Width(),
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].Height(),
                          iX2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].Width() + 1, iY1, &cdPage );
      }
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Width(),
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Height(),
                          iX1, iY2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Height() + 1, &cdPage );
      }
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].BitsProPixel() )
      {
        m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].CopyArea( 0, 0,
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].Width(),
                          m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].Height(),
                          iX2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].Width() + 1,
                          iY2 - m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].Height() + 1,
                          &cdPage );
      }
      return;
    }
  }


  GR::u32       dwColorLight    = GUIComponentDisplayer::Instance().GetSysColor( GUI::COL_3DLIGHT ),
                dwColorHiLight  = GUIComponentDisplayer::Instance().GetSysColor( GUI::COL_BTNHIGHLIGHT ),
                dwColorDark     = GUIComponentDisplayer::Instance().GetSysColor( GUI::COL_BTNSHADOW ),
                dwColorVeryDark = GUIComponentDisplayer::Instance().GetSysColor( GUI::COL_3DDKSHADOW );


  if ( ( edgeType & GUI::VFT_RAISED_BORDER )
  &&   ( edgeType & GUI::VFT_FLAT_BORDER ) )
  {
    cdPage.Line( iX1, iY1, iX2 - 1, iY1, dwColorLight );
    cdPage.Line( iX1, iY1, iX1, iY2 - 1, dwColorLight );

    cdPage.Line( iX2, iY1 + 1, iX2, iY2, dwColorDark );
    cdPage.Line( iX1 + 1, iY2, iX2, iY2, dwColorDark );
  }
  else if ( edgeType & GUI::VFT_RAISED_BORDER )
  {
    cdPage.Line( iX1 + 1, iY1 + 1, iX2 - 2, iY1 + 1, dwColorLight );
    cdPage.Line( iX1 + 1, iY1 + 1, iX1 + 1, iY2 - 2, dwColorLight );

    cdPage.Line( iX1, iY1, iX2 - 1, iY1, dwColorHiLight );
    cdPage.Line( iX1, iY1, iX1, iY2 - 1, dwColorHiLight );

    cdPage.Line( iX2, iY1 + 1, iX2, iY2, dwColorVeryDark );
    cdPage.Line( iX1 + 1, iY2, iX2, iY2, dwColorVeryDark );

    cdPage.Line( iX2 - 1, iY1 + 2, iX2 - 1, iY2 - 1, dwColorDark );
    cdPage.Line( iX1 + 2, iY2 - 1, iX2 - 1, iY2 - 1, dwColorDark );
  }
  else if ( edgeType & GUI::VFT_SUNKEN_BORDER )
  {
    cdPage.Line( iX1, iY1, iX2 - 1, iY1, dwColorDark );
    cdPage.Line( iX1, iY1, iX1, iY2 - 1, dwColorDark );

    cdPage.Line( iX1 + 1, iY1 + 1, iX2 - 2, iY1 + 1, dwColorVeryDark );
    cdPage.Line( iX1 + 1, iY1 + 1, iX1 + 1, iY2 - 2, dwColorVeryDark );

    cdPage.Line( iX2, iY1 + 1, iX2, iY2, dwColorLight );
    cdPage.Line( iX1 + 1, iY2, iX2, iY2, dwColorLight );

    cdPage.Line( iX2 - 1, iY1 + 2, iX2 - 1, iY2 - 1, dwColorHiLight );
    cdPage.Line( iX1 + 2, iY2 - 1, iX2 - 1, iY2 - 1, dwColorHiLight );
  }
}



void GUIComponent::DrawText( GR::Graphic::GFXPage* pPage, const char* szText,
                              GR::tRect& rectText, GR::u32 tatType )
{
  if ( ( m_pFont == NULL )
  ||   ( pPage == NULL ) )
  {
    return;
  }

  if ( tatType & GUI::AF_MULTILINE )
  {
    std::vector<GR::String>    vectText;

    GR::tRect     origRect( rectText );

    GR::Strings::WrapText( m_pFont, m_Caption, origRect, vectText );

    int   iY = rectText.Top;

    if ( ( tatType & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
    {
      iY += ( rectText.Height() - 20 * (int)vectText.size() ) / 2;
    }
    else if ( tatType & GUI::AF_BOTTOM )
    {
      iY = rectText.Bottom - 20 * (int)vectText.size();
    }

    std::vector<GR::String>::iterator    it( vectText.begin() );
    while ( it != vectText.end() )
    {
      GR::String&    strText = *it;

      int     iX = rectText.Left;

      int     iLength = m_pFont->TextLength( strText.c_str() );

      if ( tatType & GUI::AF_CENTER )
      {
        iX += ( rectText.Width() - iLength ) / 2;
      }
      else if ( tatType & GUI::AF_RIGHT )
      {
        iX = rectText.Right - iLength;
      }

      ( (GR::Font*)m_pFont )->PrintFont( pPage, iX, iY, strText.c_str() );
      ++it;
      iY += 20;
    }
    return;
  }

  int   iTextWidth  = m_pFont->TextLength( szText );
  int   iTextHeight = m_pFont->TextHeight();

  int   iX = 0,
        iY = 0;

  if ( ( tatType & GUI::AF_CENTER ) == GUI::AF_CENTER )
  {
    iX = ( rectText.Width() - iTextWidth ) / 2;
  }
  else if ( tatType & GUI::AF_RIGHT )
  {
    iX = rectText.Width() - iTextWidth;
  }

  if ( ( tatType & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
  {
    iY = ( rectText.Height() - iTextHeight ) / 2;
  }
  else if ( tatType & GUI::AF_BOTTOM )
  {
    iY = rectText.Height() - iTextHeight;
  }

  ( (GR::Font*)m_pFont )->PrintFont( pPage,
                    rectText.Left + iX, rectText.Top + iY,
                    szText,
                    IMAGE_METHOD_OPTIMAL );
}



void GUIComponent::Display( GUI::ComponentDisplayerBase* pDisplayer )
{
  GUIComponentDisplayer*   pCompDisplayer = (GUIComponentDisplayer*)pDisplayer;
  DisplayOnPage( pCompDisplayer->m_pPage );
}



void GUIComponent::DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer )
{
  GUIComponentDisplayer*   pCompDisplayer = (GUIComponentDisplayer*)pDisplayer;
  DisplayNonClientOnPage( pCompDisplayer->m_pPage );
}



const GR::u32 GUIComponent::GetSysColor( GUI::eColorIndex colIndex ) const
{
  return GUIComponentDisplayer::Instance().GetSysColor( colIndex );
}



void GUIComponent::SetBackground( GR::Graphic::Image* pImage )
{
  m_pImageBackground = pImage;
}



int GUIComponent::GetBorderWidth( const GUI::eBorderType eType ) const
{
  if ( eType >= GUI::BT_EDGE_LAST_ENTRY )
  {
    return 0;
  }
  if ( m_TextureSection[eType].BitsProPixel() )
  {
    if ( ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
    {
      return m_TextureSection[eType].Width();
    }
  }
  switch ( eType )
  {
    case GUI::BT_EDGE_LEFT:
    case GUI::BT_EDGE_RIGHT:
    case GUI::BT_EDGE_TOP_LEFT:
    case GUI::BT_EDGE_TOP_RIGHT:
    case GUI::BT_EDGE_BOTTOM_LEFT:
    case GUI::BT_EDGE_BOTTOM_RIGHT:
    case GUI::BT_SUNKEN_EDGE_LEFT:
    case GUI::BT_SUNKEN_EDGE_RIGHT:
    case GUI::BT_SUNKEN_EDGE_TOP_LEFT:
    case GUI::BT_SUNKEN_EDGE_TOP_RIGHT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT:
      if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
      {
        return 1;
      }
      if ( ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
      ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
      {
        return 2;
      }
      break;
  }
  return 0;
}



int GUIComponent::GetBorderHeight( const GUI::eBorderType eType ) const
{
  if ( eType >= GUI::BT_EDGE_LAST_ENTRY )
  {
    return 0;
  }
  if ( m_TextureSection[eType].BitsProPixel() )
  {
    if ( ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
    {
      return m_TextureSection[eType].Height();
    }
  }
  switch ( eType )
  {
    case GUI::BT_EDGE_TOP:
    case GUI::BT_EDGE_BOTTOM:
    case GUI::BT_EDGE_TOP_LEFT:
    case GUI::BT_EDGE_TOP_RIGHT:
    case GUI::BT_EDGE_BOTTOM_LEFT:
    case GUI::BT_EDGE_BOTTOM_RIGHT:
    case GUI::BT_SUNKEN_EDGE_TOP:
    case GUI::BT_SUNKEN_EDGE_BOTTOM:
    case GUI::BT_SUNKEN_EDGE_TOP_LEFT:
    case GUI::BT_SUNKEN_EDGE_TOP_RIGHT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT:
      if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
      {
        return 1;
      }
      if ( ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
      ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
      {
        return 2;
      }
      break;
  }
  return 0;
}




void GUIComponent::SetTextureSection( const GUI::eBorderType eType, GR::Graphic::ContextDescriptor& CDSection )
{
  m_TextureSection[eType] = CDSection;
}



void GUIComponent::DrawFocusRect( GR::Graphic::GFXPage* pPage, const GR::tRect& rcFocus )
{
  if ( VisualStyle() & GUI::VFT_HIDE_FOCUS_RECT )
  {
    return;
  }

  for ( int i = rcFocus.Left; i < rcFocus.Right - 1; i += 2 )
  {
    pPage->PutPixel( i, rcFocus.Top, pPage->GetPixel( i, rcFocus.Top ) ^ 0xffffffff );
    pPage->PutPixel( i, rcFocus.Bottom - 1, pPage->GetPixel( i, rcFocus.Bottom - 1 ) ^ 0xffffffff );
  }
  for ( int i = rcFocus.Top + 2; i < rcFocus.Bottom - 1; i += 2 )
  {
    pPage->PutPixel( rcFocus.Left, i, pPage->GetPixel( rcFocus.Left, i ) ^ 0xffffffff );
    pPage->PutPixel( rcFocus.Right - 1, i, pPage->GetPixel( rcFocus.Right - 1, i ) ^ 0xffffffff );
  }
}