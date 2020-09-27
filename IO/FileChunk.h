#ifndef FILECHUNK_H
#define FILECHUNK_H


#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>
#include <Memory/MemoryStream.h>

#include <Interface/IIOStream.h>

#include <string>



namespace GR
{
  namespace IO
  {

    class FileChunk : public ByteBuffer,
                      public IIOStream
    {

      public:

	      FileChunk();
        FileChunk( GR::u16 wChunk );

        bool                  Read( IIOStream& aFile );
        bool                  Write( IIOStream& aFile );

        GR::u16               Type() const;

        MemoryStream          GetMemoryStream() const;

        void                  AppendChunk( const FileChunk& Buffer );

        bool                  ToBuffer( ByteBuffer& TargetBuffer );
        ByteBuffer            ToBuffer();


        virtual unsigned long ReadBlock( void* pData, std::size_t NumBytes );
        virtual unsigned long WriteBlock( const void* pData, std::size_t NumBytes );


      protected:

        std::size_t           m_ReadPos;

        GR::u16               m_Type;


    };

  }
}

#endif // FILECHUNK_H



