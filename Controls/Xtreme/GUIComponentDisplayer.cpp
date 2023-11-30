#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"

#include "GUIScreen.h"



GUIComponentDisplayer::GUIComponentDisplayer() :
  m_pEventProducer( NULL )
{
  GR::Service::Environment::Instance().SetService( "GUI", this );
}



GUIComponentDisplayer::~GUIComponentDisplayer()
{
  m_ShuttingDown = true;
  GR::Service::Environment::Instance().RemoveService( "GUI" );
}



void GUIComponentDisplayer::SetClipping( int X, int Y, int Width, int Height )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  m_Clipping.set( X, Y, Width, Height );

  XViewport     viewPort = m_pActualRenderer->Viewport();

  VirtualToScreen( X, Y, Width, Height );

  if ( X < 0 )
  {
    Width += X;
    X = 0;
  }
  if ( Y < 0 )
  {
    Height += Y;
    Y = 0;
  }
  if ( X + Width > (int)m_pActualRenderer->Width() )
  {
    Width = m_pActualRenderer->Width() - X;
  }
  if ( Y + Height >= (int)m_pActualRenderer->Height() )
  {
    Height = m_pActualRenderer->Height() - Y;
  }

  if ( X > (int)m_pActualRenderer->Width() )
  {
    X = m_pActualRenderer->Width();
    Width = 0;
  }
  if ( Y > (int)m_pActualRenderer->Height() )
  {
    Y = m_pActualRenderer->Height();
    Height = 0;
  }
  if ( Width < 0 )
  {
    Width = 0;
  }
  if ( Height < 0 )
  {
    Height = 0;
  }

  viewPort.X      = X;
  viewPort.Y      = Y;
  viewPort.Width  = Width;
  viewPort.Height = Height;

  if ( ( Width == 0 )
  ||   ( Height == 0 ) )
  {
    m_NothingIsVisible = true;
  }
  else
  {
    m_NothingIsVisible = false;
    m_pActualRenderer->SetViewport( viewPort );
  }
}



void GUIComponentDisplayer::SetOffset( int X, int Y )
{
  m_CurrentOffset.set( X, Y );

  GR::tPoint      offset = VirtualToScreen( GR::tPoint( X, Y ) );

  m_pActualRenderer->Offset( offset );
}



GR::tPoint GUIComponentDisplayer::GetOffset()
{
  return m_pActualRenderer->Offset();
}



void GUIComponentDisplayer::PushClipValues()
{
  //GR::tPoint    viewerOffset = m_pActualRenderer->Offset();
  //viewerOffset = ScreenToVirtual( viewerOffset );

  GR::tPoint viewerOffset = m_CurrentOffset;
  StoreClipValues( m_Clipping.Left,
                   m_Clipping.Top,
                   m_Clipping.width(),
                   m_Clipping.height(),
                   viewerOffset.x,
                   viewerOffset.y );
}



void GUIComponentDisplayer::DisplayBackground()
{
  m_AlreadyHandledComponentsThisFrame.clear();
  if ( m_Components.empty() )
  {
    return;
  }
  if ( m_pActualRenderer )
  {
    GUIComponent*  pComp = (GUIComponent*)m_Components.front();

    if ( pComp->Class() == "Screen" )
    {
      GUIScreen*  pScreen = (GUIScreen*)pComp;

      if ( !pScreen->m_Image.empty() )
      {
        if ( m_Background.m_Textures.empty() )
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
              if ( m_Background.m_Filename != pTexture->m_LoadedFromFile )
              {
                m_Background.Clear();
                m_Background.LoadImage( pTexture->m_LoadedFromFile.c_str() );
                //dh::Log( "reload hugetexture %s", pTexture->m_LoadedFromFile.c_str() );
              }
            }
          }
        }
        m_pActualRenderer->SetShader( XRenderer::ST_FLAT );
        // TODO - Virtual vs. physical!
        m_Background.Render2d( *m_pActualRenderer, 0, 0 );
      }
    }
  }
}



void GUIComponentDisplayer::DisplayAllControls()
{
  if ( m_pActualRenderer )
  {
    m_pRenderer = m_pActualRenderer;

    m_pActualRenderer->SetState( XRenderer::RS_ZBUFFER, XRenderer::RSV_DISABLE );

    m_pActualRenderer->SetShader( XRenderer::ST_FLAT );

    m_Clipping = GR::tRect( 0, 0, m_VirtualSize.x, m_VirtualSize.y );

    ComponentDisplayerBase::DisplayAllControls();
  }
}



GUIComponentDisplayer& GUIComponentDisplayer::Instance()
{
  static  GUIComponentDisplayer    g_Instance;

  return g_Instance;
}



void GUIComponentDisplayer::SetViewport( const XViewport& Viewport )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  XViewport   viewPort( Viewport );
  VirtualToScreen( viewPort.X, viewPort.Y, viewPort.Width, viewPort.Height );

  m_pActualRenderer->SetViewport( viewPort );
  /*
  m_ClientRect.position( Viewport.X, Viewport.Y );
  m_ClientRect.size( Viewport.Width, Viewport.Height );

  GUI::ComponentEvent   Event( CET_EXTENTS_CHANGED );

  Event.Position.set( Viewport.Width, Viewport.Height );

  NotifyAllComponents( Event );
  */
}



XViewport GUIComponentDisplayer::Viewport()
{
  if ( m_pActualRenderer == NULL )
  {
    return XViewport();
  }
  XViewport   viewport = m_pActualRenderer->Viewport();

  ScreenToVirtual( viewport.X, viewport.Y, viewport.Width, viewport.Height );

  return viewport;
}



void GUIComponentDisplayer::DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4 )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  VirtualToScreen( X, Y, Width, Height );

  m_pActualRenderer->SetTexture( 0, NULL );
  m_pActualRenderer->RenderQuad2d( X, Y, Width, Height, Color, Color2, Color3, Color4 );
}



void GUIComponentDisplayer::DrawEdge( GR::u32 VisualStyle, const GR::tRect& rectEdge )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  int   X1 = rectEdge.position().x,
        Y1 = rectEdge.position().y,
        X2 = X1 + rectEdge.size().x,
        Y2 = Y1 + rectEdge.size().y;

  m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( VisualStyle & GUI::VFT_RAISED_BORDER )
  {
    DrawQuad( X1, Y1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_3DLIGHT ) );
    DrawQuad( X1, Y1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_3DLIGHT ) );

    DrawQuad( X1 + 1, Y1 + 1, X2 - X1 - 2, 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
    DrawQuad( X1 + 1, Y1 + 1, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );

    DrawQuad( X2 - 2, Y1 + 2, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_BTNSHADOW ) );
    DrawQuad( X1 + 2, Y2 - 2, X2 - X1 - 2, 1, GetSysColor( GUI::COL_BTNSHADOW ) );

    DrawQuad( X2 - 1, Y1 + 1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
    DrawQuad( X1 + 1, Y2 - 1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
  }
  else if ( VisualStyle & GUI::VFT_SUNKEN_BORDER )
  {
    DrawQuad( X1, Y1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
    DrawQuad( X1, Y1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_3DDKSHADOW ) );

    DrawQuad( X1 + 1, Y1 + 1, X2 - X1 - 2, 1, GetSysColor( GUI::COL_BTNSHADOW ) );
    DrawQuad( X1 + 1, Y1 + 1, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_BTNSHADOW ) );

    DrawQuad( X2 - 2, Y1 + 2, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_3DLIGHT ) );
    DrawQuad( X1 + 2, Y2 - 2, X2 - X1 - 2, 1, GetSysColor( GUI::COL_3DLIGHT ) );

    DrawQuad( X2 - 1, Y1 + 1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
    DrawQuad( X1 + 1, Y2 - 1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
  }
  else if ( VisualStyle & GUI::VFT_FLAT_BORDER )
  {
    GR::u32     color3DDarkShadow = GetSysColor( GUI::COL_3DDKSHADOW ); //Component.GetColor( GUI::COL_3DDKSHADOW )
    DrawQuad( X1, Y1, X2 - X1, 1, color3DDarkShadow );
    DrawQuad( X1, Y1 + 1, 1, Y2 - Y1 - 2, color3DDarkShadow );

    DrawQuad( X2 - 1, Y1 + 1, 1, Y2 - Y1 - 2, color3DDarkShadow );
    DrawQuad( X1, Y2 - 1, X2 - X1, 1, color3DDarkShadow );
  }
}



void GUIComponentDisplayer::DrawEdge( GR::u32 VisualStyle,
                                      const GR::tRect& rectEdge,
                                      const std::vector<std::pair<XTextureSection, GR::u32> >& Sections )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  int   X1 = rectEdge.position().x,
    Y1 = rectEdge.position().y,
    X2 = X1 + rectEdge.size().x,
    Y2 = Y1 + rectEdge.size().y;

  m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( VisualStyle & GUI::VFT_RAISED_BORDER )
  {
    if ( Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth() == 0 )
    {
      // per Hand zeichnen
      DrawQuad( X1, Y1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_3DLIGHT ) );
      DrawQuad( X1, Y1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_3DLIGHT ) );

      DrawQuad( X1 + 1, Y1 + 1, X2 - X1 - 2, 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
      DrawQuad( X1 + 1, Y1 + 1, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );

      DrawQuad( X2 - 2, Y1 + 2, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_BTNSHADOW ) );
      DrawQuad( X1 + 2, Y2 - 2, X2 - X1 - 2, 1, GetSysColor( GUI::COL_BTNSHADOW ) );

      DrawQuad( X2 - 1, Y1 + 1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
      DrawQuad( X1 + 1, Y2 - 1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
    }
    else
    {
      GR::u32     NCColor = GetSysColor( GUI::COL_NC_AREA ); //Component.GetColor( GUI::COL_NC_AREA );

      DrawTextureSection( X1, Y1, Sections[GUI::BT_EDGE_TOP_LEFT].first, NCColor );
      if ( VisualStyle & GUI::VFT_REPEAT_BORDER )
      {
        DrawTextureSectionHRepeat( X1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth(),
                                   Y1,
                                   rectEdge.size().x - Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth() - Sections[GUI::BT_EDGE_TOP_RIGHT].first.VisualWidth(),
                                   Sections[GUI::BT_EDGE_TOP].first,
                                   NCColor );
      }
      else
      {
        DrawTextureSection( X1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.m_Width,
                            Y1, Sections[GUI::BT_EDGE_TOP].first,
                            NCColor,
                            rectEdge.size().x - Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth() - Sections[GUI::BT_EDGE_TOP_RIGHT].first.VisualWidth() );
      }
      DrawTextureSection( X2 - Sections[GUI::BT_EDGE_TOP_RIGHT].first.VisualWidth(),
                          Y1,
                          Sections[GUI::BT_EDGE_TOP_RIGHT].first, NCColor );
      if ( VisualStyle & GUI::VFT_REPEAT_BORDER )
      {
        DrawTextureSectionVRepeat( X1,
                                   Y1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualHeight(),
                                   rectEdge.size().y - 2 * Sections[GUI::BT_EDGE_TOP].first.VisualHeight(),
                                   Sections[GUI::BT_EDGE_LEFT].first,
                                   NCColor );
      }
      else
      {
        DrawTextureSection( X1,
                            Y1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualHeight(),
                            Sections[GUI::BT_EDGE_LEFT].first,
                            NCColor,
                            -1,
                            rectEdge.size().y - 2 * Sections[GUI::BT_EDGE_TOP].first.VisualHeight() );
      }
      if ( VisualStyle & GUI::VFT_REPEAT_BORDER )
      {
        DrawTextureSectionVRepeat( X2 - Sections[GUI::BT_EDGE_RIGHT].first.VisualWidth(),
                                   Y1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualHeight(),
                                   rectEdge.size().y - 2 * Sections[GUI::BT_EDGE_TOP].first.VisualHeight(),
                                   Sections[GUI::BT_EDGE_RIGHT].first,
                                   NCColor );
      }
      else
      {
        DrawTextureSection( X2 - Sections[GUI::BT_EDGE_RIGHT].first.VisualWidth(),
                            Y1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualHeight(),
                            Sections[GUI::BT_EDGE_RIGHT].first,
                            NCColor,
                            -1,
                            rectEdge.size().y - 2 * Sections[GUI::BT_EDGE_TOP].first.VisualHeight() );
      }
      DrawTextureSection( X1, Y2 - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.m_Height,
                          Sections[GUI::BT_EDGE_BOTTOM_LEFT].first, NCColor );
      if ( VisualStyle & GUI::VFT_REPEAT_BORDER )
      {
        DrawTextureSectionHRepeat( X1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth(),
                                   Y2 - Sections[GUI::BT_EDGE_BOTTOM].first.VisualHeight(),
                                   rectEdge.size().x - Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth() - Sections[GUI::BT_EDGE_TOP_RIGHT].first.VisualWidth(),
                                   Sections[GUI::BT_EDGE_BOTTOM].first,
                                   NCColor );
      }
      else
      {
        DrawTextureSection( X1 + Sections[GUI::BT_EDGE_TOP_LEFT].first.VisualWidth(),
                            Y2 - Sections[GUI::BT_EDGE_BOTTOM].first.VisualHeight(),
                            Sections[GUI::BT_EDGE_BOTTOM].first,
                            NCColor,
                            rectEdge.width() - Sections[GUI::BT_EDGE_BOTTOM_LEFT].first.VisualWidth() - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.VisualWidth() );
      }
      DrawTextureSection( X2 - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.VisualWidth(),
                          Y2 - Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first.VisualHeight(),
                          Sections[GUI::BT_EDGE_BOTTOM_RIGHT].first, NCColor );
    }
  }
  else if ( VisualStyle & GUI::VFT_SUNKEN_BORDER )
  {
    if ( Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.VisualWidth() == 0 )
    {
      // per Hand zeichnen
      DrawQuad( X1, Y1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_3DDKSHADOW ) );
      DrawQuad( X1, Y1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_3DDKSHADOW ) );

      DrawQuad( X1 + 1, Y1 + 1, X2 - X1 - 2, 1, GetSysColor( GUI::COL_BTNSHADOW ) );
      DrawQuad( X1 + 1, Y1 + 1, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_BTNSHADOW ) );

      DrawQuad( X2 - 2, Y1 + 2, 1, Y2 - Y1 - 2, GetSysColor( GUI::COL_3DLIGHT ) );
      DrawQuad( X1 + 2, Y2 - 2, X2 - X1 - 2, 1, GetSysColor( GUI::COL_3DLIGHT ) );

      DrawQuad( X2 - 1, Y1 + 1, 1, Y2 - Y1 - 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
      DrawQuad( X1 + 1, Y2 - 1, X2 - X1 - 1, 1, GetSysColor( GUI::COL_BTNHIGHLIGHT ) );
    }
    else
    {
      GR::u32     NCColor = GetSysColor( GUI::COL_NC_AREA ); //Component.GetColor( GUI::COL_NC_AREA );
      DrawTextureSection( X1, Y1, Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first, NCColor );
      DrawTextureSection( X1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.VisualWidth(),
                          Y1,
                          Sections[GUI::BT_SUNKEN_EDGE_TOP].first, NCColor,
                          rectEdge.size().x - Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.VisualWidth() - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.VisualWidth() );
      DrawTextureSection( X2 - Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first.VisualWidth(),
                          Y1,
                          Sections[GUI::BT_SUNKEN_EDGE_TOP_RIGHT].first, NCColor );
      DrawTextureSection( X1,
                          Y1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.VisualHeight(),
                          Sections[GUI::BT_SUNKEN_EDGE_LEFT].first,
                          NCColor, -1,
                          rectEdge.size().y - 2 * Sections[GUI::BT_SUNKEN_EDGE_TOP].first.VisualHeight() );
      DrawTextureSection( X2 - Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first.VisualWidth(),
                          Y1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.VisualHeight(),
                          Sections[GUI::BT_SUNKEN_EDGE_RIGHT].first, NCColor, -1, rectEdge.size().y - 2 * Sections[GUI::BT_SUNKEN_EDGE_TOP].first.VisualHeight() );
      DrawTextureSection( X1, Y2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.VisualHeight(),
                          Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first, NCColor );
      DrawTextureSection( X1 + Sections[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.VisualWidth(),
                          Y2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first.VisualHeight(),
                          Sections[GUI::BT_SUNKEN_EDGE_BOTTOM].first, NCColor,
                          rectEdge.size().x - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT].first.VisualWidth() - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.VisualWidth() );
      DrawTextureSection( X2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.VisualWidth(),
                          Y2 - Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.VisualHeight(),
                          Sections[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first, NCColor );
    }
  }
  else if ( VisualStyle & GUI::VFT_FLAT_BORDER )
  {
    // per Hand zeichnen
    GR::u32     color3DDarkShadow = GetSysColor( GUI::COL_3DDKSHADOW ); //Component.GetColor( GUI::COL_3DDKSHADOW )
    DrawQuad( X1, Y1, X2 - X1, 1, color3DDarkShadow );
    DrawQuad( X1, Y1 + 1, 1, Y2 - Y1 - 2, color3DDarkShadow );

    DrawQuad( X2 - 1, Y1 + 1, 1, Y2 - Y1 - 2, color3DDarkShadow );
    DrawQuad( X1, Y2 - 1, X2 - X1, 1, color3DDarkShadow );
  }
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
  XFont*    pFont = (XFont*)pFontA;
  if ( pFont == NULL )
  {
    return;
  }

  GR::tRect   rectText;
  
  // NOTE - THIS IS UGLY! // was NOT THE CORRECT CLIENTRECT! (not of the component to be drawn, but of the full displayer)
  m_pCurrentlyDrawnComponent->GetClientRect( rectText );

  if ( pRect != NULL )
  {
    rectText = *pRect;
  }

  m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

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
      GR::String&    strText = *it;

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
        GR::f32     scaleX = ( (GR::f32)m_PhysicalSize.x / m_VirtualSize.x );
        GR::f32     scaleY = ( (GR::f32)m_PhysicalSize.y / m_VirtualSize.y );

        if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
        {
          GR::tPoint    shadowPos( textPos );

          shadowPos.offset( 1, 1 );
          shadowPos = VirtualToScreen( shadowPos );
          m_pActualRenderer->RenderText2d( (XFont*)pFont, shadowPos.x, shadowPos.y, strText, scaleX, scaleY, 0xff000000 );
        }
        textPos = VirtualToScreen( textPos );
        m_pActualRenderer->RenderText2d( (XFont*)pFont, textPos.x, textPos.y, strText, scaleX, scaleY, Color );
      }
      else
      {
        if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
        {
          GR::tPoint    shadowPos( textPos );
          shadowPos.offset( 1, 1 );
          shadowPos = VirtualToScreen( shadowPos );
          m_pActualRenderer->RenderText2d( (XFont*)pFont, shadowPos.x, shadowPos.y, strText, 0xff000000 );
        }
        textPos = VirtualToScreen( textPos );
        m_pActualRenderer->RenderText2d( (XFont*)pFont, textPos.x, textPos.y, strText, Color );
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
    GR::f32     scaleX = ( (GR::f32)m_PhysicalSize.x / m_VirtualSize.x );
    GR::f32     scaleY = ( (GR::f32)m_PhysicalSize.y / m_VirtualSize.y );

    if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
    {
      m_pActualRenderer->RenderText2d( (XFont*)pFont, ptShadow.x, ptShadow.y, Text, scaleX, scaleY, 0xff000000 );
    }
    m_pActualRenderer->RenderText2d( (XFont*)pFont, ptText.x, ptText.y, Text, scaleX, scaleY, Color );
  }
  else
  {
    if ( TextAlignment & GUI::AF_SHADOW_OFFSET )
    {
      m_pActualRenderer->RenderText2d( (XFont*)pFont, ptShadow.x, ptShadow.y, Text, 0xff000000 );
    }
    m_pActualRenderer->RenderText2d( (XFont*)pFont, ptText.x, ptText.y, Text, Color );
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



void GUIComponentDisplayer::DrawFocusRect( const GUIComponent& Component, const GR::tRect& rc )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( Component.VisualStyle() & GUI::VFT_HIDE_FOCUS_RECT )
  {
    return;
  }

  GR::tRect     displayRect( rc );
  VirtualToScreen( displayRect );

  m_pActualRenderer->SetTexture( 0, NULL );
  m_pActualRenderer->RenderQuad2d( displayRect.Left,
                           displayRect.Top,
                           1,
                           displayRect.height(),
                           Component.GetColor( GUI::COL_BACKGROUND ) );
  m_pActualRenderer->RenderQuad2d( displayRect.Left,
                           displayRect.Top,
                           displayRect.width(),
                           1,
                           Component.GetColor( GUI::COL_BACKGROUND ) );
  m_pActualRenderer->RenderQuad2d( displayRect.Left + displayRect.width() - 1,
                           displayRect.Top,
                           1,
                           rc.height(),
                           Component.GetColor( GUI::COL_BACKGROUND ) );
  m_pActualRenderer->RenderQuad2d( displayRect.Left,
                           displayRect.Top + displayRect.height() - 1,
                           displayRect.width(),
                           1,
                           Component.GetColor( GUI::COL_BACKGROUND ) );
}



void GUIComponentDisplayer::DrawTextScaled( Interface::IFont* pFontA,
                                            int XOffset, int YOffset,
                                            const GR::String& Text,
                                            GR::u32 TextAlignment,
                                            GR::u32 Color,
                                            GR::tRect* pRect,
                                            GR::f32 ScaleFaktor )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  XFont*    pFont = (XFont*)pFontA;
  if ( pFont == NULL )
  {
    return;
  }

  GR::tRect   rectText;
  
  GetClientRect( rectText );

  if ( pRect != NULL )
  {
    rectText = *pRect;
  }

  m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( TextAlignment & GUI::AF_MULTILINE )
  {
    std::vector<GR::String>    vectText;

    GUI::WrapText( pFont, Text, rectText, vectText );

    int   iY = rectText.position().y;

    if ( ( TextAlignment & GUI::AF_VCENTER ) == GUI::AF_VCENTER )
    {
      iY += (int)( ( ( rectText.height() - 20 * (int)vectText.size() ) / 2 ) * ScaleFaktor );
    }
    else if ( TextAlignment & GUI::AF_BOTTOM )
    {
      iY = (int)( rectText.Bottom - ( 20 * (int)vectText.size() ) * ScaleFaktor );
    }

    std::vector<GR::String>::iterator    it( vectText.begin() );
    while ( it != vectText.end() )
    {
      GR::String&    strText = *it;

      int     iX = rectText.Left;

      int     iLength = pFont->TextLength( strText );

      if ( TextAlignment & GUI::AF_CENTER )
      {
        iX += (int)( ( ( rectText.width() - iLength ) / 2 ) * ScaleFaktor );
      }
      else if ( TextAlignment & GUI::AF_RIGHT )
      {
        iX = (int)( rectText.Right - iLength * ScaleFaktor );
      }

      m_pActualRenderer->RenderText2d( pFont, XOffset + iX, YOffset + iY, strText, ScaleFaktor, ScaleFaktor, Color );
      ++it;
      iY += (int)( ( pFont->TextHeight() ) * ScaleFaktor );
    }
    return;
  }

  GR::tPoint    ptText = GUI::TextOffset( pFont, Text, TextAlignment, rectText );

  m_pActualRenderer->RenderText2d( pFont, XOffset + ptText.x, YOffset + ptText.y, Text, ScaleFaktor, ScaleFaktor, Color );
}



void GUIComponentDisplayer::DrawTextureSection( int X, int Y, const XTextureSection& TexSection, GR::u32 Color, int AlternativeWidth, int AlternativeHeight, GR::u32 AlternativeFlags )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( m_VirtualSize.x == 0 )
  {
    m_pActualRenderer->RenderTextureSection2d( X, Y, TexSection, Color, AlternativeWidth, AlternativeHeight, AlternativeFlags, 0.000000f );
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

    m_pActualRenderer->RenderTextureSection2d( alternativePos.x, alternativePos.y, TexSection, Color, alternativeEndPos.x - alternativePos.x, alternativeEndPos.y - alternativePos.y, AlternativeFlags, 0.000000f );
  }
}



void GUIComponentDisplayer::DrawTiled( int OffsetX, int OffsetY, const GR::tRect& Rect, XTextureSection Section, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  int     xRest = Rect.width() % Section.m_Width;
  int     yRest = Rect.height() % Section.m_Height;

  for ( int i = 0; i < Rect.width() / Section.m_Width; ++i )
  {
    for ( int j = 0; j < Rect.height() / Section.m_Height; ++j )
    {
      m_pActualRenderer->RenderTextureSection2d( OffsetX + Rect.Left + i * Section.m_Width,
                                       OffsetY + Rect.Top + j * Section.m_Height,
                                       Section,
                                       Color );
      if ( xRest != 0 )
      {
        XTextureSection   xSection( Section.m_pTexture, Section.m_XOffset, Section.m_YOffset, xRest, Section.m_Height, Section.m_Flags );
        m_pActualRenderer->RenderTextureSection2d( OffsetX + Rect.Left + ( Rect.width() / Section.m_Width ) * Section.m_Width,
                                           OffsetY + Rect.Top + j * Section.m_Height,
                                           xSection,
                                           Color );
      }
    }
    if ( yRest != 0 )
    {
      XTextureSection   ySection( Section.m_pTexture, Section.m_XOffset, Section.m_YOffset, Section.m_Width, yRest, Section.m_Flags );
      m_pActualRenderer->RenderTextureSection2d( OffsetX + Rect.Left + i * Section.m_Width,
                                       OffsetY + Rect.Top + ( Rect.height() / Section.m_Height ) * Section.m_Height,
                                       ySection,
                                       Color );
    }
  }
  if ( ( xRest != 0 )
  &&   ( yRest != 0 ) )
  {
    XTextureSection   restSection( Section.m_pTexture, Section.m_XOffset, Section.m_YOffset, xRest, yRest, Section.m_Flags );
    m_pActualRenderer->RenderTextureSection2d( OffsetX + Rect.Left + ( Rect.width() / Section.m_Width ) * Section.m_Width,
                                     OffsetY + Rect.Top + ( Rect.height() / Section.m_Height ) * Section.m_Height,
                                     restSection,
                                     Color );
  }
}



void GUIComponentDisplayer::DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  while ( Width > 0 )
  {
    if ( Section.VisualWidth() <= Width )
    {
      m_pActualRenderer->RenderTextureSection2d( X, Y, Section, Color );
      Width -= Section.VisualWidth();
      X += Section.VisualWidth();
    }
    else
    {
      // only need part of the section
      XTextureSection   partSection( Section );

      if ( ( ( partSection.m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_90 )
      ||   ( ( partSection.m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_270 ) )
      {
        partSection.m_Height = Width;
      }
      else
      {
        partSection.m_Width = Width;
      }
      m_pActualRenderer->RenderTextureSection2d( X, Y, partSection, Color );
      return;
    }
  }
}



void GUIComponentDisplayer::DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  while ( Height > 0 )
  {
    if ( Section.VisualHeight() <= Height )
    {
      m_pActualRenderer->RenderTextureSection2d( X, Y, Section, Color );
      Height -= Section.VisualHeight();
      Y += Section.VisualHeight();
    }
    else
    {
      // only need part of the section
      XTextureSection   partSection( Section );

      if ( ( ( partSection.m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_90 )
      ||   ( ( partSection.m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_270 ) )
      {
        partSection.m_Width = Height;
      }
      else
      {
        partSection.m_Height = Height;
      }
      m_pActualRenderer->RenderTextureSection2d( X, Y, partSection, Color );
      return;
    }
  }
}



void GUIComponentDisplayer::DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  if ( ( Width <= 0 )
  ||   ( Height <= 0 )
  ||   ( Section.m_Width <= 0 )
  ||   ( Section.m_Height <= 0 ) )
  {
    return;
  }

  while ( Height > 0 )
  {
    int     width = Width;

    int     heightToUse = Section.VisualHeight();
    if ( Height < heightToUse )
    {
      heightToUse = Height;
    }

    int x = X;
    while ( width > 0 )
    {
      XTextureSection   partSection( Section );

      int     widthToUse = width;
      if ( Section.VisualWidth() < widthToUse )
      {
        widthToUse = Section.VisualWidth();
      }

      if ( ( ( partSection.m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_90 )
      ||   ( ( partSection.m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_270 ) )
      {
        partSection.m_Height = widthToUse;
        partSection.m_Width = heightToUse;
      }
      else
      {
        partSection.m_Width = widthToUse;
        partSection.m_Height = heightToUse;
      }
      m_pActualRenderer->RenderTextureSection2d( x, Y, partSection, Color );
      width -= widthToUse;
      x += widthToUse;
    }

    Height -= heightToUse;
    Y += heightToUse;
  }
}



void GUIComponentDisplayer::DrawRect( int X, int Y, int Width, int Height, GR::u32 Color )
{
  DrawRect( GR::tPoint( X, Y ), GR::tPoint( Width, Height ), Color );
}



void GUIComponentDisplayer::DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  m_pActualRenderer->SetTexture( 0, NULL );

  GR::tPoint    virtPos = VirtualToScreen( Pos );
  GR::tPoint    endPos = Pos + Size;
  endPos = VirtualToScreen( endPos );

  m_pActualRenderer->RenderRect2d( virtPos, endPos - virtPos, Color );
}



void GUIComponentDisplayer::DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 )
{
  if ( m_pActualRenderer == NULL )
  {
    return;
  }
  m_pActualRenderer->SetTexture( 0, NULL );
  m_pActualRenderer->RenderLine2d( VirtualToScreen( Pos1 ), 
                           VirtualToScreen( Pos2 ), 
                           Color, Color2 );
}



int GUIComponentDisplayer::FontSpacing( Interface::IFont* pFontArg )
{
  XFont*    pFont = (XFont*)pFontArg;
  if ( pFont == NULL )
  {
    return 0;
  }
  return pFont->FontSpacing();
}



int GUIComponentDisplayer::TextHeight( Interface::IFont* pFontArg )
{
  XFont*    pFont = (XFont*)pFontArg;
  if ( pFont == NULL )
  {
    return 0;
  }
  return pFont->TextHeight();
}



int GUIComponentDisplayer::TextHeight( Interface::IFont* pFontArg, const GR::String& Text )
{
  XFont*    pFont = (XFont*)pFontArg;
  if ( pFont == NULL )
  {
    return 0;
  }
  return pFont->TextHeight( Text );
}



int GUIComponentDisplayer::TextLength( Interface::IFont* pFontArg, const GR::String& Text )
{
  XFont*    pFont = (XFont*)pFontArg;
  if ( pFont == NULL )
  {
    return 0;
  }
  return pFont->TextLength( Text );
}



void GUIComponentDisplayer::SetRenderer( XRenderer* pRenderer )
{
  m_pActualRenderer = pRenderer;
  m_pRenderer       = pRenderer;
}