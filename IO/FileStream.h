#ifndef FILE_STREAM_H
#define FILE_STREAM_H



#include <OS/OS.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <windows.h>
#endif

#if OPERATING_SYSTEM == OS_WEB
#include <emscripten.h>
#endif

#ifdef TANDEM_GUARDIAN
#include <cextdecs.h>
#endif

#if OPERATING_SYSTEM == OS_ANDROID
#include <android/asset_manager.h>
#endif

#include <Interface/IIOStream.h>
#include <Interface/ISmartPointer.h>

#include <Memory/ByteBuffer.h>

#include <Lang/RefCountObject.h>


namespace GR
{

  namespace IO
  {

    class FileStream : public IIOStreamBase
    {

      protected:

        int                       m_LastError;
    
        size_t                    m_CacheSize;




      public:


        FileStream();

        FileStream( const GR::Char* FileName, IIOStream::OpenType oType = OT_DEFAULT );

        FileStream( const GR::String& FileName, IIOStream::OpenType oType = OT_DEFAULT );

        virtual bool              IsGood();

        virtual bool              Open( const GR::Char* FileName, IIOStream::OpenType = OT_DEFAULT );

        virtual bool              Open( const GR::String& FileName, IIOStream::OpenType = OT_DEFAULT );


#if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
        bool                      OpenTandem( IIOStream::OpenType OType,
                                              const char* Filename,
                                              short FileCode,
                                              short PrimaryExtentSize,
                                              short SecondaryExtentSize,
                                              short MaximumExtents,
                                              short FileType,
                                              short Options,
                                              short RecordLen,
                                              short BlockLen,
                                              short KeyLen,
                                              short KeyOffset,
                                              bool Buffered );

        short                     GetFileError() const;

        short                     GetFileCode();
        short                     GetFileType();

        virtual bool              ReadLine( GR::String& Result );
        virtual bool              ReadLine( GR::WString& Result );
        virtual bool              ReadLine( char* pTarget, unsigned long MaxReadLength );

#endif

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

        class FileStreamImpl : public GR::RefCountObject
        {

          public:

            ByteBuffer        m_Cache;

            GR::i64           m_CacheBytesUsed;
            GR::i64           m_FilePosAfterCache;
            GR::u64           m_PseudoFilePos;
            GR::u64           m_CachedFileSize;



            #if OPERATING_SYSTEM == OS_WINDOWS
            HANDLE            m_Handle;

            FileStreamImpl( HANDLE hFile ) :
              m_FilePosAfterCache( 0 ),
              m_CacheBytesUsed( 0 ),
              m_PseudoFilePos( 0 ),
              m_CachedFileSize( (GR::u64)-1 )
            {
              m_Handle = hFile;
            }

            void Close()
            {
              if ( m_Handle == INVALID_HANDLE_VALUE )
              {
                return;
              }

              CloseHandle( m_Handle );
              m_Handle = INVALID_HANDLE_VALUE;
              m_Cache.Clear();
              m_FilePosAfterCache = 0;
              m_PseudoFilePos     = 0;
            }

            #elif OPERATING_SYSTEM == OS_ANDROID
            AAsset*           m_Handle;

            FileStreamImpl( AAsset* hFile ) :
              m_CacheBytesUsed( 0 ),
              m_FilePosAfterCache( 0 ),
              m_PseudoFilePos( 0 ),
              m_CachedFileSize( (GR::u64)-1 )
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
              m_PseudoFilePos = 0;
            }

            #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN

            short         m_Handle;

            bool          m_EditFileOpened;
            bool          m_LFNeedsToBeAdded;

            FileStreamImpl( short hFile ) :
              m_FilePosAfterCache( 0 ),
              m_CacheBytesUsed( 0 ),
              m_PseudoFilePos( 0 ),
              m_EditFileOpened( false ),
              m_LFNeedsToBeAdded( false ),
              m_CachedFileSize( (GR::u64)-1 )
            {
              m_Handle = hFile;
            }

            void Close()
            {
              if ( m_Handle < 0 )
              {
                return;
              }
              if ( m_EditFileOpened )
              {
                CLOSEEDIT_( m_Handle );
              }
              else
              {
                FILE_CLOSE_( m_Handle );
              }
              m_Handle = -1;
              m_Cache.Clear();
              m_FilePosAfterCache = 0;
              m_PseudoFilePos     = 0;
              m_EditFileOpened    = false;
            }

            #else

            FILE*             m_Handle;

            FileStreamImpl( FILE* hFile ) :
              m_CacheBytesUsed( 0 ),
              m_FilePosAfterCache( 0 ),
              m_PseudoFilePos( 0 ),
              m_CachedFileSize( (GR::u64)-1 )
            {
              m_Handle = hFile;
            }

            void Close()
            {
              if ( m_Handle == NULL )
              {
                return;
              }

              fclose( m_Handle );
              m_Handle = NULL;
              m_Cache.Clear();
              m_FilePosAfterCache = 0;
              m_PseudoFilePos     = 0;

#if OPERATING_SYSTEM == OS_WEB
              //if ( ( m_OpenType == IIOStream::OT_WRITE_ONLY )
              //||   ( m_OpenType == IIOStream::OT_READ_WRITE )
              //||   ( m_OpenType == IIOStream::OT_WRITE_APPEND ) )
              {
                MAIN_THREAD_EM_ASM(
                  FS.syncfs( false, function( err )
                  {
                    //if ( err ) throw err;
                    // handle callback
                  } );
                );
              }
#endif
            }
            #endif


            ~FileStreamImpl()
            {
              Close();
            }


        };

        ISmartPointer<FileStreamImpl>     m_Impl;

    };


  }

}



#endif // FILE_STREAM_H



