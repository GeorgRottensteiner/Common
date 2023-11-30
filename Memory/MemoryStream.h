#ifndef MEMORY_STREAM_H
#define MEMORY_STREAM_H



#include <Interface/IIOStream.h>

#include <Memory/ByteBuffer.h>

#include <OS/OS.h>



class MemoryStream : public IIOStreamBase
{

  protected:

    const void*               m_pData;

    size_t                    m_Size,
                              m_CurrentPos;

    ByteBuffer                m_Data;

    ByteBuffer*               m_pDataTarget;


  public:


    MemoryStream();

    MemoryStream( const void* pData, size_t Length, IIOStream::OpenType oType = OT_DEFAULT );
    MemoryStream( ByteBuffer& Target, IIOStream::OpenType oType = OT_DEFAULT );
    MemoryStream( const ByteBuffer& Target, IIOStream::OpenType oType = OT_DEFAULT );


    virtual bool              IsGood();
    virtual bool              DataAvailable();

#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn (1319)
#endif    
    
    virtual bool              Open( const void* pData, size_t Length, IIOStream::OpenType = OT_DEFAULT );
    virtual bool              Open( const ByteBuffer& Target, IIOStream::OpenType = OT_DEFAULT );
    virtual bool              Open( ByteBuffer& Target, IIOStream::OpenType = OT_DEFAULT );

    bool                      Open( OpenType oType = OT_WRITE_ONLY );

    virtual void              Close();

    virtual bool              Flush();

    const void*               Data();

    const void*               CurrentData();

    ByteBuffer                Buffer() const;

    virtual GR::u64           GetSize();

    virtual unsigned long     ReadBlock( void* pTarget, size_t ulSize );

    virtual unsigned long     WriteBlock( const void* pSource, size_t ulSize );

    virtual unsigned long     SetPosition( GR::i64 slOffset, PositionType = PT_SET );

    virtual GR::u64           GetPosition();

};



#endif // MEMORY_STREAM_H



