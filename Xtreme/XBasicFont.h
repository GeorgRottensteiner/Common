#ifndef XBASICFONT_H_
#define XBASICFONT_H_



#include <Interface/IFont.h>

#include <GR/GRTypes.h>

#include <map>
#include <string>


#include <Xtreme/XFont.h>



class XRenderer;
class XTexture;

#undef DrawText


class XBasicFont : public XFont
{

  public:

    class XLetter
    {
      public:

        XTextureSection     TextureSection;

        int                 XOffset;


        XLetter() :
          XOffset( 0 )
        {
        }
    };


    enum eFontLoadFlags
    {
      FLF_ALPHA_BIT         = 0x00000002,   // soll mind. 1 bit Alpha haben (für Alphatest)
      FLF_ALPHA_FULL        = 0x00000004,   // volles Alpha drin
      FLF_SQUARED           = 0x00000010,   // Quadr. Font-Texture, (2 Fonts in 16x8-Anordnung)
      FLF_SQUARED_ONE_FONT  = 0x00000020,   // Quadr. Font-Texture, 16x16
    };

    XRenderer*                        m_pRenderer;

    std::map<GR::u32,XLetter*>        m_Letters;

    GR::String                       m_FileName;

    int                               m_Width,
                                      m_Height;

    GR::u32                           m_CreationFlags,
                                      m_TransparentColor;

    GR::IEnvironment*                 m_pEnvironment;



    XBasicFont( XRenderer* pRenderer = NULL, GR::IEnvironment* pEnvironment = NULL );
    virtual ~XBasicFont();

    virtual int                       TextLength( const GR::Char* Text ) const;
    virtual int                       TextLength( const GR::String& Text ) const;
    virtual int                       TextHeight( const GR::Char* Text = NULL ) const;
    virtual int                       TextHeight( const GR::String& Text ) const;
    virtual int                       FontSpacing();

    XLetter*                          Letter( const char cLetter );
    void                              SetLetter( const char Letter, XLetter* pLetter );

    void                              Recreate();
    void                              Release();

    bool                              LoadFont( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor );
    bool                              LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor );
    bool                              LoadFontSquare( const char* FileName, GR::u32 Flags = FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    bool                              LoadFontSquare( const GR::String& FileName, GR::u32 Flags = FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    void                              FontFromTexture( GR::u32 Flags, XTexture* pTexture, GR::Graphic::ImageData* pImageData );

    void                              Dump();

    float                             DrawLetterDetail( float X, float Y,
                                                        GR::u32 cLetter,
                                                        float ScaleX, float ScaleY,
                                                        GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4, float Z = 0.0f );
    int                               DrawLetter( int X, int Y, GR::u32 cLetter, GR::u32 Color, float Z = 0.0f );

    void                              DrawText( int X, int Y, const GR::String& Text, GR::u32 Color, float Z = 0.0f );
    void                              DrawText( int X, int Y, const GR::String& Text, float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f );
    void                              DrawFreeText( const GR::tVector& Pos,
                                                    const GR::String& Text,
                                                    const GR::tVector& Scale,
                                                    GR::u32 Color );

};




#endif // XFONT_H_