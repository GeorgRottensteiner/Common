#include <IO/FileStream.h>

#include <Misc/Misc.h>

#include <Grafik/Image.h>
#include <Grafik/Animation.h>
#include <Grafik/ContextDescriptor.h>

#include <Grafik/ImageFormate/FormatIFF.h>

#include <String/StringUtil.h>
#include <String/Path.h>

#include <Memory/MemoryStream.h>


GR::Graphic::FormatIFF    globalIFFPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatIFF::FormatIFF()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatIFF::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream    ioIn;
      if ( !ioIn.Open( FileName ) )
      {
        dh::Log( "file %s not found", FileName.c_str() );
        return false;
      }
      GR::u32     chunkType = ioIn.ReadU32();
      if ( chunkType != ChunkType::FORM )
      {
        // not a FORM
        return false;
      }
      return true;
    }



    ImageSet* FormatIFF::LoadSet( const GR::String& FileName )
    {
      GR::IO::FileStream    ioIn;
      if ( !ioIn.Open( FileName ) )
      {
        return NULL;
      }

      GR::u32   chunkType = ioIn.ReadU32();
      if ( chunkType != ChunkType::FORM )
      {
        // not a FORM
        return false;
      }
      GR::u32   chunkSize = ( ioIn.ReadU8() << 24 ) + ( ioIn.ReadU8() << 16 ) + ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
      // padding
      if ( chunkSize & 1 )
      {
        ++chunkSize;
      }
      if ( ioIn.GetSize() - ioIn.GetPosition() > chunkSize )
      {
        dh::Log( "IFF - FORM chunk size is greater than actual file size!" );
        return NULL;
      }

      ImageSet* pSet = new ImageSet();

      GR::Graphic::ImageData* pData = NULL;
      GR::u64                 currentPos = ioIn.GetPosition();

      while ( chunkSize > 0 )
      {
        GR::u32     chunkType = ioIn.ReadU32();
        chunkSize -= 4;

        GR::u8      compression = 0;
        GR::u8      numPlanes = 0;

        switch ( chunkType )
        {
          case ChunkType::ILBM:
            {
              pData = new GR::Graphic::ImageData();
              pSet->AddFrame( pData );

              GR::u32   innerChunkType = 0;

              while ( ( chunkSize > 0 )
              &&      ( ioIn.DataAvailable() ) )
              {
                innerChunkType = ioIn.ReadU32();
                GR::u32   origInnerChunkSize = ( ioIn.ReadU8() << 24 ) + ( ioIn.ReadU8() << 16 ) + ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                GR::u32   innerChunkSize = origInnerChunkSize;
                // padding
                if ( innerChunkSize & 1 )
                {
                  ++innerChunkSize;
                }
                GR::u64                 currentInnerPos = ioIn.GetPosition();
                chunkSize -= 8;

                switch ( innerChunkType )
                {
                  case ChunkType::BMHD:
                    {
                      GR::u16     width = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                      GR::u16     height = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                      GR::i16     x = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                      GR::i16     y = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                      numPlanes = ioIn.ReadU8();
                      GR::u8      mask = ioIn.ReadU8();
                      compression = ioIn.ReadU8();
                      GR::u8      pad1 = ioIn.ReadU8();
                      GR::u16     transparentColor = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                      GR::u8      xAspect = ioIn.ReadU8();
                      GR::u8      yAspect = ioIn.ReadU8();
                      GR::i16     pageWidth = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
                      GR::i16     pageHeight = ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();

                      pData->CreateData( width, height, GR::Graphic::ImageData::ImageFormatFromDepth( numPlanes ) );
                    }
                    break;
                  case ChunkType::CMAP:
                    {
                      int   paletteIndex = 0;
                      int   curSize = origInnerChunkSize;
                      while ( curSize >= 3 )
                      {
                        GR::u8    r = ioIn.ReadU8();
                        GR::u8    g = ioIn.ReadU8();
                        GR::u8    b = ioIn.ReadU8();

                        if ( paletteIndex < (int)pData->Palette().Entries() )
                        {
                          pData->Palette().SetColor( paletteIndex, r, g, b );
                        }

                        curSize -= 3;
                        ++paletteIndex;
                      }
                    }
                    break; 
                  case ChunkType::BODY:
                    {
                      GR::Graphic::ContextDescriptor    cdImage;

                      cdImage.Attach( pData->Width(), pData->Height(), pData->LineOffsetInBytes(), pData->ImageFormat(), pData->Data() );
                      if ( compression )
                      {
                        ByteBuffer    compressedData( origInnerChunkSize );

                        ioIn.ReadBlock( compressedData.Data(), origInnerChunkSize );

                        ByteBuffer    uncompressedData;
                        uncompressedData.Reserve( 4096 );
                        MemoryStream  memIn( compressedData );

                        GR::u8    nextByte = 0;
                        do
                        {
                          nextByte = memIn.ReadU8();
                          if ( nextByte > 128 )
                          {
                            GR::u8    valueToInsert = memIn.ReadU8();
                            for ( int j = 0; j < 257 - nextByte; ++j )
                            {
                              uncompressedData.AppendU8( valueToInsert );
                            }
                          }
                          else if ( nextByte < 128 )
                          {
                            for ( int j = 0; j < nextByte + 1; ++j )
                            {
                              uncompressedData.AppendU8( memIn.ReadU8() );
                            }
                          }
                          else
                          {
                            // End of compressed data
                            break;
                          }
                        }
                        while ( ( memIn.GetPosition() < memIn.GetSize() )
                        &&      ( nextByte != 128 ) );

                        MemoryStream  memData( uncompressedData );
                        int   numMasksPerLine = ( pData->Width() + 15 ) / 16;
                        for ( int y = 0; y < pData->Height(); ++y )
                        {
                          for ( int i = 0; i < numPlanes; ++i )
                          {
                            for ( int b = 0; b < numMasksPerLine; ++b )
                            {
                              GR::u16   bitMask = ( memData.ReadU8() << 8 ) | memData.ReadU8();
                              for ( int x = 0; x < 16; ++x )
                              {
                                if ( bitMask & ( 1 << ( 15 - x ) ) )
                                {
                                  cdImage.PutDirectPixel( b * 16 + x, y, cdImage.GetDirectPixel( b * 16 + x, y ) | ( 1 << i ) );
                                }
                              }
                            }
                          }
                        }
                      }
                      else
                      {
                        int   numMasksPerLine = ( pData->Width() + 15 ) / 16;
                        for ( int y = 0; y < pData->Height(); ++y )
                        {
                          for ( int i = 0; i < numPlanes; ++i )
                          {
                            for ( int b = 0; b < numMasksPerLine; ++b )
                            {
                              GR::u16   bitMask = ( ioIn.ReadU8() << 8 ) | ioIn.ReadU8();
                              for ( int x = 0; x < 16; ++x )
                              {
                                if ( bitMask & ( 1 << ( 15 - x ) ) )
                                {
                                  cdImage.PutDirectPixel( b * 16 + x, y, cdImage.GetDirectPixel( b * 16 + x, y ) | ( 1 << i ) );
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                    break;
                  default:
                    // skip unsupported chunks
                    dh::Log( "IFF skip unsupported inner chunk type %x", innerChunkType );
                    break;
                }
                ioIn.SetPosition( currentInnerPos + innerChunkSize );
                chunkSize -= innerChunkSize;
              }
              // make sure we're placed after the chunk
              ioIn.SetPosition( currentPos + chunkSize - 12 );
            }
            break;
          default:
            // skip unsupported chunks
            dh::Log( "IFF unsupported chunk type %x", chunkType );
            chunkSize = ( ioIn.ReadU8() << 24 ) + ( ioIn.ReadU8() << 16 ) + ( ioIn.ReadU8() << 8 ) + ioIn.ReadU8();
            ioIn.SetPosition( chunkSize, IIOStream::PT_CURRENT );
            break;
        }
      }
      return pSet;
    }



    bool FormatIFF::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( ( pData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_R5G6B5 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_X1R5G5B5 )
      ||   ( pData->ImageFormat() == GR::Graphic::IF_X8R8G8B8 ) )
      {
        return true;
      }
      return false;
    }



    bool FormatIFF::CanSave( ImageSet* pSet )
    {
      if ( pSet == NULL )
      {
        return false;
      }
      return true;
    }



    bool FormatIFF::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      GR::IO::FileStream      ioOut;

      if ( !ioOut.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      int numPlanes = GR::Graphic::ImageData::DepthFromImageFormat( pData->ImageFormat() );

      ByteBuffer    totalChunk;
      totalChunk.AppendU32( ChunkType::FORM );

      ByteBuffer    ilbmChunk;

      ilbmChunk.AppendU32( ChunkType::ILBM );


      ByteBuffer    chunkBMHD;
      chunkBMHD.Reserve( 20 );
      chunkBMHD.AppendU32( ChunkType::BMHD );
      chunkBMHD.AppendU32NetworkOrder( 20 );
      chunkBMHD.AppendU16NetworkOrder( pData->Width() );
      chunkBMHD.AppendU16NetworkOrder( pData->Height() );
      chunkBMHD.AppendI16NetworkOrder( 0 );
      chunkBMHD.AppendI16NetworkOrder( 0 );
      chunkBMHD.AppendU8( (GR::u8)numPlanes );
      chunkBMHD.AppendU8( 0 );                // mask
      chunkBMHD.AppendU8( 0 );                // compression
      chunkBMHD.AppendU8( 0 );                // pad 1
      chunkBMHD.AppendU16NetworkOrder( 0 );   // Transparentcolor
      chunkBMHD.AppendU8( 0 );                // x aspect
      chunkBMHD.AppendU8( 0 );                // y aspect
      chunkBMHD.AppendI16NetworkOrder( 0 );   // page width
      chunkBMHD.AppendI16NetworkOrder( 0 );   // page height

      ilbmChunk.AppendBuffer( chunkBMHD );


      if ( pData->HasPaletteData() )
      {
        ByteBuffer    chunkCMAP;

        chunkCMAP.AppendU32( ChunkType::CMAP );
        chunkCMAP.AppendU32NetworkOrder( ( 1 << numPlanes ) * 3 );
        chunkCMAP.Grow( ( 1 << numPlanes ) * 3, true );

        memcpy( chunkCMAP.DataAt( 8 ), pData->Palette().Data(), ( 1 << numPlanes ) * 3 );

        ilbmChunk.AppendBuffer( chunkCMAP );
      }

      ByteBuffer    chunkBody;

      int   numMasksPerLine = ( pData->Width() + 15 ) / 16;
      GR::Graphic::ContextDescriptor    cdImage;
      cdImage.Attach( pData->Width(), pData->Height(), pData->LineOffsetInBytes(), pData->ImageFormat(), pData->Data() );

      chunkBody.AppendU32( ChunkType::BODY );
      chunkBody.AppendU32NetworkOrder( pData->Height() * numMasksPerLine * 2 * numPlanes );

      for ( int y = 0; y < pData->Height(); ++y )
      {
        for ( int i = 0; i < numPlanes; ++i )
        {
          for ( int b = 0; b < numMasksPerLine; ++b )
          {
            GR::u16   bitMask = 0;
            for ( int x = 0; x < 16; ++x )
            {
              if ( cdImage.GetDirectPixel( b * 16 + x, y ) & ( 1 << i ) )
              {
                bitMask |= ( 1 << ( 15 - x ) );
              }
            }
            chunkBody.AppendU16NetworkOrder( bitMask );
          }
        }
      }
      ilbmChunk.AppendBuffer( chunkBody );

      totalChunk.AppendU32NetworkOrder( ilbmChunk.Size() );
      totalChunk.AppendBuffer( ilbmChunk );

      ioOut.WriteBlock( totalChunk.Data(), totalChunk.Size() );

      ioOut.Close();

      return true;
    }



    bool FormatIFF::Save( const GR::String& FileName, ImageSet* pSet )
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



    GR::String FormatIFF::GetDescription()
    {
      return "IFF Image";
    }



    GR::String FormatIFF::GetFilterString()
    {
      return "*.IFF";
    }

  }
}