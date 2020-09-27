#include <IO/FileStream.h>

#include <Misc/Misc.h>

#include <Grafik/Image.h>
#include <Grafik/Animation.h>

#include <Grafik\ImageFormate\FormatIGF.h>

#include <String/StringUtil.h>



GR::Graphic::FormatIGF    globalIGFPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatIGF::FormatIGF()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatIGF::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream    aFile;
      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        dh::Log( "file %s not found", FileName.c_str() );
        return false;
      }
      GR::u8  bType = aFile.ReadU8();

      aFile.Close();

      if ( ( bType == GR::Graphic::IGFType::IMAGE )
           // &&   ( bType != IGF_TYPE_ANIMATION )   // da hier nur Grafiken eingeladen werden sollen,
           // &&   ( bType != IGF_TYPE_FONT )        // macht es keinen Sinn, diese Formate als erkannt
           // &&   ( bType != IGF_TYPE_PALETTE )     // gelten zu lassen
           || ( bType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST )
           || ( bType == GR::Graphic::IGFType::IMAGE_WITH_PALETTE )
           || ( bType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST_AND_PALETTE ) )
      {
        return true;
      }
      return false;
    }



    ImageSet* FormatIGF::LoadSet( const GR::String& FileName )
    {
      size_t   iLength = FileName.length();
      if ( iLength >= 3 )
      {
        if ( ( toupper( FileName[iLength - 3] ) != 'I' )
             || ( toupper( FileName[iLength - 2] ) != 'G' )
             || ( toupper( FileName[iLength - 1] ) != 'F' ) )
        {
          return NULL;
        }
      }
      else
      {
        return NULL;
      }

      GR::IO::FileStream    aFile;
      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        return NULL;
      }
      GR::Graphic::ImageData* pData = new GR::Graphic::ImageData();

      GR::u8    bType = aFile.ReadU8();

      if ( ( bType == GR::Graphic::IGFType::IMAGE )
           || ( bType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST )
           || ( bType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST_AND_PALETTE )
           || ( bType == GR::Graphic::IGFType::IMAGE_WITH_PALETTE ) )
      {
        GR::u8    ucBpp = aFile.ReadU8();
        GR::u32   dwCount = aFile.ReadU32();

        GR::u16   wWidth = aFile.ReadU16();
        GR::u16   wHeight = aFile.ReadU16();
        GR::u32   dwTransparent = aFile.ReadU32();

        GR::Graphic::eImageFormat    imgFormat = GR::Graphic::IF_UNKNOWN;

        switch ( ucBpp )
        {
          case 1:
            imgFormat = GR::Graphic::IF_INDEX1;
            break;
          case 2:
            imgFormat = GR::Graphic::IF_INDEX2;
            break;
          case 4:
            imgFormat = GR::Graphic::IF_INDEX4;
            break;
          case 8:
            imgFormat = GR::Graphic::IF_PALETTED;
            break;
          case 15:
            imgFormat = GR::Graphic::IF_X1R5G5B5;
            break;
          case 16:
            imgFormat = GR::Graphic::IF_R5G6B5;
            break;
          case 24:
            imgFormat = GR::Graphic::IF_R8G8B8;
            break;
          case 32:
            imgFormat = GR::Graphic::IF_X8R8G8B8;
            break;
          default:
            dh::Log( "Unsupported BPP for IGF: %d", ucBpp );
            break;
        }

        pData->CreateData( wWidth, wHeight, imgFormat );

        aFile.ReadBlock( pData->Data(), pData->DataSize() );

        if ( bType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST )
        {
          GR::u32 CompressListSize = aFile.ReadU32(); // Compresslist-Size
          aFile.SetPosition( CompressListSize, IIOStream::PT_CURRENT );
        }

        if ( ( bType == GR::Graphic::IGFType::IMAGE_WITH_PALETTE )
             || ( bType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST_AND_PALETTE ) )
        {
          aFile.SetPosition( 1, IIOStream::PT_CURRENT );

          pData->Palette().LoadAt( aFile );
        }
        else
        {
          pData->SetPalette();
        }
        aFile.Close();
      }

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pData );

      return pSet;
    }



    bool FormatIGF::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( ( pData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 )
           || ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
           || ( pData->ImageFormat() == GR::Graphic::IF_R5G6B5 )
           || ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
           || ( pData->ImageFormat() == GR::Graphic::IF_X1R5G5B5 )
           || ( pData->ImageFormat() == GR::Graphic::IF_X8R8G8B8 ) )
      {
        return true;
      }
      return false;
    }



    bool FormatIGF::CanSave( ImageSet* pSet )
    {
      if ( pSet == NULL )
      {
        return false;
      }
      return true;
    }



    bool FormatIGF::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      unsigned char   ucFileType = GR::Graphic::IGFType::IMAGE;

      if ( ( pData->ImageFormat() )
           && ( pData->Palette().Data() ) )
      {
        ucFileType = GR::Graphic::IGFType::IMAGE_WITH_PALETTE;
      }

      unsigned char   ucBpp = 8;

      switch ( pData->ImageFormat() )
      {
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_X8R8G8B8:
          ucBpp = 32;
          break;
        case GR::Graphic::IF_R5G6B5:
          ucBpp = 16;
          break;
        case GR::Graphic::IF_X1R5G5B5:
          ucBpp = 15;
          break;
        case GR::Graphic::IF_R8G8B8:
          ucBpp = 24;
          break;
        case GR::Graphic::IF_PALETTED:
          ucBpp = 8;
          break;
        default:
          return false;
      }

      aFile.WriteU8( ucFileType );
      aFile.WriteU8( ucBpp );
      aFile.WriteU32( 1 );

      aFile.WriteU16( pData->Width() );
      aFile.WriteU16( pData->Height() );
      aFile.WriteU32( 0 );

      aFile.WriteBlock( pData->Data(), pData->DataSize() );

      if ( ucFileType == GR::Graphic::IGFType::IMAGE_WITH_PALETTE )
      {
        aFile.WriteU8( 0 );   // 0 bedeutet 256 Farbeinträge

        for ( int i = 0; i < 256; i++ )
        {
          aFile.WriteU8( pData->Palette().Red( i ) );
          aFile.WriteU8( pData->Palette().Green( i ) );
          aFile.WriteU8( pData->Palette().Blue( i ) );
        }
      }

      aFile.Close();

      return true;
    }



    bool FormatIGF::Save( const GR::String& FileName, ImageSet* pSet )
    {
      if ( ( FileName.empty() )
      ||   ( pSet == NULL ) )
      {
        return false;
      }
      if ( pSet->FrameCount() == 1 )
      {
        return Save( FileName, pSet->Image( 0, 0 ) );
      }

      GR::Animation     tempAni;

      for ( size_t iFrame = 0; iFrame < pSet->FrameCount(); ++iFrame )
      {
        tempAni.AddFrame( new GR::Graphic::Image( *pSet->Image( iFrame, 0 ) ), pSet->Frame( iFrame )->DelayMS * 0.001f );
      }
      tempAni.Save( FileName );

      return true;
    }



    GR::String FormatIGF::GetDescription()
    {
      return "IGF Image";
    }



    GR::String FormatIGF::GetFilterString()
    {
      return "*.igf";
    }

  }
}