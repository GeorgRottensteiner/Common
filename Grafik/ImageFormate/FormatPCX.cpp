#include <IO/FileStream.h>

#include <Grafik\ImageFormate\FormatPCX.h>



GR::Graphic::FormatPCX    globalPCXPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatPCX::FormatPCX()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatPCX::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return false;
      }

      if ( aFile.ReadU8() != 10 )
      {
        aFile.Close();
        return false;
      }
      aFile.Close();
      return true;
    }



    ImageSet* FormatPCX::LoadSet( const GR::String& FileName )
    {
#pragma pack(1)
      struct tPCXHeader
      {
        char    Manufacturer;	// Always 10 -- 
        char    Version;      // Version information
                              // 0 = Ver2.5, 2 = Ver2.8 with palette, 3 = Ver2.8 without palette,
                              // 4 = PC Paintbrush for Windows, 5 = Ver 3.0

        char    Encoding;     // 1 = Run Length Encoding

        char    BitsPerPixel; // Number of bits that represent a pixel

        short   xMin,
          yMin,
          xMax,
          yMax;         // Image Dimensions

        short   HorDPI;       // Horizontal Resolution of pixels in dots per inch
        short   VerDPI;       // Vertical Resolution of pixels in dots per inch

        char    Colormap[48]; // Color palette setting

        char    Reserved1;    // Should be set to zero
        char    nPlanes;      // Number of color planes
        short   BytesPerLine; // Number of bytes to allocate per scanline
                              // Must be an even number
        short   PaletteInfo;  // How to interpet palette

        short   HorScreenSize; // Horizontal screen size in pixels
        short   VerScreenSize; // Vertical screen size in pixels

        char    Filler[54];   // An addtional 54 bytes that must be set to 0 -- 
                              // Total size of a .pcx header is 128 bytes
      };
#pragma pack()

      tPCXHeader    PCXHeader;

      WORD          wWidth,
        wHeight;


      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      aFile.ReadBlock( &PCXHeader, 128 );

      if ( ( PCXHeader.Manufacturer != 10 )                    // Keine PCX-Kennung
           || ( PCXHeader.BitsPerPixel != 8 ) )                    // <> 256 Farben
      {
        // nur echte PCXe und 256-Farben
        aFile.Close();
        return NULL;
      }

      wWidth = PCXHeader.xMax - PCXHeader.xMin + 1;
      wHeight = PCXHeader.yMax - PCXHeader.yMin + 1;

      GR::Graphic::ImageData* pData = new GR::Graphic::ImageData();

      int     iLineSize = 0;

      if ( PCXHeader.nPlanes == 3 )
      {
        // ein 24bit-PCX
        pData->CreateData( wWidth, wHeight, GR::Graphic::IF_R8G8B8 );
        iLineSize = wWidth * 3;
      }
      else
      {
        pData->CreateData( wWidth, wHeight, GR::Graphic::IF_PALETTED );
        iLineSize = wWidth;
      }


      // The total number of bytes in a scanline
      int   iBytesPerLine = (int)PCXHeader.nPlanes * PCXHeader.BytesPerLine;

      BYTE* pScanLine = new BYTE[iBytesPerLine];


      for ( WORD wY = 0; wY < wHeight; wY++ )
      {
        BYTE    ucValue = 0;
        BYTE    ucCount = 0;
        BYTE    ucTemp = 0;

        int     iBytesInLineRead = 0;

        while ( iBytesInLineRead < iBytesPerLine )
        {
          ucTemp = aFile.ReadU8();

          // "dekodieren"
          if ( ( ucTemp & 0xC0 ) == 0xC0 )
          {
            // die unteren 6 bit bestimmen, wie oft wiederholt wird
            ucCount = ucTemp & 0x3F;

            ucValue = aFile.ReadU8();
          }
          else
          {
            // nur einmal wiederholen
            ucCount = 1;
            ucValue = ucTemp;
          }

          // Fill the current portion of the scanline
          for ( int i = 0; i < ucCount; i++, iBytesInLineRead++ )
          {
            pScanLine[iBytesInLineRead] = ucValue;
          }
        }

        // If it's an 8-bit .pcx file, simply copy over the scanline
        if ( PCXHeader.nPlanes == 1 )
        {
          memcpy( pData->GetRowData( wY ), pScanLine, wWidth );
        }
        else // Converge "scan_line" into a the same form as a DIB_BITMAP 24-bit scanline
        {
          BYTE* pFinishedLine = new BYTE[iBytesPerLine];

          // scan_line actually contains three lines -- one of red, green, blue
          // They need to be intergrated into one DIB_BITMAP scanline 
          for ( int i = 0; i < wWidth; i++ )
          {
            pFinishedLine[i * PCXHeader.nPlanes + 2] = pScanLine[i];
            pFinishedLine[i * PCXHeader.nPlanes + 1] = pScanLine[i + PCXHeader.BytesPerLine];
            pFinishedLine[i * PCXHeader.nPlanes + 0] = pScanLine[i + PCXHeader.BytesPerLine * 2];
          }

          // Copy it over
          memcpy( pData->GetRowData( wY ), pFinishedLine, pData->BytesPerLine() );

          // Free mem
          delete[] pFinishedLine;
        }
      }

      // Free mem
      delete[] pScanLine;

      // Palette lesen, falls vorhanden
      BYTE    bBuffer[769];

      aFile.SetPosition( -769, IIOStream::PT_END );
      aFile.ReadBlock( &bBuffer, 769 );
      if ( bBuffer[0] == 12 )
      {
        for ( int i = 0; i < 256; i++ )
        {
          pData->Palette().SetColor( i, bBuffer[1 + i * 3], bBuffer[2 + i * 3], bBuffer[3 + i * 3] );
        }
      }

      aFile.Close();

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pData );

      return pSet;

    }



    bool FormatPCX::CanSave( GR::Graphic::ImageData* pData )
    {

      if ( pData == NULL )
      {
        return false;
      }

      if ( ( pData->ImageFormat() != GR::Graphic::IF_PALETTED )
           && ( pData->ImageFormat() != GR::Graphic::IF_R8G8B8 ) )
      {
        return false;
      }
      return true;

    }



    bool FormatPCX::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      if ( !CanSave( pData ) )
      {
        return false;
      }

      unsigned short int      count,
        col;


      GR::IO::FileStream               MyFile;


      if ( !MyFile.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      MyFile.WriteU8( 10 );   // Manufacturer
      MyFile.WriteU8( 5 );    // Version
      MyFile.WriteU8( 1 );    // RLE
      MyFile.WriteU8( 8 );    // BitsPerPixel

      MyFile.WriteU16( 0 );    // xMin
      MyFile.WriteU16( 0 );    // yMin
      MyFile.WriteU16( pData->Width() - 1 );   // xMax
      MyFile.WriteU16( pData->Height() - 1 );  // yMax

      MyFile.WriteU16( 72 );   // HorDPI
      MyFile.WriteU16( 72 );   // VerDPI

      for ( int i = 0; i < 48; ++i )
      {
        MyFile.WriteU8( 0 );
      }

      int   iBytesPerLine = pData->Width();

      MyFile.WriteU8( 0 );
      if ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
      {
        MyFile.WriteU8( 3 );    // nPlanes
        iBytesPerLine *= 3;
      }
      else
      {
        MyFile.WriteU8( 1 );    // nPlanes
      }
      MyFile.WriteU16( pData->Width() ); // Bytes per line (must be even?)
      MyFile.WriteU16( 1 );    // PaletteInfo

      for ( int i = 71; i <= 128; i++ )
      {
        MyFile.WriteU8( 0 );
      }

      BYTE* pImageData = (BYTE*)pData->Data();

      BYTE* pLine = new BYTE[iBytesPerLine];

      BYTE    ucLast = 0;
      count = 0;
      for ( int i = 0; i <= pData->Height() - 1; i++ )
      {
        if ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
        {
          BYTE* pSrc = (BYTE*)pData->GetRowData( i );
          BYTE* pTgt = pLine;

          for ( int i = 0; i < pData->Width(); i++ )
          {
            pTgt[i + 0 * pData->Width()] = pSrc[i * 3];
            pTgt[i + 1 * pData->Width()] = pSrc[i * 3 + 1];
            pTgt[i + 2 * pData->Width()] = pSrc[i * 3 + 2];
          }
        }
        else
        {
          memcpy( pLine, pData->GetRowData( i ), iBytesPerLine );
        }

        pImageData = pLine;
        ucLast = *pImageData;
        count = 1;
        for ( int j = 1; j <= iBytesPerLine - 1; j++ )
        {
          pImageData++;
          col = *pImageData;
          if ( ucLast == col )
          {
            count++;
            if ( count == 63 )
            {
              MyFile.WriteU8( 0xc0 | count );
              MyFile.WriteU8( ucLast );
              count = 0;
            }
          }
          else
          {
            if ( count )
            {
              if ( ( count == 1 )
                   && ( 0xc0 != ( 0xc0 & ucLast ) ) )
              {
                MyFile.WriteU8( ucLast );
              }
              else
              {
                MyFile.WriteU8( 0xc0 | count );
                MyFile.WriteU8( ucLast );
              }
            }
            ucLast = (unsigned char)col;
            count = 1;
          }
        }
        if ( count )
        {
          if ( ( count == 1 )
               && ( 0xc0 != ( 0xc0 & ucLast ) ) )
          {
            MyFile.WriteU8( ucLast );
          }
          else
          {
            MyFile.WriteU8( 0xc0 | count );
            MyFile.WriteU8( ucLast );
          }
        }
      }

      delete[] pLine;

      if ( pData->Palette().Data() )
      {
        // Palette
        MyFile.WriteU8( 12 );
        for ( int i = 0; i < 256; i++ )
        {
          MyFile.WriteU8( pData->Palette().Red( i ) );
          MyFile.WriteU8( pData->Palette().Green( i ) );
          MyFile.WriteU8( pData->Palette().Blue( i ) );
        }
      }

      MyFile.Close();
      return true;
    }



    GR::String FormatPCX::GetDescription()
    {
      return "PCX Image";
    }



    GR::String FormatPCX::GetFilterString()
    {
      return "*.pcx";
    }
  }
}