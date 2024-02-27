#include <IO/FileStream.h>

#include <Grafik/Image.h>

#include <Grafik\ImageFormate\FormatCur.h>
#include <Grafik\ContextDescriptor.h>

#include <String/StringUtil.h>

#include <debug/debugclient.h>

#include <windows.h>


GR::Graphic::FormatCur    globalCursorPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatCur::FormatCur()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatCur::IsFileOfType( const GR::String& FileName )
    {
      // damit nicht viele Files falsch als Icon erkannt werden
      size_t   length = FileName.length();
      if ( length >= 3 )
      {
        if ( ( toupper( FileName[length - 3] ) != 'C' )
        ||   ( toupper( FileName[length - 2] ) != 'U' )
        ||   ( toupper( FileName[length - 1] ) != 'R' ) )
        {
          return false;
        }
      }
      else
      {
        return false;
      }

      GR::IO::FileStream        file;

      if ( !file.Open( FileName ) )
      {
        return false;
      }

      file.ReadU16();
      GR::u32 type = file.ReadU16();
      file.Close();

      if ( type != 2 )
      {
        return false;
      }
      return true;

    }



    ImageSet* FormatCur::LoadSet( const GR::String& FileName )
    {
      GR::IO::FileStream        file;

      if ( !file.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      file.ReadU16();
      GR::u16 type = file.ReadU16();
      if ( type != 2 )
      {
        file.Close();
        return NULL;
      }

      GR::u16  numImages = file.ReadU16();

      GR::Graphic::ImageData* pImageData = new GR::Graphic::ImageData();
      GR::Graphic::ImageData* pImageDataMask = new GR::Graphic::ImageData();


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
      GR::u16   width = file.ReadU8(),
                height = file.ReadU8(),
                colors = file.ReadU8();
      file.ReadU8();
      GR::u16 hotSpotOffsetX = file.ReadU16();
      GR::u16 hotSpotOffsetY = file.ReadU16();
      GR::u32   numBytes = file.ReadU32(),
                imageOffset = file.ReadU32();

      if ( width == 0 )
      {
        width = 256;
      }
      if ( height == 0 )
      {
        height = 256;
      }

      file.SetPosition( imageOffset, IIOStream::PT_SET );

      if ( colors == 16 )
      {
        BITMAPINFOHEADER   iconHeader;      
        file.ReadBlock( &iconHeader, sizeof( BITMAPINFOHEADER ) );

        pImageData->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
        pImageDataMask->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );

        memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );

        for ( int i = 0; i < colors; i++ )
        {
          RGBQUAD   myQuad;

          file.ReadBlock( &myQuad, sizeof( RGBQUAD ) );
          pImageData->Palette().SetColor( i, myQuad.rgbRed, myQuad.rgbGreen, myQuad.rgbBlue );
        }
        if ( colors < 255 )
        {
          // Ersatz-Transparenz-Farbe
          pImageData->Palette().SetColor( 255, 255, 255, 255 );
        }
        for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
        {
          int count = 0;
          for ( int j = 0; j < iconHeader.biWidth / 2; j++ )
          {
            count++;
            if ( count == 4 )
            {
              count = 0;
            }
            GR::u8    byte = file.ReadU8();

            // two half bytes
            ( ( (GR::u8*)pImageData->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageData->Width() )[j * 2]      = ( byte >> 4 );
            ( ( (GR::u8*)pImageData->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageData->Height() )[j * 2 + 1] = ( byte & 0x0f );
          }

          // pad to 32bit
          while ( count % 4 )
          {
            file.ReadU8();
            count++;
          }
        }
        for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
        {
          int   count = 0;
          for ( int j = 0; j < iconHeader.biWidth / 8; j++ )
          {
            count = ( ( count + 1 ) % 4 );

            GR::u8  byte = file.ReadU8();
            GR::u32 curBit = 0x80;

            for ( int k = 0; k < 8; k++ )
            {
              if ( byte & curBit )
              {
                if ( j * 8 + k < iconHeader.biWidth )
                {
                  ( ( (GR::u8*)pImageDataMask->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                }
              }
              curBit >>= 1;
            }
          }
          // pad to 32bit
          while ( count % 4 )
          {
            file.ReadU8();
            count++;
          }
        }
      }
      else if ( colors == 0 )
      {
        BITMAPINFOHEADER   iconHeader;
        file.ReadBlock( &iconHeader, sizeof( BITMAPINFOHEADER ) );

        if ( iconHeader.biBitCount == 8 )
        {
          pImageData->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          pImageDataMask->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );

          int   widthPaddedTo32Bit = pImageData->Width();
          while ( widthPaddedTo32Bit % 4 )
          {
            ++widthPaddedTo32Bit;
          }
          int   padding = widthPaddedTo32Bit - pImageData->Width();

          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            file.ReadBlock( ( (BYTE*)pImageData->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageData->BytesPerLine(),
                             pImageData->Width() );
            file.SetPosition( padding, IIOStream::PT_CURRENT );
          }
          int   realWidth = iconHeader.biWidth / 8;
          if ( iconHeader.biWidth % 8 )
          {
            ++realWidth;
          }
          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            int   count = 0;
            for ( int j = 0; j < realWidth; j++ )
            {
              count = ( ( count + 1 ) % 4 );
              GR::u8  byte = file.ReadU8();
              GR::u32 curBit = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( byte & curBit )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < iconHeader.biWidth )
                  {
                    ( ( (GR::u8*)pImageDataMask->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                curBit >>= 1;
              }
            }
            // pad to 32bit
            while ( count % 4 )
            {
              file.ReadU8();
              count++;
            }
          }
        }
        else if ( iconHeader.biBitCount == 24 )
        {
          pImageData->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_R8G8B8 );
          pImageDataMask->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );
          int   widthPaddedTo32Bit = pImageData->Width() * 3;
          while ( widthPaddedTo32Bit % 4 )
          {
            ++widthPaddedTo32Bit;
          }
          int   padding = widthPaddedTo32Bit - pImageData->Width() * 3;

          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            file.ReadBlock( ( (GR::u8*)pImageData->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageData->Width() * 3,
                             pImageData->Width() * 3 );
            file.SetPosition( padding, IIOStream::PT_CURRENT );
          }
          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            int   count = 0;
            for ( int j = 0; j < iconHeader.biWidth / 8; j++ )
            {
              count = ( ( count + 1 ) % 4 );
              GR::u8  byte = file.ReadU8();
              GR::u32 curBit = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( byte & curBit )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < iconHeader.biWidth )
                  {
                    ( ( (GR::u8*)pImageDataMask->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                curBit >>= 1;
              }
            }
            // pad to 32bit
            while ( count % 4 )
            {
              file.ReadU8();
              count++;
            }
          }
        }
        else if ( iconHeader.biBitCount == 32 )
        {
          pImageData->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_A8R8G8B8 );
          pImageDataMask->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );
          int   widthPaddedTo32Bit = pImageData->Width() * 4;

          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            file.ReadBlock( ( (GR::u8*)pImageData->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageData->Width() * 4,
                             pImageData->Width() * 4 );
          }
          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            int   count = 0;
            for ( int j = 0; j < iconHeader.biWidth / 8; j++ )
            {
              count = ( ( count + 1 ) % 4 );
              GR::u8  byte = file.ReadU8();
              GR::u32 curBit = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( byte & curBit )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < iconHeader.biWidth )
                  {
                    ( ( (GR::u8*)pImageDataMask->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                curBit >>= 1;
              }
            }
            // pad to 32bit
            while ( count % 4 )
            {
              file.ReadU8();
              count++;
            }
          }

          // copy alpha bits into mask
          GR::Graphic::ContextDescriptor    cdMask;

          cdMask.Attach( pImageDataMask->Width(), pImageDataMask->Height(), pImageDataMask->LineOffsetInBytes(), pImageDataMask->ImageFormat(), pImageDataMask->Data() );

          for ( int j = 0; j < pImageData->Height(); ++j )
          {
            for ( int i = 0; i < pImageData->Width(); ++i )
            {
              cdMask.PutDirectPixel( i, j, ( pImageData->GetPixel( i, j ) >> 24 ) );
            }
          }
        }
        else if ( iconHeader.biBitCount == 1 )
        {
          pImageData->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_INDEX1 );
          pImageDataMask->CreateData( iconHeader.biWidth, iconHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );
          for ( int i = 0; i < 2; i++ )
          {
            RGBQUAD   myQuad;

            file.ReadBlock( &myQuad, sizeof( RGBQUAD ) );
            pImageData->Palette().SetColor( i, myQuad.rgbRed, myQuad.rgbGreen, myQuad.rgbBlue );
          }

          int   widthPaddedTo32Bit = pImageData->Width();
          while ( widthPaddedTo32Bit % 4 )
          {
            ++widthPaddedTo32Bit;
          }
          int   padding = widthPaddedTo32Bit - pImageData->Width();

          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
          {
            file.ReadBlock( ( (GR::u8*)pImageData->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageData->BytesPerLine(), widthPaddedTo32Bit / 8 );
            file.SetPosition( padding, IIOStream::PT_CURRENT );
          }
          int   realWidth = iconHeader.biWidth / 8;
          if ( iconHeader.biWidth % 8 )
          {
            ++realWidth;
          }
          for ( int i = 0; i < iconHeader.biHeight / 2; i++ )
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
                  if ( j * 8 + k < iconHeader.biWidth )
                  {
                    ( ( (GR::u8*)pImageDataMask->Data() ) + ( iconHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
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
          dh::Log( "FormatCur: Header used %d bpp, not implemented yet", iconHeader.biBitCount );
        }
      }
      else
      {
        dh::Log( "Cursor %d unsupported\n", colors );
      }

      file.Close();

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pImageData, pImageDataMask );

      return pSet;
    }



    bool FormatCur::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_INDEX1 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 ) )
      {
        return true;
      }
      return false;
    }



    bool FormatCur::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
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
      file.WriteU16( 2 );   // 1 = Icon, 2 = Cursor
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

      size_t    iDirEntryPos = (size_t)file.GetPosition();

      file.WriteU32( 0 );               // Bytes in resource
      file.WriteU32( 0 );               // Image-Offset

      size_t    iIconResStart = (size_t)file.GetPosition();

      if ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header

        icHeader.biBitCount = 8;
        icHeader.biClrImportant = 0;
        icHeader.biClrUsed = 0;
        icHeader.biCompression = BI_RGB;
        icHeader.biHeight = pData->Height() * 2;
        icHeader.biPlanes = 1;
        icHeader.biSize = sizeof( BITMAPINFOHEADER );
        icHeader.biSizeImage = 0;
        icHeader.biWidth = pData->Width();
        icHeader.biXPelsPerMeter = 0;
        icHeader.biYPelsPerMeter = 0;

        file.WriteBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        for ( int i = 0; i < 256; i++ )
        {
          RGBQUAD   myQuad;

          myQuad.rgbRed = pData->Palette().Red( i );
          myQuad.rgbGreen = pData->Palette().Green( i );
          myQuad.rgbBlue = pData->Palette().Blue( i );
          myQuad.rgbReserved = 0;

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
                file.WriteU8( ( GR::u8 )pData->GetPixel( j, pData->Height() - i - 1 ) );
              }
            }
          }
          else
          {
            file.WriteBlock( ( (BYTE*)pData->Data() ) + ( pData->Height() - i - 1 ) * pData->Width(),
                              pData->Width() );
          }
          for ( int j = 0; j < padding; ++j )
          {
            file.WriteU8( 0 );
          }
        }

        // AND-Maske
        int   iBytesPerLine = pData->Width() / 8;
        if ( pData->Width() % 8 )
        {
          iBytesPerLine++;
        }
        while ( iBytesPerLine % 4 )
        {
          iBytesPerLine++;
        }

        if ( pMask != NULL )
        {
          for ( int i = 0; i < pMask->Height(); i++ )
          {
            BYTE    bCurrentByte = 0;
            for ( int iBit = 0; iBit < iBytesPerLine * 8; ++iBit )
            {
              if ( ( iBit < pMask->Width() )
                   && ( pMask->GetPixel( iBit, pMask->Height() - 1 - i ) == 0 ) )
              {
                bCurrentByte |= ( 1 << ( 7 - ( iBit % 8 ) ) );
              }

              if ( ( iBit % 8 ) == 7 )
              {
                file.WriteU8( bCurrentByte );
                bCurrentByte = 0;
              }
            }
          }
        }
        else
        {
          for ( int i = 0; i < pData->Height(); i++ )
          {
            BYTE    bCurrentByte = 0;
            for ( int iBit = 0; iBit < iBytesPerLine * 8; ++iBit )
            {
              if ( pData->GetPixel( iBit, pData->Height() - 1 - i ) == pData->TransparentColor() )
              {
                bCurrentByte |= ( 1 << ( 7 - ( iBit % 8 ) ) );
              }

              if ( ( iBit % 8 ) == 7 )
              {
                if ( pData->TransparentColorUsed() )
                {
                  file.WriteU8( bCurrentByte );
                }
                else
                {
                  file.WriteU8( 0 );
                }
                bCurrentByte = 0;
              }
            }
          }
        }
      }
      else if ( pData->ImageFormat() == GR::Graphic::IF_INDEX1 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header

        icHeader.biBitCount       = 1;
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

        for ( int i = 0; i < 2; i++ )
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

        int   bytesPerLine = pData->Width() / 8;
        if ( pData->Width() % 8 )
        {
          bytesPerLine++;
        }
        while ( bytesPerLine % 4 )
        {
          bytesPerLine++;
        }

        for ( int i = 0; i < pData->Height(); i++ )
        {
          GR::u8    currentByte = 0;
          for ( int bit = 0; bit < bytesPerLine * 8; ++bit )
          {
            if ( pData->GetPixel( bit, pData->Height() - 1 - i ) != 0 )
            {
              currentByte |= ( 1 << ( bit % 8 ) );
            }
            if ( ( bit % 8 ) == 7 )
            {
              file.WriteU8( currentByte );
              currentByte = 0;
            }
          }
          for ( int j = 0; j < padding; ++j )
          {
            file.WriteU8( 0 );
          }
        }

        // AND-Maske
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
      else if ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header

        icHeader.biBitCount = 32;
        icHeader.biClrImportant = 0;
        icHeader.biClrUsed = 0;
        icHeader.biCompression = BI_RGB;
        icHeader.biHeight = pData->Height() * 2;
        icHeader.biPlanes = 1;
        icHeader.biSize = sizeof( BITMAPINFOHEADER );
        icHeader.biSizeImage = 0;
        icHeader.biWidth = pData->Width();
        icHeader.biXPelsPerMeter = 0;
        icHeader.biYPelsPerMeter = 0;

        file.WriteBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        // XOR-Maske (bzw. Grafik)
        int     padding = 0;
        for ( int i = 0; i < pData->Height(); i++ )
        {
          if ( pMask != NULL )
          {
            for ( int j = 0; j < pData->Width(); ++j )
            {
              file.WriteU32( ( GR::u32 )pData->GetPixel( j, pData->Height() - i - 1 ) | ( pMask->GetPixel( j, pData->Height() - i - 1 ) << 24 ) );
            }
          }
          else
          {
            file.WriteBlock( ( (BYTE*)pData->Data() ) + ( pData->Height() - i - 1 ) * pData->Width() * 4, pData->Width() * 4 );
          }
          for ( int j = 0; j < padding; ++j )
          {
            file.WriteU8( 0 );
          }
        }

        // AND-Maske
        int   iBytesPerLine = pData->Width() / 8;
        if ( pData->Width() % 8 )
        {
          iBytesPerLine++;
        }
        while ( iBytesPerLine % 4 )
        {
          iBytesPerLine++;
        }

        if ( pMask != NULL )
        {
          for ( int i = 0; i < pMask->Height(); i++ )
          {
            BYTE    bCurrentByte = 0;
            for ( int iBit = 0; iBit < iBytesPerLine * 8; ++iBit )
            {
              if ( ( iBit < pMask->Width() )
                   && ( pMask->GetPixel( iBit, pMask->Height() - 1 - i ) == 0 ) )
              {
                bCurrentByte |= ( 1 << ( 7 - ( iBit % 8 ) ) );
              }
              if ( ( iBit % 8 ) == 7 )
              {
                file.WriteU8( bCurrentByte );
                bCurrentByte = 0;
              }
            }
          }
        }
        else
        {
          for ( int i = 0; i < pData->Height(); i++ )
          {
            BYTE    bCurrentByte = 0;
            for ( int iBit = 0; iBit < iBytesPerLine * 8; ++iBit )
            {
              if ( pData->GetPixel( iBit, pData->Height() - 1 - i ) == pData->TransparentColor() )
              {
                bCurrentByte |= ( 1 << ( 7 - ( iBit % 8 ) ) );
              }

              if ( ( iBit % 8 ) == 7 )
              {
                if ( pData->TransparentColorUsed() )
                {
                  file.WriteU8( bCurrentByte );
                }
                else
                {
                  file.WriteU8( 0 );
                }
                bCurrentByte = 0;
              }
            }
          }
        }
      }

      size_t    iIconResEnd = (size_t)file.GetPosition();

      file.SetPosition( (long)iDirEntryPos, IIOStream::PT_SET );
      file.WriteU32( (unsigned long)( iIconResEnd - iIconResStart ) );
      file.WriteU32( (unsigned long)iIconResStart );

      file.Close();

      return false;
    }



    void FormatCur::SetMetaData( const GR::String& Key, const GR::String& Value )
    {
      if ( Key == "HotSpotX" )
      {
      }
    }



    GR::String FormatCur::MetaData( const GR::String& Key )
    {
      return GR::String();
    }



    GR::String FormatCur::GetFilterString()
    {
      return "*.cur";
    }



    GR::String FormatCur::GetDescription()
    {
      return "Cursor Image";
    }


  }
}