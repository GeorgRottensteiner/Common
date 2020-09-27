#pragma once

#include <Interface/IScalingRenderer.h>



class XRenderer;

class XScalingRenderer : public IScalingRenderer
{
  protected:

    XRenderer*      m_pRenderer;

    GR::tPoint      m_PhysicalSize;
    GR::tPoint      m_VirtualSize;


    void            ScreenToVirtual( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height );
    GR::tPoint      ScreenToVirtual( const GR::tPoint& ScreenCoordinates );
    GR::tPoint      VirtualToScreen( const GR::tPoint& VirtualCoordinates );
    void            VirtualToScreen( GR::tRect& VirtualCoordinates );
    void            VirtualToScreen( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height );
    


  public:

    XScalingRenderer( XRenderer& Renderer );


    virtual void DrawText( Interface::IFont* pFont,
                           int X, int Y,
                           const GR::String& Text,
                           GR::u32 Color = 0xffffffff );

    virtual void DrawTextureSection( int X, int Y,
                                     const XTextureSection& TexSection,
                                     GR::u32 Color = -1,
                                     int AlternativeWidth = -1,
                                     int AlternativeHeight = -1,
                                     GR::u32 AlternativeFlags = -1 );

    virtual void DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 );
    virtual void DrawRectangle( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color );
    virtual void DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 = 0 );


    virtual void SetScreenSizes( const GR::tPoint& PhysicalSize, const GR::tPoint& VirtualSize = GR::tPoint() );
    virtual GR::tPoint VirtualSize() const;
    virtual GR::tPoint PhysicalSize() const;

};
