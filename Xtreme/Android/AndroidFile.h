#ifndef ANDROID_FILE_STREAM_H
#define ANDROID_FILE_STREAM_H



#include <OS/OS.h>

#include <Interface/IIOStream.h>
#include <Interface/ISmartPointer.h>

#include <Memory/ByteBuffer.h>

#include <Lang/RefCountObject.h>

#include <android/asset_manager.h>


namespace GR
{

  namespace IO
  {

    class AndroidFile : public IIOStreamBase
    {

      protected:

        size_t                    m_CacheSize;


        int                       m_LastError;


      public:


        AndroidFile();

        AndroidFile( const GR::Char* FileName, IIOStream::OpenType oType = OT_DEFAULT );

        AndroidFile( const GR::String& FileName, IIOStream::OpenType oType = OT_DEFAULT );

        virtual ~AndroidFile();

        virtual bool              IsGood();

        virtual bool              Open( const GR::Char* FileName, IIOStream::OpenType = OT_DEFAULT );

        virtual bool              Open( const GR::String& FileName, IIOStream::OpenType = OT_DEFAULT );


        virtual GR::i32           LastError() const;

        virtual void              Close();

        virtual bool              Flush();

        void                      SetCacheSize( size_t Size );

    #if OPERATING_SYSTEM == OS_WINDOWS
        virtual HANDLE            GetHandle();
    #elif OPERATING_SYSTEM == OS_ANDROID
        virtual AAsset*           GetHandle();
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
        virtual short             GetHandle();
    #else
        virtual FILE*             GetHandle();
    #endif

        virtual GR::u64           GetSize();

        virtual unsigned long     ReadBlock( void* pTarget, size_t CountBytes );

        virtual unsigned long     WriteBlock( const void* pSource, size_t CountBytes );

        virtual unsigned long     SetPosition( GR::i64 Offset, PositionType = PT_SET );

        virtual GR::u64           GetPosition();

        virtual bool              DataAvailable();


      private:

        bool                    ReadToBuffer( void* pTarget, unsigned long BytesToRead, unsigned long& BytesRead );

        class AndroidFileImpl : public GR::CRefCountObject
        {

          public:

            ByteBuffer        m_Cache;

            GR::i64           m_CacheBytesUsed;
            GR::i64           m_FilePosAfterCache;
            GR::i64           m_PseudoFilePos;

            AAsset*           m_Handle;

            AndroidFileImpl( AAsset* hFile ) :
              m_FilePosAfterCache( 0 ),
              m_CacheBytesUsed( 0 ),
              m_PseudoFilePos( 0 )
            {
              m_Handle = hFile;
            }

            void Close()
            {
              if ( m_Handle == NULL )
              {
                return;
              }

              AAsset_close( m_Handle );
              m_Handle = NULL;
              m_Cache.Clear();
              m_FilePosAfterCache = 0;
              m_PseudoFilePos     = 0;
            }

            ~AndroidFileImpl()
            {
              Close();
            }

        };

        ISmartPointer<AndroidFileImpl>     m_Impl;

    };


  }

}



#endif // ANDROID_FILE_STREAM_H



