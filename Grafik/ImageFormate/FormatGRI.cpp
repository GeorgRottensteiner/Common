#include <IO/FileChunk.h>
#include <IO/FileStream.h>

#include <Misc/Misc.h>

#include <Grafik/Image.h>
#include <Grafik/Animation.h>

#include <Grafik\ImageFormate\FormatGRI.h>

#include <String/Path.h>
#include <String/StringUtil.h>

#include <GR/Gamebase/FileChunks.h>



GR::Graphic::FormatGRI    globalGRIPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatGRI::FormatGRI()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatGRI::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream    aFile;
      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        dh::Log( "file %s not found", FileName.c_str() );
        return false;
      }
      GR::u16  chunkType = aFile.ReadU16();

      aFile.Close();

      return chunkType == GR::Gamebase::FileChunks::IMAGE_CONTAINER;
    }



    ImageSet* FormatGRI::LoadSet( const GR::String& FileName )
    {
      if ( Path::Extension( FileName ).ToUpper() != "GRI" )
      {
        return NULL;
      }

      GR::IO::FileStream    aFile;
      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      GR::IO::FileChunk   chunk;

      if ( !chunk.Read( aFile ) )
      {
        return NULL;
      }
      if ( chunk.Type() != GR::Gamebase::FileChunks::IMAGE_CONTAINER )
      {
        return NULL;
      }

      ImageSet* pSet = new ImageSet();

      auto chunkReader = chunk.GetMemoryStream();
      GR::IO::FileChunk   subChunk;

      while ( subChunk.Read( chunkReader ) )
      {
        switch ( subChunk.Type() )
        {
          case GR::Gamebase::FileChunks::IMAGE_FRAME:
            {
              GR::Graphic::ImageData* pData = new GR::Graphic::ImageData();

              pSet->AddFrame( pData );

              auto frameReader = subChunk.GetMemoryStream();
              GR::IO::FileChunk   chunkFrame;

              while ( chunkFrame.Read( frameReader ) )
              {
                auto subFrameReader = chunkFrame.GetMemoryStream();

                switch ( chunkFrame.Type() )
                {
                  case GR::Gamebase::FileChunks::IMAGE_DATA:
                    {
                      GR::u32     width   = subFrameReader.ReadU32();
                      GR::u32     height  = subFrameReader.ReadU32();
                      GR::Graphic::eImageFormat   format = (GR::Graphic::eImageFormat)subFrameReader.ReadU32();

                      pData->CreateData( width, height, format );

                      subFrameReader.ReadBlock( pData->Data(), pData->DataSize() );

                      if ( format == GR::Graphic::IF_A8R8G8B8 )
                      {
                        // gather mask image from alpha channel
                        GR::Graphic::ImageData* pMask = new GR::Graphic::ImageData();

                        pMask->CreateData( width, height, GR::Graphic::IF_INDEX8 );

                        for ( GR::u32 j = 0; j < height; ++j )
                        {
                          GR::u8* pLineData = (GR::u8*)pData->GetRowData( j );
                          GR::u8* pMaskLineData = (GR::u8*)pMask->GetRowData( j );

                          for ( GR::u32 i = 0; i < width; ++i )
                          {
                            pMaskLineData[i] = pLineData[3 + i * 4];
                          }
                        }
                        //pData->ConvertSelfTo( GR::Graphic::IF_X8R8G8B8 );
                        pSet->Frames.back()->Layers[0]->pImageDataMask = pMask;
                      }
                    }
                    break;
                    /*
                  case GR::Gamebase::FileChunks::IMAGE_MASK_DATA:
                    {
                      GR::u32     width   = subFrameReader.ReadU32();
                      GR::u32     height  = subFrameReader.ReadU32();
                      GR::Graphic::eImageFormat   format = (GR::Graphic::eImageFormat)subFrameReader.ReadU32();

                      pSet->Frames.back()->Layers[0]->pImageDataMask = new GR::Graphic::ImageData();
                      pSet->Frames.back()->Layers[0]->pImageDataMask->CreateData( width, height, format );
                      subFrameReader.ReadBlock( pSet->Frames.back()->Layers[0]->pImageDataMask->Data(), pSet->Frames.back()->Layers[0]->pImageDataMask->DataSize() );
                    }
                    break;*/
                  case GR::Gamebase::FileChunks::PALETTE:
                    {
                      GR::u32     numEntries = subFrameReader.ReadU32();
                      pData->Palette().Create( numEntries );
                      for ( int i = 0; i < (int)numEntries; ++i )
                      {
                        GR::u8   r = subFrameReader.ReadU8();
                        GR::u8   g = subFrameReader.ReadU8();
                        GR::u8   b = subFrameReader.ReadU8();
                        pData->Palette().SetColor( (size_t)i, r, g, b );
                      }
                    }
                    break;
                  default:
                    dh::Log( "FormatGRI unsupported image frame chunk %x\n", chunkFrame.Type() );
                    break;
                }
              }
            }
            break;
          default:
            dh::Log( "FormatGRI unsupported image container chunk %x\n", subChunk.Type() );
            break;
        }
      }
      return pSet;
    }



    bool FormatGRI::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( ( pData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_R5G6B5 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_INDEX1 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_INDEX2 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_INDEX4 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_X1R5G5B5 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_X8R8G8B8 ) )
      {
        return true;
      }
      return false;
    }



    bool FormatGRI::CanSave( ImageSet* pSet )
    {
      if ( pSet == NULL )
      {
        return false;
      }
      return true;
    }



    GR::IO::FileChunk FormatGRI::ImageToChunk( GR::Graphic::ImageData& ImageData, GR::u16 FileChunk,
                                               GR::Graphic::ImageData* pMaskData, GR::u16 FileChunkMask, 
                                               GR::f32 FrameDelay )
    {
      GR::IO::FileChunk    chunkImage( GR::Gamebase::FileChunks::IMAGE_CONTAINER );

      GR::IO::FileChunk    chunkImageFrame( GR::Gamebase::FileChunks::IMAGE_FRAME );

      GR::IO::FileChunk    chunkImageData( FileChunk );

      GR::Graphic::ImageData* pImageData = &ImageData;
      GR::Graphic::ImageData  alphadImageData;

      // implant mask in alpha channel
      if ( pMaskData != NULL )
      {
        alphadImageData = ImageData;
        alphadImageData.ConvertSelfTo( GR::Graphic::IF_A8R8G8B8 );

        for ( int j = 0; j < ImageData.Height(); ++j )
        {
          GR::u8* pLineData = (GR::u8*)alphadImageData.GetRowData( j );
          GR::u8* pMaskLineData = (GR::u8*)pMaskData->GetRowData( j );

          for ( int i = 0; i < ImageData.Width(); ++i )
          {
            pLineData[3 + i * 4] = pMaskLineData[i];
          }
        }
        pImageData = &alphadImageData;
      }

      chunkImageData.AppendU32( pImageData->Width() );
      chunkImageData.AppendU32( pImageData->Height() );
      chunkImageData.AppendU32( (GR::u32)pImageData->ImageFormat() );
      chunkImageData.AppendData( pImageData->Data(), pImageData->DataSize() );
      chunkImageFrame.AppendChunk( chunkImageData );

      if ( ( pImageData->ImageFormat() )
      &&   ( pImageData->Palette().Data() ) )
      {
        // max 256 colors, 0 = 256
        GR::IO::FileChunk    chunkPalette( GR::Gamebase::FileChunks::PALETTE );
        chunkPalette.AppendU32( (GR::u32)pImageData->Palette().Entries() );
        for ( size_t i = 0; i < pImageData->Palette().Entries(); i++ )
        {
          chunkPalette.AppendU8( pImageData->Palette().Red( i ) );
          chunkPalette.AppendU8( pImageData->Palette().Green( i ) );
          chunkPalette.AppendU8( pImageData->Palette().Blue( i ) );
        }
        chunkImageFrame.AppendChunk( chunkPalette );
      }
      /*
      if ( pMaskData != NULL )
      { 
        GR::IO::FileChunk    chunkImageData( FileChunkMask );

        chunkImageData.AppendU32( pMaskData->Width() );
        chunkImageData.AppendU32( pMaskData->Height() );
        chunkImageData.AppendU32( (GR::u32)pMaskData->ImageFormat() );
        chunkImageData.AppendData( pMaskData->Data(), pMaskData->DataSize() );
        chunkImageFrame.AppendChunk( chunkImageData );
      }*/

      if ( FrameDelay > 0.0f )
      {
        GR::IO::FileChunk    chunkAnimationInfo( GR::Gamebase::FileChunks::ANIMATION_INFO );
        chunkAnimationInfo.AppendF32( FrameDelay );
        chunkImageFrame.AppendChunk( chunkAnimationInfo );
      }

      chunkImage.AppendChunk( chunkImageFrame );

      return chunkImage;
    }



    bool FormatGRI::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      GR::IO::FileChunk   chunkImage( ImageToChunk( *pData, GR::Gamebase::FileChunks::IMAGE_DATA, pMask, GR::Gamebase::FileChunks::IMAGE_MASK_DATA ) );

      chunkImage.Write( aFile );
      aFile.Close();

      return true;
    }



    bool FormatGRI::Save( const GR::String& FileName, ImageSet* pSet )
    {
      if ( ( FileName.empty() )
      ||   ( pSet == NULL ) )
      {
        return false;
      }
      if ( pSet->FrameCount() == 1 )
      {
        return Save( FileName, pSet->Image( 0, 0 ), pSet->Mask( 0, 0 ) );
      }


      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      for ( size_t frame = 0; frame < pSet->FrameCount(); ++frame )
      {
        GR::IO::FileChunk   chunkImage( ImageToChunk( *pSet->Image( frame, 0 ), GR::Gamebase::FileChunks::IMAGE_DATA, 
                                            pSet->Mask( frame, 0 ), GR::Gamebase::FileChunks::IMAGE_MASK_DATA,
                                            pSet->Frame( frame )->DelayMS * 0.001f  ) );
        chunkImage.Write( aFile );
      }
      aFile.Close();

      return true;
    }



    GR::String FormatGRI::GetDescription()
    {
      return "GRI Image";
    }



    GR::String FormatGRI::GetFilterString()
    {
      return "*.gri";
    }

  }
}