#ifndef X2dFont_H_
#define X2dFont_H_



#include <Interface/IFont.h>
#include <Interface/IService.h>

#include <GR/GRTypes.h>

#include <Xtreme/XTextureSection.h>

#include <map>
#include <string>
#include <vector>



#if OPERATING_SYSTEM == OS_WINDOWS
#undef DrawText
#endif


class X2dRenderer;
class XTexture;

namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}



class X2dFont : public Interface::IFont
{

  public:

    class XLetter
    {
      public:

        XTextureSection     m_TextureSection;

        int                 m_iWidth,
                            m_iXOffset;


        XLetter()
        {
        }


        virtual ~XLetter();

    };


    enum eFontLoadFlags
    {
      FLF_ALPHA_BIT         = 0x00000002,   // soll mind. 1 bit Alpha haben (für Alphatest)
      FLF_ALPHA_FULL        = 0x00000004,   // volles Alpha drin
      FLF_SQUARED           = 0x00000010,   // Quadr. Font-Texture, (2 Fonts in 16x8-Anordnung)
      FLF_SQUARED_ONE_FONT  = 0x00000020,   // Quadr. Font-Texture, 16x16
    };

    X2dRenderer*                      m_pRenderer;

    std::map<GR::u32,XLetter*>        m_mapLetters;

    GR::String                       m_strFileName;

    int                               m_iWidth,
                                      m_iHeight;

    GR::u32                           m_CreationFlags,
                                      m_TransparentColor;

    GR::IEnvironment*                 m_pEnvironment;


    X2dFont( X2dRenderer* pRenderer = NULL, GR::IEnvironment* pEnvironment = NULL );
    virtual ~X2dFont();

    virtual int                       TextLength( const GR::String& Text ) const;
    virtual int                       TextHeight( const GR::String& Text = GR::String() ) const;
    virtual int                       FontSpacing();

    XLetter*                          Letter( const char cLetter );

    void                              Recreate();
    void                              Release();

    bool                              LoadFont( const GR::String& szFileName, GR::u32 Flags, GR::u32 TransparentColor );
    bool                              LoadFontSquare( const GR::String& szFileName, GR::u32 Flags = FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    void                              FontFromTexture( GR::u32 Flags, XTexture* pTexture, GR::Graphic::ImageData* pImageData );

    void                              Dump();

    float                             DrawLetterDetail( int iX, int iY,
                                                        GR::u32 cLetter,
                                                        float fScaleX, float fScaleY,
                                                        GR::u32 Color );
    int                               DrawLetter( int iX, int iY, GR::u32 cLetter, GR::u32 Color );

    void                              DrawText( int iX, int iY, const GR::String& strText, GR::u32 Color );
    void                              DrawText( int iX, int iY, const GR::String& strText, float fScaleX, float fScaleY, GR::u32 Color );

    void                              WrapText( const GR::String& strText, GR::tRect& rectText, std::vector<GR::String>& vectText ) const;

    GR::Graphic::eImageFormat         FontImageFormat() const;

};




#endif // X2dFont_H_