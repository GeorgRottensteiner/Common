#ifndef FONT_H
#define FONT_H



#include <map>

#include <Grafik/Image.h>
#include <Interface/IFont.h>

#include <GR/GRTypes.h>



namespace GR
{

  class Font : public Interface::IFont
  {

    protected:

      enum
      {
        FONT_BORDER_SIZE = 1
      };

      struct tLetterInfo
      {
        int                   LetterXOffset,
                              LetterYOffset;
        GR::Graphic::Image*   pImage;

        tLetterInfo() :
          LetterXOffset( 0 ),
          LetterYOffset( 0 ),
          pImage( NULL )
        {
        }
      };

      typedef std::map<GR::u32,tLetterInfo>   tLetters;

      tLetters          m_Letters;

      bool              m_FixedWidth;

      int               m_Height,
                        m_LetterWidth,
                        m_BaseLineOffsetY,
                        m_FontSpacing;


      void              Set( int NewWidth, int NewHeight );


    public:

      Font( bool FixedWidth = false );
      Font( unsigned long NewWidth, unsigned long NewHeight );

      ~Font();

      void              Reset();

      bool              Load( const GR::String& FileName );

      bool              Save( const GR::String& FileName );

      bool              LoadFNT( const GR::String& FileName, unsigned char Bpp, GR::u32 Transparent = 0 );

      bool              LoadFNT( IIOStream& Stream, unsigned char Bpp );

      GR::Graphic::Image*       GetLetter( GR::u32 Letter ) const;
      GR::u32           GetDepth() const;

      void              SetLetter( GR::u32 Letter, GR::Graphic::Image* pLetter );

      virtual int       TextLength( const GR::Char* ) const;
      virtual int       TextLength( const GR::String& ) const;

      virtual int       TextHeight( const GR::String& = GR::String() ) const;  // "" gibt die Standardhöhe

      virtual int       FontSpacing();

      void              Compress( GR::u32 TransparentColor );
      bool              Convert( unsigned char Bpp, GR::Graphic::Palette* pPalette = NULL );


      bool              PrintFont( GR::Graphic::GFXPage* pActualPage, signed long XPos, signed long YPos, const GR::Char* Text, unsigned long Flags = IMAGE_METHOD_OPTIMAL, GR::Graphic::Image* pMaskImage = NULL ) const;
      bool              PrintFont( GR::Graphic::GFXPage* pActualPage, signed long XPos, signed long YPos, const GR::String& Text, unsigned long Flags = IMAGE_METHOD_OPTIMAL, GR::Graphic::Image* pMaskImage = NULL ) const;

  };

}


#endif // FONT_H



