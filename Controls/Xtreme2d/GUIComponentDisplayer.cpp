#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"
#include "GUIScreen.h"

#include <debug\debugclient.h>



GUIComponentDisplayer::GUIComponentDisplayer() :
  m_pEventProducer( NULL ),
  m_OffsetX( 0 ),
  m_OffsetY( 0 ),
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
              }
            }
          }
        }
        m_Background.Render2d( *m_pActualRenderer, 0, 0 );
      }
    }
  }
}



void GUIComponentDisplayer::SetClipping( int X, int Y, int Width, int Height )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( ( X >= (int)m_pActualRenderer->Width() )
  ||   ( X + Width <= 0 )
  ||   ( Y >= (int)m_pActualRenderer->Height() )
  ||   ( Y + Height <= 0 )
  ||   ( Width <= 0 )
  ||   ( Height <= 0 ) )
  {
    m_NothingIsVisible = true;
    return;
  }
  m_NothingIsVisible = false;
  if ( X < 0 )
  {
    Width += X;
    X = 0;
  }
  if ( X + Width > (int)m_pActualRenderer->Width() )
  {
    Width = m_pActualRenderer->Width() - X;
  }
  if ( Y < 0 )
  {
    Height += Y;
    Y = 0;
  }
  if ( Y + Height > (int)m_pActualRenderer->Height() )
  {
    Height = m_pActualRenderer->Height() - Y;
  }

  XViewport   Viewport;

  Viewport.X      = X;
  Viewport.Y      = Y;
  Viewport.Width  = Width;
  Viewport.Height = Height;

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



void GUIComponentDisplayer::SetOffset( int X, int Y )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  m_pActualRenderer->Offset( GR::tPoint( X, Y ) );
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



void GUIComponentDisplayer::DrawTextureSectionColorKeyed( int X, int Y, const XTextureSection& TexSection, 
                                                          GR::u32 ColorKey,
                                                          GR::u32 Color, 
                                                          int AlternativeWidth, int AlternativeHeight, GR::u32 AlternativeFlags )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( m_VirtualSize.x == 0 )
  {
    // TODO!
    //m_pActualRenderer->RenderTextureSectionColorized( X, Y, TexSection, Color, AlternativeWidth, AlternativeHeight, AlternativeFlags, 0.000000f );
    m_pActualRenderer->RenderTextureSectionColorKeyedColorized( X, Y, TexSection, ColorKey, Color );
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
    m_pActualRenderer->RenderTextureSectionColorKeyedColorized( alternativePos.x, alternativePos.y, TexSection, ColorKey, Color );
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

    int   y = rectText.Top;

    if ( ( TextAlignment & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
    {
      y += ( rectText.Height() - 20 * (int)vectText.size() ) / 2;
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
        x += ( rectText.Width() - iLength ) / 2;
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

          shadowPos.Offset( 1, 1 );
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
          shadowPos.Offset( 1, 1 );
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
  GR::tPoint    ptText = GUI::TextOffset( pFont, Text, TextAlignment, rectText );
  GR::tPoint    ptShadow( ptText );

  if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
  {
    ptShadow.Offset( 1, 1 );
    ptShadow.Offset( XOffset, YOffset );
  }
  ptText.Offset( XOffset, YOffset );
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



void GUIComponentDisplayer::DrawEdge( GR::u32 VisualStyle, const GR::tRect& Rect )
{
  int   X1 = Rect.Left,
        Y1 = Rect.Top,
        X2 = X1 + Rect.Width() - 1,
        Y2 = Y1 + Rect.Height() - 1;


  GR::u32 ColorLight = GetSysColor( GUI::COL_3DLIGHT ),
          ColorHiLight = GetSysColor( GUI::COL_BTNHIGHLIGHT ),
          ColorDark = GetSysColor( GUI::COL_BTNSHADOW ),
          ColorVeryDark = GetSysColor( GUI::COL_3DDKSHADOW );


  if ( VisualStyle & GUI::VFT_FLAT_BORDER )
  {
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X2 - 1, Y1 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X1, Y2 - 1 ), ColorVeryDark );

    DrawLine( GR::tPoint( X2, Y1 + 1 ), GR::tPoint( X2, Y2 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1 + 1, Y2 ), GR::tPoint( X2, Y2 ), ColorVeryDark );
  }
  else if ( VisualStyle & GUI::VFT_RAISED_BORDER )
  {
    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X2 - 2, Y1 + 1 ), ColorLight );
    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X1 + 1, Y2 - 2 ), ColorLight );

    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X2 - 1, Y1 ), ColorHiLight );
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X1, Y2 - 1 ), ColorHiLight );

    DrawLine( GR::tPoint( X2, Y1 + 1 ), GR::tPoint( X2, Y2 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1 + 1, Y2 ), GR::tPoint( X2, Y2 ), ColorVeryDark );

    DrawLine( GR::tPoint( X2 - 1, Y1 + 2 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorDark );
    DrawLine( GR::tPoint( X1 + 2, Y2 - 1 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorDark );
  }
  else if ( VisualStyle & GUI::VFT_SUNKEN_BORDER )
  {
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X2 - 1, Y1 ), ColorDark );
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X1, Y2 - 1 ), ColorDark );

    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X2 - 2, Y1 + 1 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X1 + 1, Y2 - 2 ), ColorVeryDark );

    DrawLine( GR::tPoint( X2, Y1 + 1 ), GR::tPoint( X2, Y2 ), ColorLight );
    DrawLine( GR::tPoint( X1 + 1, Y2 ), GR::tPoint( X2, Y2 ), ColorLight );

    DrawLine( GR::tPoint( X2 - 1, Y1 + 2 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorHiLight );
    DrawLine( GR::tPoint( X1 + 2, Y2 - 1 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorHiLight );
  }
}



void GUIComponentDisplayer::DrawEdge( GR::u32 EdgeType, const GR::tRect& Rect, const std::vector<std::pair<XTextureSection, GR::u32> >& Sections )
{
  int   X1 = Rect.Left,
        Y1 = Rect.Top,
        X2 = X1 + Rect.Width() - 1,
        Y2 = Y1 + Rect.Height() - 1;


  if ( ( Sections[GUI::BT_EDGE_TOP].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_TOP_LEFT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_LEFT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_RIGHT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_BOTTOM].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_pTexture )
  ||   ( Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_pTexture ) )
  {
    if ( ( EdgeType & GUI::VFT_RAISED_BORDER )
    ||   ( EdgeType & GUI::VFT_FLAT_BORDER ) )
    {
      if ( Sections[GUI::BT_EDGE_TOP].first.m_Width )
      {
        for ( int i = 0; i < ( X2 - X1 - Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Width - Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_Width ) / Sections[GUI::BT_EDGE_TOP].first.m_Width + 1; ++i )
        {
          DrawTextureSection( X1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Width + i * Sections[GUI::BT_EDGE_TOP].first.m_Width,
                              Y1,
                              Sections[GUI::BT_EDGE_TOP].first );
        }
      }
      if ( Sections[GUI::BT_EDGE_BOTTOM].first.m_Width )
      {
        for ( int i = 0; i < ( X2 - X1 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Width - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Width ) / Sections[GUI::BT_EDGE_BOTTOM].first.m_Width + 1; ++i )
        {
          DrawTextureSection( X1 + Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Width + i * Sections[GUI::BT_EDGE_BOTTOM].first.m_Width,
                                           Y2 - Sections[GUI::BT_EDGE_BOTTOM].first.m_Height + 1,
                              Sections[GUI::BT_EDGE_BOTTOM].first );
        }
      }
      if ( Sections[GUI::BT_EDGE_LEFT].first.m_Height )
      {
        for ( int i = 0; i < ( Y2 - Y1 - Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Height - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height ) / Sections[GUI::BT_EDGE_LEFT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( X1,
                              Y1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Height + i * Sections[GUI::BT_EDGE_LEFT].first.m_Height,
                              Sections[GUI::BT_EDGE_LEFT].first );
        }
      }
      if ( Sections[GUI::BT_EDGE_RIGHT].first.m_Height )
      {
        for ( int i = 0; i < ( Y2 - Y1 - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Height - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Height ) / Sections[GUI::BT_EDGE_RIGHT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( X2 - Sections[GUI::BT_EDGE_RIGHT].first.m_Width + 1,
                              Y1 + Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_Height + i * Sections[GUI::BT_EDGE_RIGHT].first.m_Height,
                              Sections[GUI::BT_EDGE_RIGHT].first );
        }
      }

      // Ecken
      if ( Sections[GUI::BT_EDGE_TOP_LEFT].first.m_pTexture )
      {
        DrawTextureSection( X1,
                            Y1, 
                            Sections[GUI::BT_EDGE_TOP_LEFT].first );
      }
      if ( Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( X2 - Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_Width + 1,
                            Y1,
                            Sections[GUI::BT_EDGE_TOP_RIGHT].first );
      }
      if ( Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_pTexture )
      {
        DrawTextureSection( X1,
                            Y2 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_EDGE_BOTTOM_LEFT].first );
      }
      if ( Sections[GUI::BT_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( X2 - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Width + 1,
                            Y2 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first );
      }
      return;
    }
    else if ( EdgeType & GUI::VFT_SUNKEN_BORDER )
    {
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP].first.m_Width )
      {
        for ( int i = 0; i < ( X2 - X1 - Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Width - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Width ) / Sections[GUI::BT_SUNKEN_EDGE_TOP].first.m_Width + 1; ++i )
        {
          DrawTextureSection( X1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Width + i * Sections[GUI::BT_SUNKEN_EDGE_TOP].first.m_Width,
                              Y1,
                              Sections[GUI::BT_SUNKEN_EDGE_TOP].first );
        }
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Width )
      {
        for ( int i = 0; i < ( X2 - X1 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Width - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.m_Width ) / Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Width + 1; ++i )
        {
          DrawTextureSection( X1 + Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Width + i * Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Width,
                              Y2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.m_Height + 1,
                              Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first );
        }
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_LEFT].first.m_Height )
      {
        for ( int i = 0; i < ( Y2 - Y1 - Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Height - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Height ) / Sections[GUI::BT_SUNKEN_EDGE_LEFT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( X1,
                                        Y1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Height + i * Sections[GUI::BT_SUNKEN_EDGE_LEFT].first.m_Height,
                                        Sections[GUI::BT_SUNKEN_EDGE_LEFT].first );
        }
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height )
      {
        for ( int i = 0; i < ( Y2 - Y1 - Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.m_Height ) / Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height + 1; ++i )
        {
          DrawTextureSection( X2 - Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Width + 1,
                                        Y1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Height + i * Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.m_Height,
                                        Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first );
        }
      }

      // Ecken
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_pTexture )
      {
        DrawTextureSection( X1, Y1, Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first );
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( X2 - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Width + 1,
                            Y1,
                            Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first );
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_pTexture )
      {
        DrawTextureSection( X1,
                            Y2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first );
      }
      if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_pTexture )
      {
        DrawTextureSection( X2 - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.m_Width + 1,
                            Y2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.m_Height + 1,
                            Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first );
      }
      return;
    }
  }


  GR::u32       ColorLight    = GetSysColor( GUI::COL_3DLIGHT ),
                ColorHiLight  = GetSysColor( GUI::COL_BTNHIGHLIGHT ),
                ColorDark     = GetSysColor( GUI::COL_BTNSHADOW ),
                ColorVeryDark = GetSysColor( GUI::COL_3DDKSHADOW );


  if ( EdgeType & GUI::VFT_FLAT_BORDER )
  {
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X2 - 1, Y1 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X1, Y2 - 1 ), ColorVeryDark );

    DrawLine( GR::tPoint( X2, Y1 + 1 ), GR::tPoint( X2, Y2 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1 + 1, Y2 ), GR::tPoint( X2, Y2 ), ColorVeryDark );
  }
  else if ( EdgeType & GUI::VFT_RAISED_BORDER )
  {
    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X2 - 2, Y1 + 1 ), ColorLight );
    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X1 + 1, Y2 - 2 ), ColorLight );

    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X2 - 1, Y1 ), ColorHiLight );
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X1, Y2 - 1 ), ColorHiLight );

    DrawLine( GR::tPoint( X2, Y1 + 1 ), GR::tPoint( X2, Y2 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1 + 1, Y2 ), GR::tPoint( X2, Y2 ), ColorVeryDark );

    DrawLine( GR::tPoint( X2 - 1, Y1 + 2 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorDark );
    DrawLine( GR::tPoint( X1 + 2, Y2 - 1 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorDark );
  }
  else if ( EdgeType & GUI::VFT_SUNKEN_BORDER )
  {
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X2 - 1, Y1 ), ColorDark );
    DrawLine( GR::tPoint( X1, Y1 ), GR::tPoint( X1, Y2 - 1 ), ColorDark );

    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X2 - 2, Y1 + 1 ), ColorVeryDark );
    DrawLine( GR::tPoint( X1 + 1, Y1 + 1 ), GR::tPoint( X1 + 1, Y2 - 2 ), ColorVeryDark );

    DrawLine( GR::tPoint( X2, Y1 + 1 ), GR::tPoint( X2, Y2 ), ColorLight );
    DrawLine( GR::tPoint( X1 + 1, Y2 ), GR::tPoint( X2, Y2 ), ColorLight );

    DrawLine( GR::tPoint( X2 - 1, Y1 + 2 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorHiLight );
    DrawLine( GR::tPoint( X1 + 2, Y2 - 1 ), GR::tPoint( X2 - 1, Y2 - 1 ), ColorHiLight );
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



void GUIComponentDisplayer::DrawFocusRect( const GR::tRect& Rect, GR::u32 VisualStyle )
{
  if ( VisualStyle & GUI::VFT_HIDE_FOCUS_RECT )
  {
    return;
  }

  for ( int i = Rect.Left; i < Rect.Right - 1; i += 2 )
  {
    DrawLine( GR::tPoint( i, Rect.Top ),
                         GR::tPoint( i, Rect.Top ),
                         0xff808080 );
    DrawLine( GR::tPoint( i, Rect.Bottom - 1 ),
                         GR::tPoint( i, Rect.Bottom - 1 ),
                         0xff808080 );
  }
  for ( int i = Rect.Top + 2; i < Rect.Bottom - 1; i += 2 )
  {
    DrawLine( GR::tPoint( Rect.Left, i ),
                         GR::tPoint( Rect.Left, i ),
                         0xff808080 );
    DrawLine( GR::tPoint( Rect.Right - 1, i ),
                         GR::tPoint( Rect.Right - 1, i ),
                         0xff808080 );
  }
}



void GUIComponentDisplayer::SetRenderer( X2dRenderer* pRenderer )
{
  m_pActualRenderer = pRenderer;
  m_pRenderer       = pRenderer;
}