#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"
#include "GUIScreen.h"

#include <debug\debugclient.h>



GUIComponentDisplayer::GUIComponentDisplayer() :
  m_pEventProducer( NULL ),
  m_iOffsetX( 0 ),
  m_iOffsetY( 0 ),
  m_pActualRenderer( NULL )
{
  GR::Service::Environment::Instance().SetService( "GUI", this );
}



GUIComponentDisplayer::~GUIComponentDisplayer()
{
  GR::Service::Environment::Instance().RemoveService( "GUI" );
}



void GUIComponentDisplayer::DisplayBackground()
{
  if ( m_Components.empty() )
  {
    return;
  }
  if ( m_pActualRenderer )
  {
    m_pRenderer = m_pActualRenderer;

    GUIComponent*  pComp = (GUIComponent*)m_Components.front();

    if ( pComp->Class() == "Screen" )
    {
      GUIScreen*  pScreen = (GUIScreen*)pComp;

      if ( !pScreen->m_Image.empty() )
      {
        if ( m_Background.m_Filename.empty() )
        {
          Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
          if ( pLoader )
          {
            Xtreme::Asset::XAsset* pAssetTexture = pLoader->Asset( Xtreme::Asset::XA_IMAGE, pScreen->m_Image.c_str() );
            if ( pAssetTexture == NULL )
            {
              dh::Log( "GUIComponentDisplayer::DisplayBackground Image (%s) not found", pScreen->m_Image.c_str() );
            }
            else
            {
              XTexture* pTexture = (XTexture*)pAssetTexture->Handle( "Texture" );
              if ( m_Background.m_pRenderer == NULL )
              {
                m_Background.Initialize( m_pActualRenderer, m_pEventProducer );
              }
              if ( m_Background.m_Filename != GR::String( pTexture->m_LoadedFromFile.c_str() ) )
              {
                m_Background.Clear();
                m_Background.LoadImage( pTexture->m_LoadedFromFile.c_str() );
                //dh::Log( "reload hugetexture %s", pTexture->m_LoadedFromFile.c_str() );
              }
            }
          }
        }
        m_Background.Render2d( *m_pActualRenderer, 0, 0 );
      }
    }
  }
}



void GUIComponentDisplayer::SetClipping( int iX, int iY, int iWidth, int iHeight )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( ( iX >= (int)m_pActualRenderer->Width() )
  ||   ( iX + iWidth <= 0 )
  ||   ( iY >= (int)m_pActualRenderer->Height() )
  ||   ( iY + iHeight <= 0 )
  ||   ( iWidth <= 0 )
  ||   ( iHeight <= 0 ) )
  {
    m_NothingIsVisible = true;
    return;
  }
  m_NothingIsVisible = false;
  if ( iX < 0 )
  {
    iWidth += iX;
    iX = 0;
  }
  if ( iX + iWidth > (int)m_pActualRenderer->Width() )
  {
    iWidth = m_pActualRenderer->Width() - iX;
  }
  if ( iY < 0 )
  {
    iHeight += iY;
    iY = 0;
  }
  if ( iY + iHeight > (int)m_pActualRenderer->Height() )
  {
    iHeight = m_pActualRenderer->Height() - iY;
  }

  XViewport   Viewport;

  Viewport.X      = iX;
  Viewport.Y      = iY;
  Viewport.Width  = iWidth;
  Viewport.Height = iHeight;

  ScreenToVirtual( Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height );

  m_pActualRenderer->SetViewport( Viewport );
}



GR::tPoint GUIComponentDisplayer::GetOffset()
{
  if ( m_pActualRenderer == NULL )
  {
    return GR::tPoint();
  }
  return m_pActualRenderer->Offset();
}



void GUIComponentDisplayer::SetOffset( int iX, int iY )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  m_pActualRenderer->Offset( GR::tPoint( iX, iY ) );
}



void GUIComponentDisplayer::PushClipValues()
{
  XViewport     Viewport = m_pActualRenderer->Viewport();

  GR::tPoint    ptOffset = m_pActualRenderer->Offset();

  // hier das Offset abziehen war neu
  StoreClipValues( Viewport.X,
                   Viewport.Y,
                   Viewport.Width - ptOffset.x,
                   Viewport.Height - ptOffset.y,
                   ptOffset.x,
                   ptOffset.y );
}



GUIComponentDisplayer& GUIComponentDisplayer::Instance()
{
  static GUIComponentDisplayer   g_Instance;

  return g_Instance;
}



void GUIComponentDisplayer::DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4 )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  VirtualToScreen( X, Y, Width, Height );

  m_pActualRenderer->RenderQuad( X, Y, Width, Height, Color );
}



void GUIComponentDisplayer::DrawRect( int X, int Y, int Width, int Height, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  VirtualToScreen( X, Y, Width, Height );

  m_pActualRenderer->RenderRect( GR::tPoint( X, Y ), GR::tPoint( Width, Height ), Color );
}



void GUIComponentDisplayer::DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  int     X = Pos.x;
  int     Y = Pos.y;
  int     Width = Size.x;
  int     Height = Size.y;
  VirtualToScreen( X, Y, Width, Height );

  m_pActualRenderer->RenderRect( GR::tPoint( X, Y ), GR::tPoint( Width, Height ), Color );
}



void GUIComponentDisplayer::DrawTextureSection( int X, int Y, const XTextureSection& TexSection, GR::u32 Color, int AlternativeWidth, int AlternativeHeight, GR::u32 AlternativeFlags )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( m_VirtualSize.x == 0 )
  {
    // TODO!
    //m_pActualRenderer->RenderTextureSectionColorized( X, Y, TexSection, Color, AlternativeWidth, AlternativeHeight, AlternativeFlags, 0.000000f );
    m_pActualRenderer->RenderTextureSectionColorized( X, Y, TexSection, Color );
  }
  else
  {
    GR::tPoint    alternativePos = VirtualToScreen( GR::tPoint( X, Y ) );
    GR::tPoint    alternativeEndPos( X + TexSection.m_Width, Y + TexSection.m_Height );
    if ( AlternativeWidth != -1 )
    {
      alternativeEndPos.x = X + AlternativeWidth;
    }
    if ( AlternativeHeight != -1 )
    {
      alternativeEndPos.y = Y + AlternativeHeight;
    }
    alternativeEndPos = VirtualToScreen( alternativeEndPos );

    // TODO!
    //m_pActualRenderer->RenderTextureSectionColorized( alternativePos.x, alternativePos.y, TexSection, Color, alternativeEndPos.x - alternativePos.x, alternativeEndPos.y - alternativePos.y, AlternativeFlags, 0.000000f );
    m_pActualRenderer->RenderTextureSectionColorized( alternativePos.x, alternativePos.y, TexSection, Color );
  }
}



void GUIComponentDisplayer::DrawText( Interface::IFont* pFont,
                                      const GR::String& Text,
                                      const GR::tRect& Rect,
                                      GR::u32 TextAlignment,
                                      GR::u32 Color )
{
  DrawText( pFont, 0, 0, Text, TextAlignment, Color, &Rect );
}



void GUIComponentDisplayer::DrawText( Interface::IFont* pFontA,
                                      int XOffset, int YOffset,
                                      const GR::String& Text,
                                      GR::u32 TextAlignment,
                                      GR::u32 Color,
                                      const GR::tRect* pRect )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  XFont* pFont = (XFont*)pFontA;
  if ( pFont == NULL )
  {
    return;
  }

  // NOTE - THIS IS NOT THE CORRECT CLIENTRECT! (not of the component to be drawn, but of the full displayer)
  GR::tRect   rectText;

  GetClientRect( rectText );
  if ( pRect != NULL )
  {
    rectText = *pRect;
  }

  //m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( TextAlignment & GUI::AF_MULTILINE )
  {
    std::vector<GR::String>    vectText;

    if ( TextAlignment & GUI::AF_AUTOWRAP )
    {
      GUI::WrapText( pFont, Text, rectText, vectText );
    }
    else
    {
      GR::tRect     emptyRect;
      GUI::BreakText( pFont, Text, emptyRect, vectText );
    }

    int   y = rectText.position().y;

    if ( ( TextAlignment & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
    {
      y += ( rectText.height() - 20 * (int)vectText.size() ) / 2;
    }
    else if ( TextAlignment & GUI::AF_BOTTOM )
    {
      y = rectText.Bottom - 20 * (int)vectText.size();
    }

    std::vector<GR::String>::iterator    it( vectText.begin() );
    while ( it != vectText.end() )
    {
      GR::String& strText = *it;

      int     x = rectText.Left;

      int     iLength = pFont->TextLength( strText );

      if ( ( TextAlignment & GUI::AF_CENTER ) == GUI::AF_CENTER )
      {
        x += ( rectText.width() - iLength ) / 2;
      }
      else if ( TextAlignment & GUI::AF_RIGHT )
      {
        x = rectText.Right - iLength;
      }

      GR::tPoint    textPos( XOffset + x, YOffset + y );

      if ( m_VirtualSize.x != 0 )
      {
        GR::f32     scaleX = ( ( GR::f32 )m_PhysicalSize.x / m_VirtualSize.x );
        GR::f32     scaleY = ( ( GR::f32 )m_PhysicalSize.y / m_VirtualSize.y );

        if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
        {
          GR::tPoint    shadowPos( textPos );

          shadowPos.offset( 1, 1 );
          shadowPos = VirtualToScreen( shadowPos );
          m_pActualRenderer->RenderText( (X2dFont*)pFont, shadowPos.x, shadowPos.y, strText, scaleX, scaleY, 0xff000000 );
        }
        textPos = VirtualToScreen( textPos );
        m_pActualRenderer->RenderText( (X2dFont*)pFont, textPos.x, textPos.y, strText, scaleX, scaleY, Color );
      }
      else
      {
        if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
        {
          GR::tPoint    shadowPos( textPos );
          shadowPos.offset( 1, 1 );
          shadowPos = VirtualToScreen( shadowPos );
          m_pActualRenderer->RenderText( (X2dFont*)pFont, shadowPos.x, shadowPos.y, strText, 0xff000000 );
        }
        textPos = VirtualToScreen( textPos );
        m_pActualRenderer->RenderText( (X2dFont*)pFont, textPos.x, textPos.y, strText, Color );
      }

      ++it;

      GR::tPoint    textHeight = VirtualToScreen( GR::tPoint( 0, pFont->TextHeight() ) );
      y += textHeight.y;
    }
    return;
  }
  /*
  int   iY = rectText.position().y;

  if ( ( textAlignment & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
  {
    iY += ( rectText.height() - 20 * (int)vectText.size() ) / 2;
  }
  else if ( textAlignment & GUI::AF_BOTTOM )
  {
    iY = rectText.Bottom - 20 * (int)vectText.size();
  }
  */

  GR::tPoint    ptText = GUI::TextOffset( pFont, Text, TextAlignment, rectText );
  GR::tPoint    ptShadow( ptText );

  if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
  {
    ptShadow.offset( 1, 1 );
    ptShadow.offset( XOffset, YOffset );
  }
  ptText.offset( XOffset, YOffset );
  ptText = VirtualToScreen( ptText );

  if ( m_VirtualSize.x != 0 )
  {
    GR::f32     scaleX = ( ( GR::f32 )m_PhysicalSize.x / m_VirtualSize.x );
    GR::f32     scaleY = ( ( GR::f32 )m_PhysicalSize.y / m_VirtualSize.y );

    if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
    {
      m_pActualRenderer->RenderText( (X2dFont*)pFont, ptShadow.x, ptShadow.y, Text, scaleX, scaleY, 0xff000000 );
    }
    m_pActualRenderer->RenderText( (X2dFont*)pFont, ptText.x, ptText.y, Text, scaleX, scaleY, Color );
  }
  else
  {
    if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
    {
      m_pActualRenderer->RenderText( (X2dFont*)pFont, ptShadow.x, ptShadow.y, Text, 0xff000000 );
    }
    m_pActualRenderer->RenderText( (X2dFont*)pFont, ptText.x, ptText.y, Text, Color );
  }
}



void GUIComponentDisplayer::DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 )
{
  m_pActualRenderer->RenderLine( Pos1, Pos2, Color );
}



void GUIComponentDisplayer::DrawEdge( GR::u32 VisualStyle, const GR::tRect& rectEdge )
{
  int   iX1 = rectEdge.position().x,
    iY1 = rectEdge.position().y,
    iX2 = iX1 + rectEdge.width() - 1,
    iY2 = iY1 + rectEdge.height() - 1;


  GR::u32 dwColorLight = GetSysColor( GUI::COL_3DLIGHT ),
          dwColorHiLight = GetSysColor( GUI::COL_BTNHIGHLIGHT ),
          dwColorDark = GetSysColor( GUI::COL_BTNSHADOW ),
          dwColorVeryDark = GetSysColor( GUI::COL_3DDKSHADOW );


  if ( VisualStyle & GUI::VFT_FLAT_BORDER )
  {
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX2 - 1, iY1 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX1, iY2 - 1 ), dwColorVeryDark );

    DrawLine( GR::tPoint( iX2, iY1 + 1 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1 + 1, iY2 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );
  }
  else if ( VisualStyle & GUI::VFT_RAISED_BORDER )
  {
    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX2 - 2, iY1 + 1 ), dwColorLight );
    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX1 + 1, iY2 - 2 ), dwColorLight );

    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX2 - 1, iY1 ), dwColorHiLight );
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX1, iY2 - 1 ), dwColorHiLight );

    DrawLine( GR::tPoint( iX2, iY1 + 1 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1 + 1, iY2 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );

    DrawLine( GR::tPoint( iX2 - 1, iY1 + 2 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorDark );
    DrawLine( GR::tPoint( iX1 + 2, iY2 - 1 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorDark );
  }
  else if ( VisualStyle & GUI::VFT_SUNKEN_BORDER )
  {
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX2 - 1, iY1 ), dwColorDark );
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX1, iY2 - 1 ), dwColorDark );

    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX2 - 2, iY1 + 1 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX1 + 1, iY2 - 2 ), dwColorVeryDark );

    DrawLine( GR::tPoint( iX2, iY1 + 1 ), GR::tPoint( iX2, iY2 ), dwColorLight );
    DrawLine( GR::tPoint( iX1 + 1, iY2 ), GR::tPoint( iX2, iY2 ), dwColorLight );

    DrawLine( GR::tPoint( iX2 - 1, iY1 + 2 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorHiLight );
    DrawLine( GR::tPoint( iX1 + 2, iY2 - 1 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorHiLight );
  }
}



void GUIComponentDisplayer::DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge, const std::vector<std::pair<XTextureSection, GR::u32> >& Sections )
{
  int   iX1 = rectEdge.position().x,
        iY1 = rectEdge.position().y,
        iX2 = iX1 + rectEdge.width() - 1,
        iY2 = iY1 + rectEdge.height() - 1;


  if ( ( Sections[GUI::BT_EDGE_TOP].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_TOP_LEFT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_LEFT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_RIGHT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_BOTTOM].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_pTexture ) )
  {
    if ( ( edgeType & GUI::VFT_RAISED_BORDER )
    ||   ( edgeType & GUI::VFT_FLAT_BORDER ) )
    {
      if ( Sections[GUI::BT_EDGE_TOP].first.m_Width )
      {
        for ( int i = 0; i < ( iX2 - iX1 - Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Width - Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_Width ) / Sections[GUI::BT_EDGE_TOP].first.m_Width + 1; ++i )
        {
          DrawTextureSection( iX1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Width + i * Sections[GUI::BT_EDGE_TOP].first.m_Width,
                              iY1,
                              Sections[GUI::BT_EDGE_TOP].first );
        }
      }
      if ( Sections[GUI::BT_EDGE_BOTTOM].first.m_Width )
      {
        for ( int i = 0; i < ( iX2 - iX1 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Width - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Width ) / Sections[GUI::BT_EDGE_BOTTOM].first.m_Width + 1; ++i )
        {
          DrawTextureSection( iX1 + Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Width + i * Sections[GUI::BT_EDGE_BOTTOM].first.m_Width,
                                           iY2 - Sections[GUI::BT_EDGE_BOTTOM].first.m_Height + 1,
                              Sections[GUI::BT_EDGE_BOTTOM].first );
        }
      }
      if ( Sections[GUI::BT_EDGE_LEFT].first.m_Height )
      {
        for ( int i = 0; i < ( iY2 - iY1 - Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Height - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height ) / Sections[GUI::BT_EDGE_LEFT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( iX1,
                              iY1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Height + i * Sections[GUI::BT_EDGE_LEFT].first.m_Height,
                              Sections[GUI::BT_EDGE_LEFT].first );
        }
      }
      if ( Sections[GUI::BT_EDGE_RIGHT].first.m_Height )
      {
        for ( int i = 0; i < ( iY2 - iY1 - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Height - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Height ) / Sections[GUI::BT_EDGE_RIGHT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( iX2 - Sections[GUI::BT_EDGE_RIGHT].first.m_Width + 1,
                              iY1 + Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_Height + i * Sections[GUI::BT_EDGE_RIGHT].first.m_Height,
                              Sections[GUI::BT_EDGE_RIGHT].first );
        }
      }

      // Ecken
      if ( Sections[GUI::BT_EDGE_TOP_LEFT].first.m_pTexture )
      {
        DrawTextureSection( iX1,
                            iY1, 
                            Sections[GUI::BT_EDGE_TOP_LEFT].first );
      }
      if ( Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( iX2 - Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_Width + 1,
                            iY1,
                            Sections[GUI::BT_EDGE_TOP_RIGHT].first );
      }
      if ( Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_pTexture )
      {
        DrawTextureSection( iX1,
                            iY2 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_EDGE_BOTTOM_LEFT].first );
      }
      if ( Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( iX2 - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Width + 1,
                            iY2 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first );
      }
      return;
    }
    else if ( edgeType & GUI::VFT_SUNKEN_BORDER )
    {
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP].first.m_Width )
      {
        for ( int i = 0; i < ( iX2 - iX1 - Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Width - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Width ) / Sections[GUI::BT_SUNKEN_EDGE_TOP].first.m_Width + 1; ++i )
        {
          DrawTextureSection( iX1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Width + i * Sections[GUI::BT_SUNKEN_EDGE_TOP].first.m_Width,
                              iY1,
                              Sections[GUI::BT_SUNKEN_EDGE_TOP].first );
        }
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Width )
      {
        for ( int i = 0; i < ( iX2 - iX1 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Width - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.m_Width ) / Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Width + 1; ++i )
        {
          DrawTextureSection( iX1 + Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Width + i * Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Width,
                              iY2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Height + 1,
                              Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first );
        }
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_LEFT].first.m_Height )
      {
        for ( int i = 0; i < ( iY2 - iY1 - Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Height - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Height ) / Sections[GUI::BT_SUNKEN_EDGE_LEFT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( iX1,
                                        iY1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Height + i * Sections[GUI::BT_SUNKEN_EDGE_LEFT].first.m_Height,
                                        Sections[GUI::BT_SUNKEN_EDGE_LEFT].first );
        }
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height )
      {
        for ( int i = 0; i < ( iY2 - iY1 - Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.m_Height ) / Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( iX2 - Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Width + 1,
                                        iY1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Height + i * Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height,
                                        Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first );
        }
      }

      // Ecken
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_pTexture )
      {
        DrawTextureSection( iX1, iY1, Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first );
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( iX2 - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Width + 1,
                            iY1,
                            Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first );
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_pTexture )
      {
        DrawTextureSection( iX1,
                            iY2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first );
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( iX2 - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Width + 1,
                            iY2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first );
      }
      return;
    }
  }


  GR::u32       dwColorLight    = GetSysColor( GUI::COL_3DLIGHT ),
                dwColorHiLight  = GetSysColor( GUI::COL_BTNHIGHLIGHT ),
                dwColorDark     = GetSysColor( GUI::COL_BTNSHADOW ),
                dwColorVeryDark = GetSysColor( GUI::COL_3DDKSHADOW );


  if ( edgeType & GUI::VFT_FLAT_BORDER )
  {
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX2 - 1, iY1 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX1, iY2 - 1 ), dwColorVeryDark );

    DrawLine( GR::tPoint( iX2, iY1 + 1 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1 + 1, iY2 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );
  }
  else if ( edgeType & GUI::VFT_RAISED_BORDER )
  {
    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX2 - 2, iY1 + 1 ), dwColorLight );
    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX1 + 1, iY2 - 2 ), dwColorLight );

    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX2 - 1, iY1 ), dwColorHiLight );
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX1, iY2 - 1 ), dwColorHiLight );

    DrawLine( GR::tPoint( iX2, iY1 + 1 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1 + 1, iY2 ), GR::tPoint( iX2, iY2 ), dwColorVeryDark );

    DrawLine( GR::tPoint( iX2 - 1, iY1 + 2 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorDark );
    DrawLine( GR::tPoint( iX1 + 2, iY2 - 1 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorDark );
  }
  else if ( edgeType & GUI::VFT_SUNKEN_BORDER )
  {
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX2 - 1, iY1 ), dwColorDark );
    DrawLine( GR::tPoint( iX1, iY1 ), GR::tPoint( iX1, iY2 - 1 ), dwColorDark );

    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX2 - 2, iY1 + 1 ), dwColorVeryDark );
    DrawLine( GR::tPoint( iX1 + 1, iY1 + 1 ), GR::tPoint( iX1 + 1, iY2 - 2 ), dwColorVeryDark );

    DrawLine( GR::tPoint( iX2, iY1 + 1 ), GR::tPoint( iX2, iY2 ), dwColorLight );
    DrawLine( GR::tPoint( iX1 + 1, iY2 ), GR::tPoint( iX2, iY2 ), dwColorLight );

    DrawLine( GR::tPoint( iX2 - 1, iY1 + 2 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorHiLight );
    DrawLine( GR::tPoint( iX1 + 2, iY2 - 1 ), GR::tPoint( iX2 - 1, iY2 - 1 ), dwColorHiLight );
  }
}



void GUIComponentDisplayer::DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color )
{
  if ( Section.m_Width == 0 )
  {
    return;
  }
  for ( int i = 0; i < Width / Section.m_Width + 1; ++i )
  {
    DrawTextureSection( X + i * Section.m_Width, Y, Section, Color );
  }
}



void GUIComponentDisplayer::DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color )
{
  if ( Section.m_Height == 0 )
  {
    return;
  }
  for ( int i = 0; i < Height / Section.m_Height + 1; ++i )
  {
    DrawTextureSection( X, Y + i * Section.m_Height, Section, Color );
  }
}



void GUIComponentDisplayer::DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color )
{
  if ( ( Section.m_Width == 0 )
  ||   ( Section.m_Height == 0 ) )
  {
    return;
  }
  for ( int j = 0; j < Height / Section.m_Height + 1; ++j )
  {
    for ( int i = 0; i < Width / Section.m_Width + 1; ++i )
    {
      DrawTextureSection( X + i * Section.m_Width,
                          Y + j * Section.m_Height,
                          Section, Color );
    }
  }
}



void GUIComponentDisplayer::DrawFocusRect( const GR::tRect& rcFocus, GR::u32 VisualStyle )
{
  if ( VisualStyle & GUI::VFT_HIDE_FOCUS_RECT )
  {
    return;
  }

  for ( int i = rcFocus.Left; i < rcFocus.Right - 1; i += 2 )
  {
    DrawLine( GR::tPoint( i, rcFocus.Top ),
                         GR::tPoint( i, rcFocus.Top ),
                         0xff808080 );
    DrawLine( GR::tPoint( i, rcFocus.Bottom - 1 ),
                         GR::tPoint( i, rcFocus.Bottom - 1 ),
                         0xff808080 );
  }
  for ( int i = rcFocus.Top + 2; i < rcFocus.Bottom - 1; i += 2 )
  {
    DrawLine( GR::tPoint( rcFocus.Left, i ),
                         GR::tPoint( rcFocus.Left, i ),
                         0xff808080 );
    DrawLine( GR::tPoint( rcFocus.Right - 1, i ),
                         GR::tPoint( rcFocus.Right - 1, i ),
                         0xff808080 );
  }
}



void GUIComponentDisplayer::SetRenderer( X2dRenderer* pRenderer )
{
  m_pActualRenderer = pRenderer;
  m_pRenderer       = pRenderer;
}