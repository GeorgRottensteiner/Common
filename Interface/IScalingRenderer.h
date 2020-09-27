#pragma once

#include <GR/GRTypes.h>



namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}



namespace Interface
{
  struct IFont;
}


struct XTextureSection;


class IScalingRenderer
{

  public:

    virtual ~IScalingRenderer()
    {
    }


    virtual void DrawText( Interface::IFont* pFont,
                           int X, int Y,
                           const GR::String& Text,
                           GR::u32 Color = 0xffffffff ) = 0;

    virtual void DrawTextureSection( int X, int Y,
                                     const XTextureSection& TexSection,
                                     GR::u32 Color = -1,
                                     int AlternativeWidth = -1,
                                     int AlternativeHeight = -1,
                                     GR::u32 AlternativeFlags = -1 ) = 0;

    virtual void DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;
    virtual void DrawRectangle( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color ) = 0;
    virtual void DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 = 0 ) = 0;


    virtual void SetScreenSizes( const GR::tPoint& PhysicalSize, const GR::tPoint& VirtualSize = GR::tPoint() ) = 0;
    virtual GR::tPoint VirtualSize() const = 0;
    virtual GR::tPoint PhysicalSize() const = 0;

};
