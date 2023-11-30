#include <IO/FileStream.h>
#include <Memory/MemoryStream.h>

#include "FileChunk.h"


namespace GR
{
  namespace IO
  {

    FileChunk::FileChunk()
      : m_Type( 0 ),
        m_ReadPos( 0 )
    {
    }



    FileChunk::FileChunk( GR::u16 ChunkType )
      : m_Type( ChunkType ),
        m_ReadPos( 0 )
    {
    }



    bool FileChunk::Read( IIOStream& aFile )
    {
      Clear();
      // reset reader stats
      m_ReadPos     = 0;
      m_ReadFailed  = false;

      m_Type = aFile.ReadU16();

      GR::u32 size = aFile.ReadU32();
      if ( aFile.Error() )
      {
        return false;
      }
      Resize( size );

      aFile.ReadBlock( m_pData, size );
      return true;
    }



    bool FileChunk::Write( IIOStream& aFile )
    {
      aFile.WriteU16( m_Type );
      aFile.WriteU32( (GR::u32)Size() );
      if ( Size() )
      {
        aFile.WriteBlock( m_pData, Size() );
      }
      return true;
    }



    bool FileChunk::ToBuffer( ByteBuffer& TargetBuffer )
    {
      TargetBuffer.Resize( Size() + 6 );

      TargetBuffer.SetU16At( 0, m_Type );
      TargetBuffer.SetU32At( 2, (GR::u32)Size() );
      TargetBuffer.SetDataAt( 6, m_pData, Size() );
      return true;
    }



    ByteBuffer FileChunk::ToBuffer()
    {
      ByteBuffer    result( Size() + 6 );

      result.SetU16At( 0, m_Type );
      result.SetU32At( 2, (GR::u32)Size() );
      result.SetDataAt( 6, m_pData, Size() );
      return result;
    }



    GR::u16 FileChunk::Type() const
    {
      return m_Type;
    }



    MemoryStream FileChunk::GetMemoryStream() const
    {
      return MemoryStream( m_pData, Size() );
    }



    void FileChunk::AppendChunk( const FileChunk& Chunk )
    {
      AppendU16( Chunk.Type() );
      AppendU32( (GR::u32)Chunk.Size() );
      AppendData( Chunk.Data(), Chunk.Size() );
    }



    bool FileChunk::DataAvailable()
    {
      return m_ReadPos < Size();
    }



    unsigned long FileChunk::ReadBlock( void* pData, std::size_t NumBytes )
    {
      if ( m_ReadPos >= Size() )
      {
        // reached the end
        m_ReadFailed = true;
        return 0;
      }
      if ( m_ReadPos + NumBytes > Size() )
      {
        memcpy( pData, DataAt( m_ReadPos ), Size() - NumBytes );
        m_ReadPos += Size() - NumBytes;
        return (GR::u32)( Size() - NumBytes );
      }
      memcpy( pData, DataAt( m_ReadPos ), NumBytes );
      m_ReadPos += NumBytes;
      return (GR::u32)NumBytes;
    }



    unsigned long FileChunk::WriteBlock( const void* pData, std::size_t NumBytes )
    {
      AppendData( pData, NumBytes );
      return (GR::u32)NumBytes;
    }


  }
}