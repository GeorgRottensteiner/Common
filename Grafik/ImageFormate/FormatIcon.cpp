#include <windows.h>

#include <IO/FileStream.h>

#include <Grafik/Image.h>

#include <Grafik\ImageFormate\FormatIcon.h>

#include <String/StringUtil.h>

#include <debug/debugclient.h>



namespace GR
{
  namespace Graphic
  {
    FormatIcon        s_GlobalIconPlugin;



    FormatIcon::FormatIcon()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatIcon::IsFileOfType( const GR::String& FileName )
    {
      // damit nicht viele Files falsch als Icon erkannt werden
      size_t   length = FileName.length();
      if ( length >= 3 )
      {
        if ( ( toupper( FileName[length - 3] ) != 'I' )
        ||   ( toupper( FileName[length - 2] ) != 'C' )
        ||   ( toupper( FileName[length - 1] ) != 'O' ) )
        {
          return false;
        }
      }
      else
      {
        return false;
      }

      GR::IO::FileStream        file;

      if ( !file.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return false;
      }

      file.ReadU16();
      GR::u16 type = file.ReadU16();
      file.Close();

      if ( type != 1 )
      {
        return false;
      }
      return true;
    }



    ImageSet* FormatIcon::LoadSet( const GR::String& FileName )
    {
      GR::IO::FileStream        file;

      if ( !file.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      GR::Graphic::ImageData* pImageData = new GR::Graphic::ImageData();
      GR::Graphic::ImageData* pImageDataMask = new GR::Graphic::ImageData();

      file.ReadU16();
      GR::u16 type = file.ReadU16();
      if ( type != 1 )
      {
        file.Close();
        return NULL;
      }

      file.ReadU16();


      /*
      typedef struct
      {
          BYTE        bWidth;          // Width, in pixels, of the image
          BYTE        bHeight;         // Height, in pixels, of the image
          BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
          BYTE        bReserved;       // Reserved ( must be 0)
          WORD        wPlanes;         // Color Planes
          WORD        wBitCount;       // Bits per pixel
          DWORD       dwBytesInRes;    // How many bytes in this resource?
          DWORD       dwImageOffset;   // Where in the file is this image?
      } ICONDIRENTRY, *LPICONDIRENTRY;
      */
      GR::u16     width = file.ReadU8(),
                  height = file.ReadU8(),
                  numColors = file.ReadU8();
      file.ReadU8();
      GR::u16     numPlanes = file.ReadU16(),
                  bitCount = file.ReadU16();
      GR::u32     bytes = file.ReadU32(),
                  offset = file.ReadU32();

      file.SetPosition( offset, IIOStream::PT_SET );

      if ( numColors == 16 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header
        file.ReadBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
        pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );

        memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );

        for ( int i = 0; i < numColors; i++ )
        {
          RGBQUAD   myQuad;

          file.ReadBlock( &myQuad, sizeof( RGBQUAD ) );
          pImageData->Palette().SetColor( i, myQuad.rgbRed, myQuad.rgbGreen, myQuad.rgbBlue );
        }
        if ( numColors < 255 )
        {
          // Ersatz-Transparenz-Farbe
          pImageData->Palette().SetColor( 255, 255, 255, 255 );
        }
        for ( int i = 0; i < icHeader.biHeight / 2; i++ )
        {
          int count = 0;
          for ( int j = 0; j < icHeader.biWidth / 2; j++ )
          {
            count = ( ( count + 1 ) % 4 );

            GR::u8    byte = file.ReadU8();
            // da sind 2 Bytes drin!
            ( ( (GR::u8*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Width() )[j * 2]      = ( byte >> 4 );
            ( ( (GR::u8*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Height() )[j * 2 + 1] = ( byte & 0x0f );
          }
          // Padding auf 32bit
          while ( count % 4 )
          {
            file.ReadU8();
            ++count;
          }
        }
        for ( int i = 0; i < icHeader.biHeight / 2; i++ )
        {
          int   count = 0;
          for ( int j = 0; j < icHeader.biWidth / 8; j++ )
          {
            count = ( ( count + 1 ) % 4 );
            GR::u8    byte = file.ReadU8();
            GR::u32   bitMask = 0x80;

            for ( int k = 0; k < 8; k++ )
            {
              if ( byte & bitMask )
              {
                // da ist ein Bit!
                if ( j * 8 + k < icHeader.biWidth )
                {
                  ( ( (BYTE*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                }
              }
              bitMask >>= 1;
            }
          }
          // Padding auf 32bit
          while ( count % 4 )
          {
            file.ReadU8();
            ++count;
          }
        }
      }
      else if ( bitCount == 8 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header
        file.ReadBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );
        pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
        pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
        memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );

        for ( int i = 0; i < 256; i++ )
        {
          RGBQUAD   myQuad;

          file.ReadBlock( &myQuad, sizeof( RGBQUAD ) );
          pImageData->Palette().SetColor( i, myQuad.rgbRed, myQuad.rgbGreen, myQuad.rgbBlue );
        }
        int   Full32 = pImageData->Width();
        while ( Full32 % 4 )
        {
          ++Full32;
        }
        int   Padding = Full32 - pImageData->Width();

        for ( int i = 0; i < icHeader.biHeight / 2; i++ )
        {
          file.ReadBlock( ( ( GR::u8*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Width(), pImageData->Width() );
          file.SetPosition( Padding, IIOStream::PT_CURRENT );
        }
        // read mask
        int   realWidth = icHeader.biWidth / 8;
        if ( icHeader.biWidth % 8 )
        {
          ++realWidth;
        }
        for ( int i = 0; i < icHeader.biHeight / 2; i++ )
        {
          int   count = 0;
          for ( int j = 0; j < realWidth; j++ )
          {
            count = ( ( count + 1 ) % 4 );
            GR::u8    byte = file.ReadU8();
            GR::u32   bitMask = 0x80;

            for ( int k = 0; k < 8; k++ )
            {
              if ( byte & bitMask )
              {
                // da ist ein Bit!
                if ( j * 8 + k < icHeader.biWidth )
                {
                  ( ( (GR::u8*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                }
              }
              bitMask >>= 1;
            }
          }
          // Padding auf 32bit
          while ( count % 4 )
          {
            file.ReadU8();
            ++count;
          }
        }
      }
      else if ( bitCount == 0 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header
        file.ReadBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        if ( icHeader.biBitCount == 8 )
        {
          pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );
          for ( int i = 0; i < 256; i++ )
          {
            RGBQUAD   myQuad;

            file.ReadBlock( &myQuad, sizeof( RGBQUAD ) );
            pImageData->Palette().SetColor( i, myQuad.rgbRed, myQuad.rgbGreen, myQuad.rgbBlue );
          }

          int   Full32 = pImageData->Width();
          while ( Full32 % 4 )
          {
            ++Full32;
          }
          int   Padding = Full32 - pImageData->Width();

          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            file.ReadBlock( ( (GR::u8*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->BytesPerLine(), pImageData->Width() );
            file.SetPosition( Padding, IIOStream::PT_CURRENT );
          }
          int   realWidth = icHeader.biWidth / 8;
          if ( icHeader.biWidth % 8 )
          {
            ++realWidth;
          }
          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            int   count = 0;
            for ( int j = 0; j < realWidth; j++ )
            {
              count = ( ( count + 1 ) % 4 );
              GR::u8    byte = file.ReadU8();
              GR::u32   bitMask = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( byte & bitMask )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < icHeader.biWidth )
                  {
                    ( ( (GR::u8*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                bitMask >>= 1;
              }
            }
            // Padding auf 32bit
            while ( count % 4 )
            {
              file.ReadU8();
              ++count;
            }
          }
        }
        else if ( icHeader.biBitCount == 24 )
        {
          pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_R8G8B8 );
          pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );
          int   Full32 = pImageData->Width() * 3;
          while ( Full32 % 4 )
          {
            ++Full32;
          }
          int   Padding = Full32 - pImageData->Width() * 3;

          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            file.ReadBlock( ( (GR::u8*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Width() * 3, pImageData->Width() * 3 );
            file.SetPosition( Padding, IIOStream::PT_CURRENT );
          }
          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            int   count = 0;
            for ( int j = 0; j < icHeader.biWidth / 8; j++ )
            {
              count = ( ( count + 1 ) % 4 );
              GR::u8    byte = file.ReadU8();
              GR::u32   bitMask = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( byte & bitMask )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < icHeader.biWidth )
                  {
                    ( ( (GR::u8*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                bitMask >>= 1;
              }
            }
            // Padding auf 32bit
            while ( count % 4 )
            {
              file.ReadU8();
              ++count;
            }
          }
        }
        else
        {
          dh::Log( "Header has %d bpp", icHeader.biBitCount );
        }
      }
      else
      {
        dh::Log( "Icon with bit count %d unsupported", bitCount );
      }

      file.Close();

      ImageSet*    pSet = new ImageSet();

      pSet->AddFrame( pImageData, pImageDataMask );

      return pSet;
    }



    bool FormatIcon::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      {
        return true;
      }
      return false;
    }



    bool FormatIcon::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      GR::IO::FileStream        file;

      if ( !file.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      /*
      WORD           idReserved;   // Reserved (must be 0)
        WORD           idType;       // Resource Type (1 for icons)
        WORD           idCount;      // How many images?
        ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
        */

      file.WriteU16( 0 );   // reserved
      file.WriteU16( 1 );   // 1 = Icon
      file.WriteU16( 1 );   // 1 Icon im File TODO - mehr als eins

      /*
      typedef struct
      {
          BYTE        bWidth;          // Width, in pixels, of the image
          BYTE        bHeight;         // Height, in pixels, of the image
          BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
          BYTE        bReserved;       // Reserved ( must be 0)
          WORD        wPlanes;         // Color Planes
          WORD        wBitCount;       // Bits per pixel
          DWORD       dwBytesInRes;    // How many bytes in this resource?
          DWORD       dwImageOffset;   // Where in the file is this image?
      } ICONDIRENTRY, *LPICONDIRENTRY;
      */

      file.WriteU8( pData->Width() );
      file.WriteU8( pData->Height() );
      file.WriteU8( 0 );               // 0: >= 8bpp
      file.WriteU8( 0 );               // reserved
      file.WriteU16( 0 );               // 1 Plane
      file.WriteU16( 0 );               // bitcount

      size_t    dirEntryPos = (size_t)file.GetPosition();

      file.WriteU32( 0 );               // Bytes in resource
      file.WriteU32( 0 );               // Image-Offset

      size_t    iconResStart = (size_t)file.GetPosition();
      {
        BITMAPINFOHEADER   icHeader;      // DIB header

        icHeader.biBitCount       = 8;
        icHeader.biClrImportant   = 0;
        icHeader.biClrUsed        = 0;
        icHeader.biCompression    = BI_RGB;
        icHeader.biHeight         = pData->Height() * 2;
        icHeader.biPlanes         = 1;
        icHeader.biSize           = sizeof( BITMAPINFOHEADER );
        icHeader.biSizeImage      = 0;
        icHeader.biWidth          = pData->Width();
        icHeader.biXPelsPerMeter  = 0;
        icHeader.biYPelsPerMeter  = 0;

        file.WriteBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        for ( int i = 0; i < 256; i++ )
        {
          RGBQUAD   myQuad;

          myQuad.rgbRed       = pData->Palette().Red( i );
          myQuad.rgbGreen     = pData->Palette().Green( i );
          myQuad.rgbBlue      = pData->Palette().Blue( i );
          myQuad.rgbReserved  = 0;

          file.WriteBlock( &myQuad, sizeof( RGBQUAD ) );
        }

        // XOR-Maske (bzw. Grafik)
        int     padding = 0;
        if ( pData->BytesPerLine() % 4 )
        {
          padding = 4 - pData->BytesPerLine() % 4;
        }
        for ( int i = 0; i < pData->Height(); i++ )
        {
          if ( pMask != NULL )
          {
            for ( int j = 0; j < pData->Width(); ++j )
            {
              if ( pMask->GetPixel( j, pData->Height() - i - 1 ) == 0 )
              {
                file.WriteU8( 0 );
              }
              else
              {
                file.WriteU8( (GR::u8)pData->GetPixel( j, pData->Height() - i - 1 ) );
              }
            }
          }
          else
          {
            file.WriteBlock( ( (GR::u8*)pData->Data() ) + ( pData->Height() - i - 1 ) * pData->Width(), pData->Width() );
          }
          for ( int j = 0; j < padding; ++j )
          {
            file.WriteU8( 0 );
          }
        }

        // AND-Maske
        int   bytesPerLine = pData->Width() / 8;
        if ( pData->Width() % 8 )
        {
          bytesPerLine++;
        }
        while ( bytesPerLine % 4 )
        {
          bytesPerLine++;
        }

        if ( pMask != NULL )
        {
          for ( int i = 0; i < pMask->Height(); i++ )
          {
            GR::u8    currentByte = 0;
            for ( int bit = 0; bit < bytesPerLine * 8; ++bit )
            {
              if ( ( bit < pMask->Width() )
              &&   ( pMask->GetPixel( bit, pMask->Height() - 1 - i ) == 0 ) )
              {
                currentByte |= ( 1 << ( 7 - ( bit % 8 ) ) );
              }

              if ( ( bit % 8 ) == 7 )
              {
                file.WriteU8( currentByte );
                currentByte = 0;
              }
            }
          }
        }
        else
        {
          for ( int i = 0; i < pData->Height(); i++ )
          {
            GR::u8    currentByte = 0;
            for ( int bit = 0; bit < bytesPerLine * 8; ++bit )
            {
              if ( pData->GetPixel( bit, pData->Height() - 1 - i ) == pData->TransparentColor() )
              {
                currentByte |= ( 1 << ( 7 - ( bit % 8 ) ) );
              }

              if ( ( bit % 8 ) == 7 )
              {
                if ( pData->TransparentColorUsed() )
                {
                  file.WriteU8( currentByte );
                }
                else
                {
                  file.WriteU8( 0 );
                }
                currentByte = 0;
              }
            }
          }
        }
      }

      size_t    iconResEnd = (size_t)file.GetPosition();

      file.SetPosition( (long)dirEntryPos, IIOStream::PT_SET );
      file.WriteU32( (unsigned long)( iconResEnd - iconResStart ) );
      file.WriteU32( (unsigned long)iconResStart );

      file.Close();

      return false;
    }



    GR::String FormatIcon::GetDescription()
    {
      return "Icon Image";
    }



    GR::String FormatIcon::GetFilterString()
    {
      return "*.ico";
    }



  }

}