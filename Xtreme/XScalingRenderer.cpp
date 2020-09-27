#pragma once

#include "XScalingRenderer.h"

#include <Xtreme/XRenderer.h>


#undef DrawText



XScalingRenderer::XScalingRenderer( XRenderer& Renderer ) :
  m_pRenderer( &Renderer )
{
}



void XScalingRenderer::ScreenToVirtual( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height )
{
  if ( ( m_VirtualSize.x == 0 )
  ||   ( m_PhysicalSize.x == 0 ) )
  {
    return;
  }
  int     newX = ( X * m_VirtualSize.x ) / m_PhysicalSize.x;
  int     newY = ( Y * m_VirtualSize.y ) / m_PhysicalSize.y;
  Width = ( ( X + Width ) * m_VirtualSize.x ) / m_PhysicalSize.x - newX;
  Height = ( ( Y + Height ) * m_VirtualSize.y ) / m_PhysicalSize.y - newY;

  X = newX;
  Y = newY;
}



GR::tPoint XScalingRenderer::ScreenToVirtual( const GR::tPoint& ScreenCoordinates )
{
  if ( ( m_VirtualSize.x == 0 )
  ||   ( m_PhysicalSize.x == 0 ) )
  {
    return ScreenCoordinates;
  }
  return GR::tPoint( ( ScreenCoordinates.x * m_VirtualSize.x ) / m_PhysicalSize.x,
    ( ScreenCoordinates.y * m_VirtualSize.y ) / m_PhysicalSize.y );
}



GR::tPoint XScalingRenderer::VirtualToScreen( const GR::tPoint& VirtualCoordinates )
{
  if ( ( m_VirtualSize.x == 0 )
  ||   ( m_PhysicalSize.x == 0 ) )
  {
    return VirtualCoordinates;
  }
  return GR::tPoint( ( VirtualCoordinates.x * m_PhysicalSize.x ) / m_VirtualSize.x,
                     ( VirtualCoordinates.y * m_PhysicalSize.y ) / m_VirtualSize.y );
}



void XScalingRenderer::VirtualToScreen( GR::tRect& VirtualCoordinates )
{
  if ( ( m_VirtualSize.x == 0 )
  ||   ( m_PhysicalSize.x == 0 ) )
  {
    return;
  }

  int     newX = ( VirtualCoordinates.Left * m_PhysicalSize.x ) / m_VirtualSize.x;
  int     newY = ( VirtualCoordinates.Top * m_PhysicalSize.y ) / m_VirtualSize.y;
  int     width = ( VirtualCoordinates.Right * m_PhysicalSize.x ) / m_VirtualSize.x - newX;
  int     height = ( VirtualCoordinates.Bottom * m_PhysicalSize.y ) / m_VirtualSize.y - newY;

  VirtualCoordinates.Left = newX;
  VirtualCoordinates.Top = newY;
  VirtualCoordinates.Right = newX + width;
  VirtualCoordinates.Bottom = newY + height;
}



void XScalingRenderer::VirtualToScreen( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height )
{
  if ( ( m_VirtualSize.x == 0 )
  ||   ( m_PhysicalSize.x == 0 ) )
  {
    return;
  }
  int     newX = ( X * m_PhysicalSize.x ) / m_VirtualSize.x;
  int     newY = ( Y * m_PhysicalSize.y ) / m_VirtualSize.y;
  Width = ( ( X + Width ) * m_PhysicalSize.x ) / m_VirtualSize.x - newX;
  Height = ( ( Y + Height ) * m_PhysicalSize.y ) / m_VirtualSize.y - newY;

  X = newX;
  Y = newY;
}



void XScalingRenderer::DrawRectangle( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color )
{
  if ( m_pRenderer == NULL )
  {
    return;
  }
  m_pRenderer->SetTexture( 0, NULL );

  GR::tPoint    virtPos = VirtualToScreen( Pos );
  GR::tPoint    endPos = Pos + Size;
  endPos = VirtualToScreen( endPos );

  m_pRenderer->RenderRect2d( virtPos, endPos - virtPos, Color );
}



void XScalingRenderer::DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 )
{
  if ( m_pRenderer == NULL )
  {
    return;
  }
  m_pRenderer->SetTexture( 0, NULL );
  m_pRenderer->RenderLine2d( VirtualToScreen( Pos1 ),
                             VirtualToScreen( Pos2 ),
                             Color, Color2 );
}



void XScalingRenderer::SetScreenSizes( const GR::tPoint& PhysicalSize, const GR::tPoint& VirtualSize )
{
  m_VirtualSize = VirtualSize;
  m_PhysicalSize = PhysicalSize;
}



GR::tPoint XScalingRenderer::VirtualSize() const
{
  return m_VirtualSize;
}



GR::tPoint XScalingRenderer::PhysicalSize() const
{
  return m_PhysicalSize;
}



void XScalingRenderer::DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4 )
{
  if ( m_pRenderer == NULL )
  {
    return;
  }
  VirtualToScreen( X, Y, Width, Height );

  m_pRenderer->SetTexture( 0, NULL );
  m_pRenderer->RenderQuad2d( X, Y, Width, Height, Color, Color2, Color3, Color4 );
}



void XScalingRenderer::DrawText( Interface::IFont* pFontA,
                                 int X, int Y,
                                 const GR::String& Text,
                                 GR::u32 Color )
{
  if ( m_pRenderer == NULL )
  {
    return;
  }
  XFont*    pFont = (XFont*)pFontA;
  if ( pFont == NULL )
  {
    return;
  }

  m_pRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( m_VirtualSize.x != 0 )
  {
    GR::f32     scaleX = ( ( GR::f32 )m_PhysicalSize.x / m_VirtualSize.x );
    GR::f32     scaleY = ( ( GR::f32 )m_PhysicalSize.y / m_VirtualSize.y );

    m_pRenderer->RenderText2d( (XFont*)pFont, X, Y, Text, scaleX, scaleY, Color );
  }
  else
  {
    m_pRenderer->RenderText2d( (XFont*)pFont, X, Y, Text, Color );
  }
}



void XScalingRenderer::DrawTextureSection( int X, int Y, const XTextureSection& TexSection, GR::u32 Color, int AlternativeWidth, int AlternativeHeight, GR::u32 AlternativeFlags )
{
  if ( m_pRenderer == NULL )
  {
    return;
  }
  if ( m_VirtualSize.x == 0 )
  {
    m_pRenderer->RenderTextureSection2d( X, Y, TexSection, Color, AlternativeWidth, AlternativeHeight, AlternativeFlags, 0.000000f );
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

    m_pRenderer->RenderTextureSection2d( alternativePos.x, alternativePos.y, TexSection, Color, alternativeEndPos.x - alternativePos.x, alternativeEndPos.y - alternativePos.y, AlternativeFlags, 0.000000f );
  }
}



