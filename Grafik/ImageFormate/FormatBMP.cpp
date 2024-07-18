#include <IO/FileStream.h>

#include <debug/debugclient.h>

#include "FormatBMP.h"



GR::Graphic::FormatBMP    globalBMPPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatBMP::FormatBMP()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatBMP::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream    aFile;

      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        return false;
      }
      if ( aFile.ReadU8() != 'B' )
      {
        return false;
      }
      if ( aFile.ReadU8() != 'M' )
      {
        return false;
      }
      aFile.Close();
      return true;
    }



    ImageSet* FormatBMP::LoadSet( const GR::String& FileName )
    {
      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      BITMAPFILEHEADER bmfh;

      aFile.ReadBlock( &bmfh, sizeof( BITMAPFILEHEADER ) );

      BITMAPINFOHEADER bmih;

      aFile.ReadBlock( &bmih, sizeof( BITMAPINFOHEADER ) );

      GR::Graphic::eImageFormat    formatBMP = GR::Graphic::IF_UNKNOWN;

      if ( bmih.biBitCount == 8 )
      {
        formatBMP = GR::Graphic::IF_PALETTED;
      }
      else if ( bmih.biBitCount == 1 )
      {
        formatBMP = GR::Graphic::IF_MONOCHROME;
      }
      else if ( bmih.biBitCount == 4 )
      {
        formatBMP = GR::Graphic::IF_INDEX4;
      }
      else if ( bmih.biBitCount == 16 )
      {
        formatBMP = GR::Graphic::IF_X1R5G5B5;
      }
      else if ( bmih.biBitCount == 24 )
      {
        formatBMP = GR::Graphic::IF_R8G8B8;
      }
      else if ( bmih.biBitCount == 32 )
      {
        formatBMP = GR::Graphic::IF_A8R8G8B8;
      }
      else
      {
        aFile.Close();
        dh::Log( "FormatBMP::Load  biBitCount %d not supported yet.\n", bmih.biBitCount );
        return NULL;
      }

      /*
      dh::Log( "FormatBMP::Load  offset %d  - size %d + %d = %d\n",
            bmfh.bfOffBits, sizeof( BITMAPFILEHEADER ), sizeof( BITMAPINFOHEADER ),
            sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) );

      dh::Log( "FormatBMP::Load  Compression %d  Size %dx%d  clrused %d\n",
          bmih.biCompression,
          bmih.biWidth,
          bmih.biHeight,
          bmih.biClrUsed );
      */

      bool    upsideDown = ( bmih.biHeight >= 0 );

      int     yStep = 1;
      int     yStart = 0;
      if ( upsideDown )
      {
        yStep = -1;
        yStart = bmih.biHeight - 1;
      }

      GR::Graphic::ImageData* pImageData = new GR::Graphic::ImageData();
      GR::Graphic::ImageData* pImageMask = NULL;

      pImageData->CreateData( bmih.biWidth, abs( bmih.biHeight ), formatBMP );

      int   paddedWidth = pImageData->BytesPerLine(),
        paddedBytes = 0;
      while ( paddedWidth % 4 )
      {
        ++paddedWidth;
        ++paddedBytes;
      }

      switch ( formatBMP )
      {
        case GR::Graphic::IF_PALETTED:
          {
            int   colors = bmih.biClrUsed;
            if ( colors == 0 )
            {
              colors = 256;
            }
            for ( int i = 0; i < colors; i++ )
            {
              pImageData->Palette().SetColor( i,
                                              aFile.ReadU8(),
                                              aFile.ReadU8(),
                                              aFile.ReadU8() );
              aFile.ReadU8();
            }

            if ( bmih.biCompression == BI_RLE8 )
            {
              int   x = 0,
                y = yStart;

              BYTE* pData = (BYTE*)pImageData->Data() + y * bmih.biWidth;

              BYTE  firstByte,
                secondByte;

              while ( true )
              {
                firstByte = aFile.ReadU8();
                secondByte = aFile.ReadU8();

                if ( firstByte == 0 )
                {
                  // absolute Mode
                  if ( secondByte >= 3 )
                  {
                    aFile.ReadBlock( pData, secondByte );
                    if ( secondByte % 2 )
                    {
                      aFile.ReadU8();
                    }
                    x += secondByte;
                    pData += secondByte;
                  }
                  // End of Line
                  else if ( secondByte == 0 )
                  {
                    x = 0;
                    y += yStep;
                    pData = (BYTE*)pImageData->Data() + y * bmih.biWidth;
                  }
                  // End of Bitmap
                  else if ( secondByte == 1 )
                  {
                    break;
                  }
                  // Delta
                  else
                  {
                    int   dx = aFile.ReadU8();
                    int   dy = aFile.ReadU8();
                    x += dx;
                    pData += x;
                    y += dy * yStep;
                    pData += dy * bmih.biWidth * yStep;
                  }
                }
                else
                {
                  // Encoded Mode
                  for ( int i = 0; i < firstByte; i++ )
                  {
                    *pData++ = secondByte;
                    x++;
                  }
                }
              }
            }
            else
            {
              BYTE* pData = NULL;

              for ( int j = 0; j < abs( bmih.biHeight ); j++ )
              {
                pData = (BYTE*)pImageData->Data() + ( yStart + j * yStep ) * bmih.biWidth;
                aFile.ReadBlock( pData, bmih.biWidth );
                for ( int i = 0; i < paddedBytes; ++i )
                {
                  aFile.ReadU8();
                }
              }
            }
          }
          break;
        case GR::Graphic::IF_INDEX4:
          {
            int   iColors = bmih.biClrUsed;
            if ( iColors == 0 )
            {
              iColors = 16;
            }
            for ( int i = 0; i < 16; i++ )
            {
              pImageData->Palette().SetColor( i,
                                              aFile.ReadU8(),
                                              aFile.ReadU8(),
                                              aFile.ReadU8() );
              aFile.ReadU8();
            }

            if ( bmih.biCompression == BI_RLE4 )
            {
              int   x = 0,
                y = yStart;

              BYTE* pData = (BYTE*)pImageData->GetRowData( y );

              BYTE  firstByte,
                secondByte;

              while ( true )
              {
                firstByte = aFile.ReadU8();
                secondByte = aFile.ReadU8();

                if ( firstByte == 0 )
                {
                  // absolute Mode
                  if ( secondByte >= 3 )
                  {
                    aFile.ReadBlock( pData, secondByte / 2 );
                    if ( secondByte % 4 )
                    {
                      // runs must be aligned on WORD boundaries
                      aFile.ReadU8();
                    }
                    x += secondByte;
                    pData += secondByte / 2;
                  }
                  // End of Line
                  else if ( secondByte == 0 )
                  {
                    x = 0;
                    y += yStep;
                    pData = (BYTE*)pImageData->GetRowData( y );
                  }
                  // End of Bitmap
                  else if ( secondByte == 1 )
                  {
                    break;
                  }
                  // Delta
                  else
                  {
                    int   dx = aFile.ReadU8();
                    int   dy = aFile.ReadU8();
                    x += dx;
                    pData += x / 2;
                    y += dy * yStep;
                    pData += dy * yStep * pImageData->LineOffsetInBytes();
                  }
                }
                else
                {
                  // Encoded Mode
                  for ( int i = 0; i < firstByte / 2; i++ )
                  {
                    *pData++ = secondByte;
                    x += 2;
                  }
                  if ( firstByte & 1 )
                  {
                    if ( x & 1 )
                    {
                      *pData++ |= ( ( secondByte & 0xf0 ) >> 4 );
                    }
                    else
                    {
                      *pData |= ( secondByte & 0x0f );
                    }
                    ++x;
                  }
                }
              }
            }
            else
            {
              BYTE* pData = NULL;

              for ( int j = 0; j < abs( bmih.biHeight ); j++ )
              {
                pData = (BYTE*)pImageData->Data() + ( yStart + j * yStep ) * pImageData->BytesPerLine();
                aFile.ReadBlock( pData, pImageData->BytesPerLine() );
                for ( int i = 0; i < paddedBytes; ++i )
                {
                  aFile.ReadU8();
                }
              }
            }
          }
          break;
        case GR::Graphic::IF_INDEX1:
          {
            int   colors = bmih.biClrUsed;
            if ( colors == 0 )
            {
              colors = 2;
            }
            for ( int i = 0; i < 2; i++ )
            {
              pImageData->Palette().SetColor( i,
                                              aFile.ReadU8(),
                                              aFile.ReadU8(),
                                              aFile.ReadU8() );
              aFile.ReadU8();
            }

            BYTE* pData = NULL;

            for ( int j = 0; j < abs( bmih.biHeight ); j++ )
            {
              pData = (BYTE*)pImageData->Data() + ( yStart + j * yStep ) * pImageData->LineOffsetInBytes();
              aFile.ReadBlock( pData, pImageData->BytesPerLine() );
              for ( int i = 0; i < paddedBytes; ++i )
              {
                aFile.ReadU8();
              }
            }
          }
          break;
        case GR::Graphic::IF_R8G8B8:
          {

            BYTE* pData = NULL;

            for ( int j = 0; j < abs( bmih.biHeight ); j++ )
            {
              pData = (BYTE*)pImageData->Data() + ( yStart + j * yStep ) * bmih.biWidth * 3;
              aFile.ReadBlock( pData, bmih.biWidth * 3 );
              for ( int i = 0; i < paddedBytes; ++i )
              {
                aFile.ReadU8();
              }
            }
          }
          break;
        case GR::Graphic::IF_A8R8G8B8:
          {

            BYTE* pData = NULL;

            for ( int j = 0; j < abs( bmih.biHeight ); j++ )
            {
              pData = (BYTE*)pImageData->Data() + ( yStart + j * yStep ) * bmih.biWidth * 4;
              aFile.ReadBlock( pData, bmih.biWidth * 4 );
              for ( int i = 0; i < paddedBytes; ++i )
              {
                aFile.ReadU8();
              }
            }
            // create mask from alpha bytes
            pImageMask = new GR::Graphic::ImageData();
            pImageMask->CreateData( bmih.biWidth, abs( bmih.biHeight ), GR::Graphic::IF_PALETTED );

            for ( int j = 0; j < pImageData->Height(); ++j )
            {
              GR::u8* pSource = (GR::u8*)pImageData->GetRowData( j ) + 3;
              GR::u8* pTargetMask = (GR::u8*)pImageMask->GetRowData( j );

              for ( int i = 0; i < pImageData->Width(); ++i )
              {
                *pTargetMask++ = *pSource;
                pSource += 4;
              }
            }
          }
          break;
        case GR::Graphic::IF_X1R5G5B5:
          {
            bool    is16 = false;

            if ( bmih.biCompression == BI_BITFIELDS )
            {
              // 3 Masken
              GR::u32   rMask = aFile.ReadU32();
              GR::u32   gMask = aFile.ReadU32();
              GR::u32   bMask = aFile.ReadU32();

              if ( rMask == 0xf800 )
              {
                formatBMP = GR::Graphic::IF_R5G6B5;
                pImageData->CreateData( bmih.biWidth, abs( bmih.biHeight ), formatBMP );
                is16 = true;
              }
            }

            BYTE* pData = NULL;

            for ( int j = 0; j < abs( bmih.biHeight ); j++ )
            {
              pData = (BYTE*)pImageData->Data() + ( yStart + j * yStep ) * bmih.biWidth * 2;
              aFile.ReadBlock( pData, bmih.biWidth * 2 );
              for ( int i = 0; i < paddedBytes; ++i )
              {
                aFile.ReadU8();
              }
            }

            if ( is16 )
            {
              pImageData->ConvertSelfTo( GR::Graphic::IF_X1R5G5B5 );
            }
          }
          break;
        default:
          dh::Log( "FormatBMP::Load  ImageFormat %d not supported yet.\n", formatBMP );
          break;
      }
      aFile.Close();

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pImageData, pImageMask );

      return pSet;
    }



    bool FormatBMP::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( pData == NULL )
      {
        return false;
      }
      if ( ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_MONOCHROME )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_INDEX4 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_X1R5G5B5 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_X8R8G8B8 ) 
      ||   ( pData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
      {
        return true;
      }
      return false;
    }



    bool FormatBMP::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      if ( !CanSave( pData ) )
      {
        return false;
      }


      GR::IO::FileStream               MyFile;


      if ( !MyFile.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      // BitmapFileHeader
      MyFile.WriteU16( 0x4d42 );
      MyFile.WriteU32( 0 );    // Size of Bitmap File
      MyFile.WriteU32( 0 );
      MyFile.WriteU32( 0 );    // Offset to Bitmap bits

      // BitmapInfoHeader
      MyFile.WriteU32( 40 );   // size of BITMAPINFOHEADER
      MyFile.WriteU32( pData->Width() );
      MyFile.WriteU32( pData->Height() );
      MyFile.WriteU16( 1 );
      int bits = pData->BitsProPixel();
      if ( bits == 15 )
      {
        bits = 16;
      }
      MyFile.WriteU16( bits );
      MyFile.WriteU32( 0 );      // BI_RGB
      MyFile.WriteU32( 0 );      // sizeof Image
      MyFile.WriteU32( 0 );
      MyFile.WriteU32( 0 );
      MyFile.WriteU32( 0 );
      MyFile.WriteU32( 0 );

      if ( pData->BitsProPixel() <= 8 )
      {
        // Palette
        for ( size_t i = 0; i < pData->Palette().Entries(); ++i )
        {
          MyFile.WriteU8( pData->Palette().Blue( i ) );
          MyFile.WriteU8( pData->Palette().Green( i ) );
          MyFile.WriteU8( pData->Palette().Red( i ) );
          MyFile.WriteU8( 0 );
        }
      }

      if ( ( ( pData->ImageFormat() == GR::Graphic::IF_X8R8G8B8 )
      ||     ( pData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
      &&   ( pMask != NULL ) )
      {
        ByteBuffer    lineData( pData->BytesPerLine() );
        for ( int j = pData->Height() - 1; j >= 0; --j )
        {
          // need to inject mask values!
          memcpy( lineData.Data(), pData->GetRowData( j ), pData->BytesPerLine() );
          for ( int i = 0; i < pData->Width(); ++i )
          {
            lineData.SetByteAt( i * 4 + 3, pMask->GetPixel( i, j ) );
          }

          MyFile.WriteBlock( lineData.Data(), pData->BytesPerLine() );
          int   padding = ( pData->BytesPerLine() % 4 );

          while ( padding % 4 )
          {
            MyFile.WriteU8( 0 );
            padding++;
          }
        }
      }
      else
      {
        for ( int j = pData->Height() - 1; j >= 0; --j )
        {
          MyFile.WriteBlock( pData->GetRowData( j ), pData->BytesPerLine() );
          int   padding = ( pData->BytesPerLine() % 4 );

          while ( padding % 4 )
          {
            MyFile.WriteU8( 0 );
            padding++;
          }
        }
      }
      MyFile.Flush();
      GR::u32 size = ( GR::u32 )MyFile.GetSize();

      MyFile.SetPosition( 2, IIOStream::PT_SET );
      MyFile.WriteU32( size );
      MyFile.SetPosition( 4, IIOStream::PT_CURRENT );
      if ( pData->Palette().Entries() > 0 )
      {
        MyFile.WriteU32( 14 + 40 + ( GR::u32 )pData->Palette().Entries() * 4 );
      }
      else
      {
        MyFile.WriteU32( 14 + 40 );
      }
      MyFile.Close();

      return true;
    }



    GR::String  FormatBMP::GetFilterString()
    {
      return "*.bmp";
    }



    GR::String  FormatBMP::GetDescription()
    {
      return "BMP Image";
    }

  }
}