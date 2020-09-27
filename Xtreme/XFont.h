#ifndef XFONT_H_
#define XFONT_H_



#include <Interface/IFont.h>
#include <Interface/IService.h>
#include <Interface/IString.h>

#include <GR/GRTypes.h>

#include <map>
#include <string>


#include <Xtreme/XTextureSection.h>



class XRenderer;
class XTexture;

namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}



#undef DrawText

class XFont : public Interface::IFont
{

  public:

    enum eFontLoadFlags
    {
      FLF_ALPHA_BIT         = 0x00000002,   // soll mind. 1 bit Alpha haben (für Alphatest)
      FLF_ALPHA_FULL        = 0x00000004,   // volles Alpha drin
      FLF_SQUARED           = 0x00000010,   // Quadr. Font-Texture, (2 Fonts in 16x8-Anordnung)
      FLF_SQUARED_ONE_FONT  = 0x00000020,   // Quadr. Font-Texture, 16x16
    };

    XFont( XRenderer* = NULL, GR::IEnvironment* = NULL )
    {
    }
    virtual ~XFont()
    {
    }

    virtual int                       TextLength( const GR::Char* Text ) const = 0;
    virtual int                       TextLength( const GR::String& Text ) const = 0;
    virtual int                       TextHeight( const GR::Char* Text = NULL ) const = 0;
    virtual int                       TextHeight( const GR::String& Text ) const = 0;
    virtual int                       FontSpacing() = 0;

    virtual void                      Recreate() = 0;
    virtual void                      Release() = 0;

    virtual bool                      LoadFont( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor ) = 0;
    virtual bool                      LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor ) = 0;
    virtual bool                      LoadFontSquare( const char* FileName, GR::u32 Flags = FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual bool                      LoadFontSquare( const GR::String& FileName, GR::u32 Flags = FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual void                      FontFromTexture( GR::u32 Flags, XTexture* pTexture, GR::Graphic::ImageData* pImageData ) = 0;

    virtual void                      Dump() = 0;

    virtual float                     DrawLetterDetail( float iX, float iY, 
                                                GR::u32 cLetter, 
                                                float fScaleX, float fScaleY, 
                                                GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4, float fZ = 0.0f ) = 0;
    virtual int                       DrawLetter( int iX, int iY, GR::u32 cLetter, GR::u32 Color, float fZ = 0.0f ) = 0;

    virtual void                      DrawText( int iX, int iY, const GR::String& strText, GR::u32 Color, float fZ = 0.0f ) = 0;
    virtual void                      DrawText( int iX, int iY, const GR::String& strText, float fScaleX, float fScaleY, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;
    virtual void                      DrawFreeText( const GR::tVector& vectPos, 
                                                    const GR::String& strText, 
                                                    const GR::tVector& vectScale, 
                                                    GR::u32 Color ) = 0;

};




#endif // XFONT_H_