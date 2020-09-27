#include <Grafik/Font.h>
#include <Grafik/Image.h>

#include <Misc/Misc.h>

#include <Interface/IIOStream.h>

#include <IO/FileStream.h>

#include <Grafik/ImageData.h>

#include <String/StringUtil.h>

#include <debug/debugclient.h>



namespace GR
{

  Font::Font( bool FixedWidth ) :
    m_FontSpacing( 1 ),
    m_BaseLineOffsetY( 0 )
  {
    for ( int i = 0; i < 256; i++ )
    {
      m_Letters[(GR::u32)i] = tLetterInfo();
    }
    Set( 0, 0 );
    m_FixedWidth   = FixedWidth;
  }



  Font::Font( unsigned long NewWidth, unsigned long NewHeight ) :
    m_FontSpacing( 1 ),
    m_BaseLineOffsetY( 0 ),
    m_FixedWidth( false )
  {
    for ( int i = 0; i < 256; i++ )
    {
      m_Letters[(GR::u32)i] = tLetterInfo();
    }
    Set( NewWidth, NewHeight );
  }



  Font::~Font()
  {
    Reset();
  }



  void Font::Set( int NewWidth, int NewHeight )
  {
    Reset();
    m_FixedWidth   = true;

    m_Height       = NewHeight;
    m_LetterWidth  = NewWidth;
    for ( int i = 0; i < 256; i++ )
    {
      m_Letters[(GR::u32)i] = tLetterInfo();
    }
  }



  void Font::Reset()
  {
    m_FixedWidth   = true;
    m_Height       = 0;
    m_LetterWidth  = 0;

    tLetters::iterator   it( m_Letters.begin() );
    while ( it != m_Letters.end() )
    {
      tLetterInfo&    Letter = it->second;

      delete Letter.pImage;

      ++it;
    }
    m_Letters.clear();
  }



  bool Font::Load( const GR::Char* Filename )
  {
    GR::IO::FileStream       fileFont;

    if ( !fileFont.Open( Filename, IIOStream::OT_READ_ONLY ) )
    {
      dh::Log( "Font::Load File not found %s", Filename );
      return false;
    }

    if ( fileFont.ReadU8() != GR::Graphic::IGFType::FONT )
    {
      dh::Log( "Font::Load not a valid font %s", Filename );
      return false;
    }

    GR::Graphic::eImageFormat format = (GR::Graphic::eImageFormat)fileFont.ReadU8();

    size_t      charCount = (size_t)fileFont.ReadU32();

    GR::u32     chunk = 0;


    while ( chunk = fileFont.ReadU32() )
    {
      if ( chunk == 0x0001 )
      {
        // Image-Chunk
        for ( size_t i = 0; i < charCount; ++i )
        {
          // Zeichen
          GR::u32     dwChar = fileFont.ReadU32();

          GR::u32     dwWidth = fileFont.ReadU32();
          GR::u32     dwHeight = fileFont.ReadU32();

          tLetterInfo&    LetterInfo = m_Letters[dwChar];

          if ( LetterInfo.pImage )
          {
            delete LetterInfo.pImage;
          }
          unsigned char   ucDepth = (unsigned char)GR::Graphic::ImageData::DepthFromImageFormat( format );
          if ( format == GR::Graphic::IF_X1R5G5B5 )
          {
            ucDepth = 15;
          }
          LetterInfo.pImage = new GR::Graphic::Image( dwWidth, dwHeight, ucDepth );
          LetterInfo.LetterXOffset = 0;
          LetterInfo.LetterYOffset = 0;

          fileFont.ReadBlock( LetterInfo.pImage->GetData(), 
                              LetterInfo.pImage->GetSize() );
        }
      }
    }

    fileFont.Close();
    return true;
  }



  bool Font::Save( const GR::Char* Filename )
  {
    GR::IO::FileStream       fileFont;

    if ( !fileFont.Open( Filename, IIOStream::OT_WRITE_ONLY ) )
    {
      dh::Log( "Font::Save File not found %s", Filename );
      return false;
    }

    fileFont.WriteU8( GR::Graphic::IGFType::FONT );

    if ( m_Letters.empty() )
    {
      fileFont.WriteU8( GR::Graphic::IF_UNKNOWN );
    }
    else
    {
      bool    imageFound = false;

      tLetters::iterator   it( m_Letters.begin() );
      while ( it != m_Letters.end() )
      {
        tLetterInfo& LetterInfo = it->second;

        if ( LetterInfo.pImage )
        {
          fileFont.WriteU8( GR::Graphic::ImageData::ImageFormatFromDepth( LetterInfo.pImage->GetDepth() ) );
          imageFound = true;
          break;
        }
        ++it;
      }
      if ( !imageFound )
      {
        fileFont.WriteU8( GR::Graphic::IF_UNKNOWN );
      }
    }

    // Char Count
    GR::u32     count = 0;

    tLetters::iterator   it( m_Letters.begin() );
    while ( it != m_Letters.end() )
    {
      tLetterInfo& LetterInfo = it->second;

      if ( LetterInfo.pImage )
      {
        ++count;
      }
      ++it;
    }

    fileFont.WriteU32( count );

    // Image-Chunk
    fileFont.WriteU32( 0x0001 );
    it = m_Letters.begin();
    while ( it != m_Letters.end() )
    {
      tLetterInfo& LetterInfo = it->second;

      if ( LetterInfo.pImage )
      {
        // char
        fileFont.WriteU32( it->first );

        // Image
        fileFont.WriteU32( LetterInfo.pImage->GetWidth() );
        fileFont.WriteU32( LetterInfo.pImage->GetHeight() );

        fileFont.WriteBlock( LetterInfo.pImage->GetData(), 
                            LetterInfo.pImage->GetSize() );
      }

      ++it;
    }
    fileFont.Close();
    return true;
  }



  bool Font::LoadFNT( const GR::Char* FileName, unsigned char Bpp, GR::u32 Transparent )
  {
    GR::IO::FileStream    ioIn;

    if ( !ioIn.Open( FileName ) )
    {
      dh::Log( "Font::LoadFNT File not found %s", FileName );
      return false;
    }
    unsigned char   ucBuffer[32];

    unsigned long   count       = 0,
                    i;


    Reset();

    ioIn.ReadBlock( ucBuffer, 32 );
    count = ucBuffer[23] + (unsigned long)( ucBuffer[24] * 256 );
    m_LetterWidth = 0;
    m_Height      = 0;

    for ( i = 0; i < count; i++ )
    {
      tLetterInfo&    Letter = m_Letters[(GR::u32)i];

      Letter.pImage = new GR::Graphic::Image();
      Letter.pImage->LoadBTNAt( ioIn, Bpp );
      if ( Letter.pImage->GetWidth() > m_LetterWidth )
      {
        m_LetterWidth = Letter.pImage->GetWidth();
      }
      if ( Letter.pImage->GetHeight() > m_Height )
      {
        m_Height = Letter.pImage->GetHeight();
      }
      Letter.pImage->SetTransparentColorRGB( Transparent );
      Letter.pImage->Compress();
    }
    m_BaseLineOffsetY  = 0;
    m_FixedWidth       = false;

    ioIn.Close();
    return true;
  }



  bool Font::LoadFNT( IIOStream& Stream, unsigned char Bpp )
  {
    if ( !Stream.IsGood() )
    {
      return false;
    }

    unsigned char   ucBuffer[32];

    unsigned long   count = 0,
                    i;

    Stream.ReadBlock( ucBuffer, 32 );

    Reset();

    count = ucBuffer[23] + (unsigned long)( ucBuffer[24] * 256 );
    m_LetterWidth = 0;
    m_Height      = 0;

    for ( i = 0; i < count; i++ )
    {
      tLetterInfo&    Letter = m_Letters[(GR::u32)i];

      WORD    width  = Stream.ReadU16();
      WORD    height = Stream.ReadU16();
    
      unsigned char bppTemp = Bpp;
    
      if ( bppTemp == 16 )
      {
        bppTemp = 15;
      }

      Letter.pImage = new GR::Graphic::Image( width, height, bppTemp );
      Stream.ReadBlock( Letter.pImage->GetData(), Letter.pImage->GetSize() );

      if ( Letter.pImage->GetWidth() > m_LetterWidth )
      {
        m_LetterWidth = Letter.pImage->GetWidth();
      }
      if ( Letter.pImage->GetHeight() > m_Height )
      {
        m_Height = Letter.pImage->GetHeight();
      }
    }
    m_FixedWidth = false;

    Stream.Close();

    return true;
  }



  GR::Graphic::Image* Font::GetLetter( GR::u32 Letter ) const
  {
    tLetters::const_iterator    it( m_Letters.find( Letter ) );
    if ( it == m_Letters.end() )
    {
      return NULL;
    }
    return it->second.pImage;
  }



  void Font::SetLetter( GR::u32 Letter, GR::Graphic::Image* pLetter )
  {
    tLetterInfo&    letter = m_Letters[Letter];

    if ( letter.pImage )
    {
      delete letter.pImage;
    }
    letter.pImage = pLetter;

    if ( ( m_FixedWidth )
    &&   ( pLetter ) )
    {
      if ( m_LetterWidth < letter.pImage->GetWidth() )
      {
        m_LetterWidth = letter.pImage->GetWidth();
      }
    }
  }



  int Font::TextLength( const GR::String& Text ) const
  {
    unsigned long   length = 0;

    if ( Text.empty() )
    {
      return 0;
    }

    for ( size_t i = 0; i < Text.length(); i++ )
    {
      if ( !m_FixedWidth )
      {
        GR::Graphic::Image* pLetter = GetLetter( Text[i] );
        if ( pLetter )
        {
          length += pLetter->GetWidth() + m_FontSpacing;
        }
      }
      else
      {
        length += m_LetterWidth + m_FontSpacing;
      }
    }
    if ( length )
    {
      length -= m_FontSpacing;
    }
    return length;
  }



  int Font::TextLength( const GR::Char* Text ) const
  {
    unsigned long   length = 0;

    if ( Text == NULL )
    {
      return 0;
    }
    size_t len = GR::Strings::Length( Text );
    if ( len == 0 )
    {
      return 0;
    }

    for ( size_t i = 0; i < len; i++ )
    {
      if ( !m_FixedWidth )
      {
        GR::Graphic::Image*   pLetter = GetLetter( Text[i] );
        if ( pLetter )
        {
          length += pLetter->GetWidth() + m_FontSpacing;
        }
      }
      else
      {
        length += m_LetterWidth + m_FontSpacing;
      }
    }
    if ( length )
    {
      length -= m_FontSpacing;
    }
    return length;
  }



  int Font::TextHeight( const GR::Char* Text ) const
  {
    size_t    len = 0;
    if ( ( Text == NULL )
    ||   ( ( len = GR::Strings::Length( Text ) ) == 0 ) )
    {
      // Standard-Höhe der üblichen Übeltäter
      return TextHeight( "AZÄÖÜyg\\" );
    }

    unsigned long   height = 0;

    for ( size_t i = 0; i < len; i++ )
    {
      GR::Graphic::Image*   pLetter = GetLetter( Text[i] );
      if ( pLetter )
      {
        if ( pLetter->GetHeight() > height )
        {
          height = pLetter->GetHeight();
        }
      }
    }
    return height;
  }



  int Font::FontSpacing()
  {
    return m_FontSpacing;
  }



  bool Font::PrintFont( GR::Graphic::GFXPage* pActualPage, signed long X, signed long Y, const GR::String& Text, unsigned long Flags, GR::Graphic::Image* pMaskImage ) const
  {
    // TODO - nix UTF8!
    for ( size_t i = 0; i < Text.length(); i++ )
    {
      tLetters::const_iterator   it( m_Letters.find( ( GR::u8 )Text[i] ) );
      if ( it == m_Letters.end() )
      {
        continue;
      }

      const tLetterInfo& Letter = it->second;

      GR::Graphic::Image* pLetter = Letter.pImage;
      if ( pLetter == NULL )
      {
        continue;
      }
      if ( Flags == IMAGE_METHOD_OPTIMAL )
      {
        if ( pLetter->GetCompressList() )
        {
          pLetter->PutImage( pActualPage,
            X + Letter.LetterXOffset,
            Y + m_BaseLineOffsetY + Letter.LetterYOffset,
            IMAGE_METHOD_OPTIMAL, pMaskImage );
        }
        else
        {
          pLetter->PutImage( pActualPage,
            X + Letter.LetterXOffset,
            Y + m_BaseLineOffsetY + Letter.LetterYOffset,
            IMAGE_METHOD_TRANSPARENT, pMaskImage );
        }
      }
      else
      {
        pLetter->PutImage( pActualPage,
          X + Letter.LetterXOffset,
          Y + m_BaseLineOffsetY + Letter.LetterYOffset,
          Flags, pMaskImage );
      }
      if ( !m_FixedWidth )
      {
        X += pLetter->GetWidth() + m_FontSpacing;
      }
      else
      {
        X += m_LetterWidth + m_FontSpacing;
      }
    }

    return true;
  }



  bool Font::PrintFont( GR::Graphic::GFXPage* pActualPage, signed long X, signed long Y, const GR::Char* Text, unsigned long Flags, GR::Graphic::Image* pMaskImage ) const
  {
    if ( Text == NULL )
    {
      return false;
    }

    // TODO - nix UTF8!
    for ( size_t i = 0; i < strlen( Text ); i++ )
    {
      tLetters::const_iterator   it( m_Letters.find( (GR::u8)Text[i] ) );
      if ( it == m_Letters.end() )
      {
        continue;
      }

      const tLetterInfo&    Letter = it->second;

      GR::Graphic::Image*   pLetter = Letter.pImage;
      if ( pLetter == NULL )
      {
        continue;
      }
      if ( Flags == IMAGE_METHOD_OPTIMAL )
      {
        if ( pLetter->GetCompressList() )
        {
          pLetter->PutImage( pActualPage, 
                             X + Letter.LetterXOffset,
                             Y + m_BaseLineOffsetY + Letter.LetterYOffset, 
                             IMAGE_METHOD_OPTIMAL, pMaskImage );
        }
        else
        {
          pLetter->PutImage( pActualPage, 
                             X + Letter.LetterXOffset,
                             Y + m_BaseLineOffsetY + Letter.LetterYOffset, 
                             IMAGE_METHOD_TRANSPARENT, pMaskImage );
        }
      }
      else
      {
        pLetter->PutImage( pActualPage, 
                           X + Letter.LetterXOffset,
                           Y + m_BaseLineOffsetY + Letter.LetterYOffset, 
                           Flags, pMaskImage );
      }
      if ( !m_FixedWidth )
      {
        X += pLetter->GetWidth() + m_FontSpacing;
      }
      else
      {
        X += m_LetterWidth + m_FontSpacing;
      }
    }

    return true;
  }



  void Font::Compress( GR::u32 TransparentColor )
  {
    tLetters::iterator   it( m_Letters.begin() );
    while ( it != m_Letters.end() )
    {
      tLetterInfo&    LetterInfo = it->second;

      if ( LetterInfo.pImage )
      {
        LetterInfo.pImage->SetTransparentColorRGB( TransparentColor );
        LetterInfo.pImage->Compress();
      }
      ++it;
    }
  }



  GR::u32 Font::GetDepth() const
  {
    tLetters::const_iterator   it( m_Letters.begin() );
    while ( it != m_Letters.end() )
    {
      const tLetterInfo&    LetterInfo = it->second;

      if ( LetterInfo.pImage )
      {
        return LetterInfo.pImage->GetDepth();
      }

      ++it;
    }
    return 0;
  }



  bool Font::Convert( unsigned char Bpp, GR::Graphic::Palette* pPalette )
  {
    tLetters::const_iterator   it( m_Letters.begin() );
    while ( it != m_Letters.end() )
    {
      const tLetterInfo&    LetterInfo = it->second;

      if ( LetterInfo.pImage )
      {
        LetterInfo.pImage->Convert( Bpp, pPalette );
      }

      ++it;
    }

    return TRUE;
  }

}
