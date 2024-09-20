#ifndef GUARDIAN_FILE_H
#define GUARDIAN_FILE_H

#include <OS/OS.h>

#if OPERATING_SYSTEM == OS_TANDEM

#include <tal.h>
#include <cextdecs.h>

#include <Interface/IIOStream.h>
#include <Interface/ISmartPointer.h>

#include <Memory/ByteBuffer.h>

#include <Lang/RefCountObject.h>


namespace GR
{

  namespace IO
  {

    class GuardianFile : public IIOStreamBase
    {

      protected:

        size_t                    m_CacheSize;


        short                     m_LastError;


      public:


        GuardianFile();

        GuardianFile( const GR::Char* szFileName, IIOStream::OpenType oType = OT_DEFAULT );

        virtual ~GuardianFile();

        virtual bool              IsGood();

        virtual bool              DataAvailable();

        virtual bool              Open( const GR::Char* szFileName, IIOStream::OpenType = OT_DEFAULT );

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
                                              bool Buffered,
                                              bool OddUnstructured = true );

        virtual GR::i32           LastError() const;

        short                     GetFileError() const;

        virtual void              Close();

        virtual bool              Flush();

        void                      SetCacheSize( size_t Size );

        virtual short             GetHandle();

        virtual GR::u64           GetSize();

        GR::u64                   GetNumberOfRecords();

        virtual bool              ReadLine( GR::String& Result );
        virtual bool              ReadLine( GR::WString& Result );
        virtual bool              ReadLine( char* pTarget, unsigned long MaxReadLength );

        virtual unsigned long     ReadBlock( void* pTarget, GR::up CountBytes );

        virtual unsigned long     WriteBlock( const void* pSource, GR::up CountBytes );

        virtual unsigned long     SetPosition( GR::i64 Offset, PositionType = PT_SET );

        virtual GR::u64           GetPosition();
    
        bool                      HadReadError();


      private:

        bool                    ReadToBuffer( void* pTarget, unsigned long BytesToRead, unsigned long& BytesRead );

        class GuardianFileImpl : public GR::RefCountObject
        {

          public:

            ByteBuffer        m_Cache;

            GR::i64           m_CacheBytesUsed;
            GR::i64           m_FilePosAfterCache;
            GR::i64           m_PseudoFilePos;

            short             m_Handle;
            bool              m_EditFileOpened;



            GuardianFileImpl( short hFile ) :
              m_FilePosAfterCache( 0 ),
              m_PseudoFilePos( 0 ),
              m_CacheBytesUsed( 0 ),
              m_EditFileOpened( false )
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
              m_CacheBytesUsed    = 0;
              m_EditFileOpened    = false;
            }

            ~GuardianFileImpl()
            {
              Close();
            }


        };

        ISmartPointer<GuardianFileImpl>     m_Impl;

    };


  }

}


#endif // OS_TANDEM

#endif // GUARDIAN_FILE_H



