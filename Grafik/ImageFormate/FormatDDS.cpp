#include <IO/FileStream.h>

#include <Grafik/Image.h>

#include <Grafik\ImageFormate\FormatDDS.h>

#include <debug/debugclient.h>

#include <ddraw.h>



GR::Graphic::FormatDDS    globalDDSPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatDDS::FormatDDS()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatDDS::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream    aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return false;
      }

      char      szMagic[5];

      szMagic[4] = 0;

      aFile.ReadBlock( szMagic, 4 );

      aFile.Close();

      if ( ( szMagic[0] != 'D' )
           || ( szMagic[1] != 'D' )
           || ( szMagic[2] != 'S' )
           || ( szMagic[3] != ' ' ) )
      {
        return false;
      }

      return true;
    }



    ImageSet* FormatDDS::LoadSet( const GR::String& FileName )
    {
      GR::IO::FileStream    aFile;
      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      aFile.ReadU32();

      DDSURFACEDESC2      ddsd2;

      aFile.ReadBlock( &ddsd2, sizeof( ddsd2 ) );

      if ( ddsd2.dwSize != 124 )
      {
        aFile.Close();
        return NULL;
      }

      if ( ddsd2.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXEDTO8 )
      {
        dh::Log( "Indexed to 8\n" );
      }
      else if ( ddsd2.ddpfPixelFormat.dwFlags & DDPF_RGB )
      {
        dh::Log( "rgb\n" );
      }
      else if ( ddsd2.ddpfPixelFormat.dwFlags & DDPF_FOURCC )
      {
        dh::Log( "Im Moment nicht unterstütztes DDS-Format FourCC (%x) = (%c%c%c%c).\n",
                 ddsd2.ddpfPixelFormat.dwFlags,
                 ( ddsd2.ddpfPixelFormat.dwFourCC & 0xff ),
                 ( ddsd2.ddpfPixelFormat.dwFourCC & 0xff00 ) >> 8,
                 ( ddsd2.ddpfPixelFormat.dwFourCC & 0xff0000 ) >> 16,
                 ( ddsd2.ddpfPixelFormat.dwFourCC & 0xff000000 ) >> 24 );
        aFile.Close();
        return NULL;
      }
      else
      {
        dh::Log( "Im Moment nicht unterstütztes DDS-Format (%x).\n", ddsd2.ddpfPixelFormat.dwFlags );
        aFile.Close();
        return NULL;
      }

      GR::Graphic::eImageFormat    imgFormat = GR::Graphic::IF_UNKNOWN;

      GR::Graphic::ImageData* pData = new GR::Graphic::ImageData();
      if ( ddsd2.ddpfPixelFormat.dwFlags & DDPF_RGB )
      {
        if ( ( ddsd2.ddpfPixelFormat.dwRBitMask == 0xff0000 )
             && ( ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000 ) )
        {
          // 32 bit
          imgFormat = GR::Graphic::IF_A8R8G8B8;
        }
      }

      if ( imgFormat == GR::Graphic::IF_UNKNOWN )
      {
        dh::Log( "Im Moment nicht unterstütztes DDS-Format.\n" );

        delete pData;
        aFile.Close();
        return NULL;
      }

      pData->CreateData( ddsd2.dwWidth, ddsd2.dwHeight, imgFormat );

      aFile.ReadBlock( pData->Data(), pData->DataSize() );


      aFile.Close();

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pData );

      return pSet;
    }



    bool FormatDDS::CanSave( GR::Graphic::ImageData* pData )
    {
      return true;
    }



    bool FormatDDS::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      /*
      GR::IO::FileStream      aFile;

      if ( aFile.Open( szFileName, FILE_WRITE_NEW ) )
      {
        return false;
      }

      unsigned char   ucFileType = IGF_TYPE_IMAGE;

      if ( ( pData->m_ImageFormat )
      &&   ( pData->m_pPalette ) )
      {
        ucFileType = IGF_TYPE_IMAGE_WITH_PAL;
      }

      unsigned char   ucBpp = 8;

      switch ( pData->m_ImageFormat )
      {
        case IF_A8R8G8B8:
        case IF_X8R8G8B8:
          ucBpp = 32;
          break;
        case IF_R5G6B5:
          ucBpp = 16;
          break;
        case IF_X1R5G5B5:
          ucBpp = 15
          break;
        case IF_R8G8B8:
          ucBpp = 24;
          break;
        case IF_PALETTED:
          ucBpp = 8;
          break;
        default:
          return false;
      }

      aFile.WriteU8( ucFileType );
      aFile.WriteU8( ucBpp );
      aFile.WriteU32( 1 );

      aFile.WriteU16( pData->m_iWidth );
      aFile.WriteU16( pData->m_iHeight );
      aFile.WriteU32( 0 );

      aFile.WriteBlock( pData->m_pData, pData->DataSize() );

      if ( ucFileType == IGF_TYPE_IMAGE_WITH_PAL )
      {
        aFile.WriteU8( 0 );   // 0 bedeutet 256 Farbeinträge

        for ( int i = 0; i < 256; i++ )
        {
          aFile.WriteU8( pData->m_pPalette->GetRed( i ) );
          aFile.WriteU8( pData->m_pPalette->GetGreen( i ) );
          aFile.WriteU8( pData->m_pPalette->GetBlue( i ) );
        }
      }

      aFile.Close();

      */
      return true;
    }



    GR::String FormatDDS::GetDescription()
    {
      return "DDS Image";
    }



    GR::String FormatDDS::GetFilterString()
    {
      return "*.dds";
    }
  }
}