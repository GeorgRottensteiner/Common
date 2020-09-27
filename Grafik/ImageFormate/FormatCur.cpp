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
      size_t   iLength = FileName.length();
      if ( iLength >= 3 )
      {
        if ( ( toupper( FileName[iLength - 3] ) != 'C' )
             || ( toupper( FileName[iLength - 2] ) != 'U' )
             || ( toupper( FileName[iLength - 1] ) != 'R' ) )
        {
          return false;
        }
      }
      else
      {
        return false;
      }

      GR::IO::FileStream        aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return false;
      }

      aFile.ReadU16();
      WORD wType = aFile.ReadU16();
      aFile.Close();

      if ( wType != 2 )
      {
        return false;
      }
      return true;

    }



    ImageSet* FormatCur::LoadSet( const GR::String& FileName )
    {
      GR::IO::FileStream        aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      GR::Graphic::ImageData* pImageData = new GR::Graphic::ImageData();
      GR::Graphic::ImageData* pImageDataMask = new GR::Graphic::ImageData();

      aFile.ReadU16();
      WORD wType = aFile.ReadU16();
      if ( wType != 2 )
      {
        aFile.Close();
        return NULL;
      }

      GR::u16  numImages = aFile.ReadU16();


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
      WORD    wWidth = aFile.ReadU8(),
        wHeight = aFile.ReadU8(),
        wColors = aFile.ReadU8();
      aFile.ReadU8();
      GR::u16 hotSpotOffsetX = aFile.ReadU16();
      GR::u16 hotSpotOffsetY = aFile.ReadU16();
      DWORD   dwBytes = aFile.ReadU32(),
        dwOffset = aFile.ReadU32();

      if ( wWidth == 0 )
      {
        wWidth = 256;
      }
      if ( wHeight == 0 )
      {
        wHeight = 256;
      }

      aFile.SetPosition( dwOffset, IIOStream::PT_SET );

      if ( wColors == 16 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header
        aFile.ReadBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
        pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );

        memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );

        for ( int i = 0; i < wColors; i++ )
        {
          RGBQUAD   myQuad;

          aFile.ReadBlock( &myQuad, sizeof( RGBQUAD ) );
          pImageData->Palette().SetColor( i, myQuad.rgbRed, myQuad.rgbGreen, myQuad.rgbBlue );
        }
        if ( wColors < 255 )
        {
          // Ersatz-Transparenz-Farbe
          pImageData->Palette().SetColor( 255, 255, 255, 255 );
        }
        for ( int i = 0; i < icHeader.biHeight / 2; i++ )
        {
          int iCount = 0;
          for ( int j = 0; j < icHeader.biWidth / 2; j++ )
          {
            iCount++;
            if ( iCount == 4 )
            {
              iCount = 0;
            }
            BYTE    bByte = aFile.ReadU8();
            // da sind 2 Bytes drin!
            ( ( (BYTE*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Width() )[j * 2]
              = ( bByte >> 4 );
            ( ( (BYTE*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Height() )[j * 2 + 1]
              = ( bByte & 0x0f );
          }
          // Padding auf 32bit
          while ( iCount % 4 )
          {
            aFile.ReadU8();
            iCount++;
          }
        }
        for ( int i = 0; i < icHeader.biHeight / 2; i++ )
        {
          int   iCount = 0;
          for ( int j = 0; j < icHeader.biWidth / 8; j++ )
          {
            iCount = ( ( iCount + 1 ) % 4 );
            BYTE    bByte = aFile.ReadU8();
            DWORD   dwBit = 0x80;

            for ( int k = 0; k < 8; k++ )
            {
              if ( bByte & dwBit )
              {
                // da ist ein Bit!
                if ( j * 8 + k < icHeader.biWidth )
                {
                  ( ( (BYTE*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                }
              }
              dwBit >>= 1;
            }
          }
          // Padding auf 32bit
          while ( iCount % 4 )
          {
            aFile.ReadU8();
            iCount++;
          }
        }
      }
      else if ( wColors == 0 )
      {
        BITMAPINFOHEADER   icHeader;      // DIB header
        aFile.ReadBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        if ( icHeader.biBitCount == 8 )
        {
          pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );

          int   Full32 = pImageData->Width();
          while ( Full32 % 4 )
          {
            ++Full32;
          }
          int   Padding = Full32 - pImageData->Width();

          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            aFile.ReadBlock( ( (BYTE*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->BytesPerLine(),
                             pImageData->Width() );
            aFile.SetPosition( Padding, IIOStream::PT_CURRENT );
          }
          int   RealWidth = icHeader.biWidth / 8;
          if ( icHeader.biWidth % 8 )
          {
            ++RealWidth;
          }
          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            int   iCount = 0;
            for ( int j = 0; j < RealWidth; j++ )
            {
              iCount = ( ( iCount + 1 ) % 4 );
              BYTE    bByte = aFile.ReadU8();
              DWORD   dwBit = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( bByte & dwBit )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < icHeader.biWidth )
                  {
                    ( ( (BYTE*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                dwBit >>= 1;
              }
            }
            // Padding auf 32bit
            while ( iCount % 4 )
            {
              aFile.ReadU8();
              iCount++;
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
            aFile.ReadBlock( ( (BYTE*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Width() * 3,
                             pImageData->Width() * 3 );
            aFile.SetPosition( Padding, IIOStream::PT_CURRENT );
          }
          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            int   iCount = 0;
            for ( int j = 0; j < icHeader.biWidth / 8; j++ )
            {
              iCount = ( ( iCount + 1 ) % 4 );
              BYTE    bByte = aFile.ReadU8();
              DWORD   dwBit = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( bByte & dwBit )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < icHeader.biWidth )
                  {
                    ( ( (BYTE*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                dwBit >>= 1;
              }
            }
            // Padding auf 32bit
            while ( iCount % 4 )
            {
              aFile.ReadU8();
              iCount++;
            }
          }
        }
        else if ( icHeader.biBitCount == 32 )
        {
          pImageData->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_A8R8G8B8 );
          pImageDataMask->CreateData( icHeader.biWidth, icHeader.biHeight / 2, GR::Graphic::IF_PALETTED );
          memset( pImageDataMask->Data(), 0xff, pImageDataMask->DataSize() );
          int   Full32 = pImageData->Width() * 4;
          int   Padding = 0;

          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            aFile.ReadBlock( ( (BYTE*)pImageData->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageData->Width() * 4,
                             pImageData->Width() * 4 );
            aFile.SetPosition( Padding, IIOStream::PT_CURRENT );
          }
          for ( int i = 0; i < icHeader.biHeight / 2; i++ )
          {
            int   iCount = 0;
            for ( int j = 0; j < icHeader.biWidth / 8; j++ )
            {
              iCount = ( ( iCount + 1 ) % 4 );
              BYTE    bByte = aFile.ReadU8();
              DWORD   dwBit = 0x80;

              for ( int k = 0; k < 8; k++ )
              {
                if ( bByte & dwBit )
                {
                  // da ist ein Bit!
                  if ( j * 8 + k < icHeader.biWidth )
                  {
                    ( ( (BYTE*)pImageDataMask->Data() ) + ( icHeader.biHeight / 2 - i - 1 ) * pImageDataMask->Width() )[j * 8 + k] = 0;
                  }
                }
                dwBit >>= 1;
              }
            }
            // Padding auf 32bit
            while ( iCount % 4 )
            {
              aFile.ReadU8();
              iCount++;
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
        else
        {
          dh::Log( "Header sagt %d bpp\n", icHeader.biBitCount );
        }
      }
      else
      {
        dh::Log( "Cursor %d unsupported\n", wColors );
      }

      aFile.Close();

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pImageData, pImageDataMask );

      return pSet;
    }



    bool FormatCur::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
           || ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 ) )
      {
        return true;
      }
      return false;
    }



    bool FormatCur::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      GR::IO::FileStream        aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      /*
      WORD           idReserved;   // Reserved (must be 0)
        WORD           idType;       // Resource Type (1 for icons)
        WORD           idCount;      // How many images?
        ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
        */

      aFile.WriteU16( 0 );   // reserved
      aFile.WriteU16( 2 );   // 1 = Icon, 2 = Cursor
      aFile.WriteU16( 1 );   // 1 Icon im File TODO - mehr als eins

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

      aFile.WriteU8( pData->Width() );
      aFile.WriteU8( pData->Height() );
      aFile.WriteU8( 0 );               // 0: >= 8bpp
      aFile.WriteU8( 0 );               // reserved
      aFile.WriteU16( 0 );               // 1 Plane
      aFile.WriteU16( 0 );               // bitcount

      size_t    iDirEntryPos = (size_t)aFile.GetPosition();

      aFile.WriteU32( 0 );               // Bytes in resource
      aFile.WriteU32( 0 );               // Image-Offset

      size_t    iIconResStart = (size_t)aFile.GetPosition();

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

        aFile.WriteBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        for ( int i = 0; i < 256; i++ )
        {
          RGBQUAD   myQuad;

          myQuad.rgbRed = pData->Palette().Red( i );
          myQuad.rgbGreen = pData->Palette().Green( i );
          myQuad.rgbBlue = pData->Palette().Blue( i );
          myQuad.rgbReserved = 0;

          aFile.WriteBlock( &myQuad, sizeof( RGBQUAD ) );
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
                aFile.WriteU8( 0 );
              }
              else
              {
                aFile.WriteU8( ( GR::u8 )pData->GetPixel( j, pData->Height() - i - 1 ) );
              }
            }
          }
          else
          {
            aFile.WriteBlock( ( (BYTE*)pData->Data() ) + ( pData->Height() - i - 1 ) * pData->Width(),
                              pData->Width() );
          }
          for ( int j = 0; j < padding; ++j )
          {
            aFile.WriteU8( 0 );
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
                aFile.WriteU8( bCurrentByte );
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
                  aFile.WriteU8( bCurrentByte );
                }
                else
                {
                  aFile.WriteU8( 0 );
                }
                bCurrentByte = 0;
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

        aFile.WriteBlock( &icHeader, sizeof( BITMAPINFOHEADER ) );

        // XOR-Maske (bzw. Grafik)
        int     padding = 0;
        for ( int i = 0; i < pData->Height(); i++ )
        {
          if ( pMask != NULL )
          {
            for ( int j = 0; j < pData->Width(); ++j )
            {
              aFile.WriteU32( ( GR::u32 )pData->GetPixel( j, pData->Height() - i - 1 ) | ( pMask->GetPixel( j, pData->Height() - i - 1 ) << 24 ) );
            }
          }
          else
          {
            aFile.WriteBlock( ( (BYTE*)pData->Data() ) + ( pData->Height() - i - 1 ) * pData->Width() * 4, pData->Width() * 4 );
          }
          for ( int j = 0; j < padding; ++j )
          {
            aFile.WriteU8( 0 );
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
                aFile.WriteU8( bCurrentByte );
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
                  aFile.WriteU8( bCurrentByte );
                }
                else
                {
                  aFile.WriteU8( 0 );
                }
                bCurrentByte = 0;
              }
            }
          }
        }
      }

      size_t    iIconResEnd = (size_t)aFile.GetPosition();

      aFile.SetPosition( (long)iDirEntryPos, IIOStream::PT_SET );
      aFile.WriteU32( (unsigned long)( iIconResEnd - iIconResStart ) );
      aFile.WriteU32( (unsigned long)iIconResStart );

      aFile.Close();

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