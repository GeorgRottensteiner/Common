#include <Memory\MemoryStream.h>

#include <debug/debugclient.h>



MemoryStream::MemoryStream() :
  IIOStreamBase(),
  m_pData( NULL ),
  m_Size( 0 ),
  m_CurrentPos( 0 ),
  m_pDataTarget( NULL )
{
}



MemoryStream::MemoryStream( const void* pData, size_t Size, IIOStream::OpenType Type ) :
  IIOStreamBase(),
  m_pData( NULL ),
  m_Size( 0 ),
  m_CurrentPos( 0 ),
  m_pDataTarget( NULL )
{
  Open( pData, Size, Type );
}



MemoryStream::MemoryStream( ByteBuffer& Target, IIOStream::OpenType Type ) :
  IIOStreamBase(),
  m_pData( NULL ),
  m_Size( 0 ),
  m_CurrentPos( 0 ),
  m_pDataTarget( NULL )
{
  Open( Target, Type );
}



MemoryStream::MemoryStream( const ByteBuffer& Target, IIOStream::OpenType Type ) :
  IIOStreamBase(),
  m_pData( NULL ),
  m_Size( 0 ),
  m_CurrentPos( 0 ),
  m_pDataTarget( NULL )
{
  Open( Target, Type );
}



void MemoryStream::Close()
{
  m_pData       = NULL;
  m_Size        = 0;
  m_CurrentPos  = 0;

  IIOStreamBase::Close();

  m_OpenType    = OT_CLOSED;
}



bool MemoryStream::Open( OpenType Type )
{
  if ( Type != IIOStream::OT_WRITE_ONLY )
  {
    return false;
  }
  return Open( NULL, 0, Type );
}



bool MemoryStream::Open( const void* pData, size_t Size, OpenType Type )
{
  if ( m_pData )
  {
    return false;
  }

  m_OpenType  = Type;

  switch ( m_OpenType )
  {
    case OT_READ_ONLY:
      m_pData       = pData;
      m_Size        = Size;
      m_CurrentPos  = 0;
      break;
    case OT_WRITE_ONLY:
      m_Data.Clear();
      m_Size        = 0;
      m_CurrentPos  = 0;
      m_Opened      = true;
      return true;
    default:
      return false;
  }

  if ( m_pData )
  {
    m_Opened = true;
  }

  return ( !!m_pData );
}



bool MemoryStream::Open( ByteBuffer& Target, OpenType Type )
{
  if ( m_pData )
  {
    return false;
  }

  m_OpenType  = Type;

  switch ( m_OpenType )
  {
    case OT_READ_ONLY:
      m_pData       = Target.Data();
      m_Size        = Target.Size();
      m_CurrentPos  = 0;
      break;
    case OT_WRITE_ONLY:
      m_pDataTarget = &Target;
      m_Size        = 0;
      m_CurrentPos  = 0;
      m_Opened      = true;
      return true;
    default:
      return false;
  }

  if ( ( m_pData )
  ||   ( m_pDataTarget ) )
  {
    m_Opened = true;
  }

  return m_Opened;
}



bool MemoryStream::Open( const ByteBuffer& Target, OpenType Type )
{
  if ( m_pData )
  {
    return false;
  }

  if ( ( Type != OT_READ_ONLY )
  &&   ( Type != OT_READ_ONLY_SHARED ) )
  {
    return false;
  }

  m_OpenType  = Type;

  m_pData      = Target.Data();
  m_Size       = Target.Size();
  m_CurrentPos = 0;

  if ( ( m_pData )
  ||   ( m_pDataTarget ) )
  {
    m_Opened = true;
  }

  return m_Opened;
}



GR::u64 MemoryStream::GetSize()
{
  return (GR::u64)m_Size;
}



unsigned long MemoryStream::ReadBlock( void* pDestination, size_t CountBytes )
{
  if ( ( m_pData == NULL )
  ||   ( !m_Opened ) )
  {
    m_ReadFailed = true;
    return 0;
  }

  if ( m_OpenType != OT_READ_ONLY )
  {
    return 0;
  }

  static unsigned long    ulBytesRead;

  if ( m_CurrentPos + CountBytes > m_Size )
  {
    // über das Ende rauslesen
    if ( m_CurrentPos < m_Size )
    {
      // den lesbaren Rest allerdings noch einlesen
      memcpy( pDestination, (unsigned char*)m_pData + m_CurrentPos, m_Size - m_CurrentPos );
      ulBytesRead   = (unsigned long)( m_Size - m_CurrentPos );
      m_CurrentPos  = m_Size;
    }
    else
    {
      ulBytesRead = 0;
    }
    m_ReadFailed = true;
  }
  else
  {
    memcpy( pDestination, (unsigned char*)m_pData + m_CurrentPos, CountBytes );
    m_CurrentPos  += CountBytes;
    m_ReadFailed  = false;
    ulBytesRead   = (GR::u32)CountBytes;
  }
  return ulBytesRead;
}



unsigned long MemoryStream::WriteBlock( const void* pSource, size_t CountBytes )
{
  if ( m_OpenType == OT_WRITE_ONLY )
  {
    if ( m_pDataTarget )
    {
      m_pDataTarget->AppendData( pSource, CountBytes );
      m_Size = m_pDataTarget->Size();
    }
    else
    {
      m_Data.AppendData( pSource, CountBytes );
      m_Size = m_Data.Size();
    }
  }

  return (GR::u32)CountBytes;
}



unsigned long MemoryStream::SetPosition( GR::i64 Offset, PositionType Position )
{
  if ( m_pData == NULL )
  {
    return (unsigned long)-1;
  }
  if ( Position == IIOStream::PT_END )
  {
    Offset = (int)m_Size - Offset;
  }
  else if ( Position == IIOStream::PT_CURRENT )
  {
    Offset += (int)m_CurrentPos;
  }
  if ( ( Offset < 0 )
  ||   ( Offset > (int)m_Size ) )
  {
    return (unsigned long)-1;
  }
  m_CurrentPos = (size_t)Offset;
  return (unsigned long)m_CurrentPos;
}



GR::u64 MemoryStream::GetPosition()
{
  if ( ( m_OpenType == OT_WRITE_ONLY )
  &&   ( m_Opened ) )
  {
    if ( m_pDataTarget )
    {
      return m_pDataTarget->Size();
    }
    return m_Data.Size();
  }

  if ( m_pData == NULL )
  {
    return (unsigned long)-1;
  }
  return (GR::u64)m_CurrentPos;
}



bool MemoryStream::Flush()
{
  return true;
}



bool MemoryStream::IsGood()
{
  if ( ( m_OpenType == OT_WRITE_ONLY )
  &&   ( m_Opened ) )
  {
    return true;
  }
  if ( ( m_OpenType == OT_READ_ONLY )
  ||   ( m_OpenType == OT_READ_ONLY_SHARED ) )
  {
    return true;
  }
  if ( ( m_pData == NULL )
  &&   ( m_pDataTarget == NULL ) )
  {
    return false;
  }
  return true;
}



bool MemoryStream::DataAvailable()
{
  if ( ( m_OpenType == OT_CLOSED )
  ||   ( m_OpenType == OT_WRITE_ONLY )
  ||   ( m_OpenType == OT_WRITE_APPEND ) )
  {
    return false;
  }

  return m_CurrentPos < m_Size;
}



const void* MemoryStream::Data()
{
  if ( ( m_OpenType == OT_WRITE_ONLY )
  &&   ( m_Opened ) )
  {
    if ( m_pDataTarget )
    {
      return m_pDataTarget->Data();
    }
    return m_Data.Data();
  }
  return m_pData;
}



const void* MemoryStream::CurrentData()
{
  if ( ( m_OpenType == OT_WRITE_ONLY )
  &&   ( m_Opened ) )
  {
    return NULL;
  }

  return ( (unsigned char*)m_pData ) + m_CurrentPos;
}



ByteBuffer MemoryStream::Buffer() const
{
  if ( !m_Opened )
  {
    return ByteBuffer();
  }

  if ( m_OpenType == OT_WRITE_ONLY )
  {
    return m_Data;
  }
  return ByteBuffer( m_pData, m_Size );
}



