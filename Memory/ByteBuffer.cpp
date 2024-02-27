#include <memory.h>

#include <debug/debugclient.h>

#include "ByteBuffer.h"



#ifdef __TANDEM
#pragma nowarn (262)
#endif



ByteBuffer::ByteBuffer() :
  m_pData( NULL ),
  m_Size( 0 ),
  m_AllocatedSize( 0 )
{
}

ByteBuffer::ByteBuffer( size_t Size ) :
  m_pData( NULL ),
  m_Size( Size ),
  m_AllocatedSize( Size )
{
  if ( Size )
  {
    m_pData = new GR::u8[Size];
    memset( m_pData, 0, Size );
  }
}



ByteBuffer::ByteBuffer( const GR::String& HexData ) :
  m_pData( NULL ),
  m_Size( 0 ),
  m_AllocatedSize( 0 )
{
  AppendHex( HexData );
}



ByteBuffer::ByteBuffer( const void* pData, size_t Size ) :
  m_pData( NULL ),
  m_Size( Size ),
  m_AllocatedSize( Size )
{
  if ( ( pData )
  &&   ( Size ) )
  {
    m_pData = new GR::u8[Size];
    memcpy( m_pData, pData, Size );
  }
  else
  {
    m_Size = 0;
    m_AllocatedSize = 0;
  }
}



ByteBuffer::~ByteBuffer()
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }
}



size_t ByteBuffer::Size() const
{
  return m_Size;
}



size_t ByteBuffer::AllocatedSize() const
{
  return m_AllocatedSize;
}



bool ByteBuffer::InsertByteAt( size_t Offset, GR::u8 Byte )
{
  if ( Offset > Size() )
  {
    dh::Log( "ByteBuffer::InsertByteAt trying to insert byte outside of buffer (Pos %d, Size %d)", Offset, Size() );
    return false;
  }
  GR::u8* pNewData = new GR::u8[Size() + 1];
  if ( Offset > 0 )
  {
    memcpy( pNewData, m_pData, Offset );
  }
  pNewData[Offset] = Byte;
  if ( Offset < Size() )
  {
    memcpy( pNewData + Offset + 1, m_pData + Offset, Size() - Offset );
  }
  delete[] m_pData;
  m_pData = pNewData;
  ++m_Size;
  return true;
}



bool ByteBuffer::InsertBytesAt( size_t Offset, size_t NumBytes, GR::u8 ByteValue )
{
  if ( Offset > Size() )
  {
    dh::Log( "ByteBuffer::InsertBytesAt trying to insert bytes outside of buffer (Pos %d, Size %d)", Offset, Size() );
    return false;
  }
  GR::u8* pNewData = new GR::u8[Size() + NumBytes];
  if ( Offset > 0 )
  {
    memcpy( pNewData, m_pData, Offset );
  }
  memset( pNewData + Offset, ByteValue, NumBytes );
  if ( Offset < Size() )
  {
    memcpy( pNewData + Offset + NumBytes, m_pData + Offset, Size() - Offset );
  }
  delete[] m_pData;
  m_pData = pNewData;
  m_Size += NumBytes;
  return true;
}



bool ByteBuffer::TruncateFront( size_t BytesToCut )
{
  if ( BytesToCut > m_Size )
  {
    dh::Log( "ByteBuffer::TruncateFront Trying to trunc %d bytes of %d", BytesToCut, m_Size );
    return false;
  }
  if ( m_Size == BytesToCut )
  {
    m_Size = 0;
    return true;
  }
  memmove( m_pData, (GR::u8*)m_pData + BytesToCut, m_Size - BytesToCut );
  m_Size -= BytesToCut;

  return true;
}



bool ByteBuffer::Truncate( size_t BytesToCut )
{
  if ( BytesToCut > m_Size )
  {
    dh::Log( "ByteBuffer::Truncate Trying to trunc %d bytes of %d", BytesToCut, m_Size );
    return false;
  }

  m_Size -= BytesToCut;
  return true;
}



bool ByteBuffer::TruncateAt( size_t RemainingLength )
{
  if ( RemainingLength > m_Size )
  {
    dh::Log( "ByteBuffer::TruncateAt Trying to trunc %d to %d bytes", m_Size, RemainingLength );
    return false;
  }

  m_Size = RemainingLength;
  return true;
}



const void* ByteBuffer::DataAt( size_t Index ) const
{
  if ( m_pData == NULL )
  {
    return NULL;
  }
  if ( Index >= m_Size )
  {
    return NULL;
  }

  return ( (GR::u8*)m_pData ) + Index;
}



void* ByteBuffer::DataAt( size_t Index )
{
  if ( m_pData == NULL )
  {
    return NULL;
  }
  if ( Index >= m_Size )
  {
    return NULL;
  }
  return ( (GR::u8*)m_pData ) + Index;
}



const void* ByteBuffer::Data() const
{
  return m_pData;
}



void* ByteBuffer::Data()
{
  return m_pData;
}



void ByteBuffer::Set( const void* pData, const size_t Size )
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }
  m_Size           = Size;
  m_AllocatedSize  = Size;

  if ( m_Size )
  {
    m_pData = new GR::u8[m_Size];

    memcpy( m_pData, pData, m_Size );
  }
}



ByteBuffer& ByteBuffer::operator=( const ByteBuffer& rhs )
{
  if ( &rhs == this )
  {
    return *this;
  }

  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }

  m_Size           = rhs.m_Size;
  m_AllocatedSize  = rhs.m_Size;

  if ( m_Size )
  {
    m_pData = new GR::u8[m_Size];
    memcpy( m_pData, rhs.m_pData, m_Size );
  }

  return *this;
}



ByteBuffer::ByteBuffer( const ByteBuffer& rhs ) :
  m_pData( NULL ),
  m_Size( 0 ),
  m_AllocatedSize( 0 )
{
  m_Size           = rhs.m_Size;
  m_AllocatedSize  = m_Size;
  if ( m_Size )
  {
    m_pData = new GR::u8[m_Size];
    memcpy( m_pData, rhs.m_pData, m_Size );
  }
}



void ByteBuffer::AppendBuffer( const ByteBuffer& Buffer )
{
  AppendData( Buffer.Data(), Buffer.Size() );
}



void ByteBuffer::AppendData( const void* pData, const size_t Size )
{
  if ( ( pData == NULL )
  ||   ( Size == 0 ) )
  {
    return;
  }

  if ( m_AllocatedSize - m_Size < Size )
  {
    GR::u8*   pNewData = new GR::u8[m_Size + Size];

    memcpy( pNewData, m_pData, m_Size );
    memcpy( ( (GR::u8*)pNewData ) + m_Size, pData, Size );

    delete[] m_pData;
    m_AllocatedSize = m_Size + Size;

    m_pData = pNewData;
  }
  else
  {
    memcpy( ( (GR::u8*)m_pData ) + m_Size, pData, Size );
  }

  m_Size += Size;
}



void ByteBuffer::AppendU8( const GR::u8 Data )
{
  AppendData( &Data, 1 );
}



void ByteBuffer::AppendU16( const GR::u16 Data )
{
  AppendU8( (GR::u8)( Data & 0xff ) );
  AppendU8( (GR::u8)( ( Data >> 8 ) & 0xff ) );
}



void ByteBuffer::AppendI16( const GR::i16 Data )
{
  AppendU16( (GR::u16)Data );
}



void ByteBuffer::AppendU16NetworkOrder( const GR::u16 Data )
{
  Reserve( Size() + 2 );
  AppendU8( (GR::u8)( ( Data >> 8 ) & 0xff ) );
  AppendU8( (GR::u8)( Data & 0xff ) );
}



void ByteBuffer::AppendI16NetworkOrder( const GR::i16 Data )
{
  AppendU16NetworkOrder( (GR::u16)Data );
}



void ByteBuffer::AppendU32( const GR::u32 Data )
{
  AppendData( &Data, 4 );
}



void ByteBuffer::AppendU32NetworkOrder( const GR::u32 Data )
{
  Reserve( Size() + 4 );
  AppendU8( (GR::u8)( ( Data >> 24 ) & 0xff ) );
  AppendU8( (GR::u8)( ( Data >> 16 ) & 0xff ) );
  AppendU8( (GR::u8)( ( Data >> 8 ) & 0xff ) );
  AppendU8( (GR::u8)( Data & 0xff ) );
}



void ByteBuffer::AppendU64( const GR::u64 Data )
{
  AppendData( &Data, 8 );
}



void ByteBuffer::AppendI32( const GR::i32 Value )
{
  AppendData( &Value, 4 );
}



void ByteBuffer::AppendI32NetworkOrder( const GR::i32 Value )
{
  AppendU32NetworkOrder( Value );
}



void ByteBuffer::AppendF32( const GR::f32 Float )
{
  AppendData( &Float, 4 );
}



void ByteBuffer::AppendString( const GR::String& Text )
{
  AppendU32( (GR::u32)Text.length() );
  AppendData( Text.c_str(), Text.length() );
}



void ByteBuffer::AppendString( const GR::WString& Text )
{
  AppendU32( (GR::u32)Text.length() );
  AppendData( Text.c_str(), Text.length() * 2 );
}



void ByteBuffer::Grow( size_t NewSize, bool SetPos )
{
  GR::u8*     pTemp = new GR::u8[m_AllocatedSize + NewSize];

  m_AllocatedSize += NewSize;

  if ( m_Size )
  {
    memcpy( pTemp, m_pData, Size() );
  }
  // mit 0 füllen
  memset( pTemp + m_Size, 0, NewSize );

  delete[] m_pData;

  m_pData = pTemp;

  if ( SetPos )
  {
    m_Size = m_AllocatedSize;
  }
}



void ByteBuffer::Clear()
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }
  m_Size           = 0;
  m_AllocatedSize  = 0;
}



void ByteBuffer::SetDataAt( size_t Index, const void* pData, const size_t Size )
{
  if ( Index + Size > m_Size )
  {
    dh::Log( "ByteBuffer::SetDataAt Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }
  memcpy( ( (GR::u8*)m_pData ) + Index, pData, Size );
}



void ByteBuffer::SetByteAt( size_t Index, GR::u8 Byte )
{
  if ( Index >= m_Size )
  {
    dh::Log( "ByteBuffer::SetByteAt Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }

  ( (GR::u8*)m_pData )[Index] = Byte;
}



void ByteBuffer::SetU16At( size_t Index, GR::u16 Word )
{
  if ( Index + 1 >= m_Size )
  {
    dh::Log( "ByteBuffer::SetU16At Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }

#ifdef __TANDEM
  ( (GR::u8*)m_pData )[Index] = (GR::u8)( ( Word >> 8 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 1] = (GR::u8)( Word & 0xff );
#else
  *(GR::u16*)( ( (GR::u8*)m_pData ) + Index ) = Word;
#endif  
}



void ByteBuffer::SetU16NetworkOrderAt( size_t Index, GR::u16 Word )
{
  if ( Index + 1 >= m_Size )
  {
    dh::Log( "ByteBuffer::SetU16At Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }

  ( (GR::u8*)m_pData )[Index] = (GR::u8)( ( Word >> 8 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 1] = (GR::u8)( Word & 0xff );
}



void ByteBuffer::SetU32At( size_t Index, GR::u32 DWord )
{
  if ( Index + 3 >= m_Size )
  {
    dh::Log( "ByteBuffer::SetU32At Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }
  *(GR::u32*)( ( (GR::u8*)m_pData ) + Index ) = DWord;
}



void ByteBuffer::SetU32NetworkOrderAt( size_t Index, GR::u32 DWord )
{
  if ( Index + 3 >= m_Size )
  {
    dh::Log( "ByteBuffer::SetU32NetworkOrderAt Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }

  ( (GR::u8*)m_pData )[Index + 0] = (GR::u8)( ( DWord >> 24 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 1] = (GR::u8)( ( DWord >> 16 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 2] = (GR::u8)( ( DWord >> 8 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 3] = (GR::u8)( DWord & 0xff );
}



void ByteBuffer::SetU64At( size_t Index, GR::u64 QWord )
{
  if ( Index + 7 >= m_Size )
  {
    dh::Log( "ByteBuffer::SetU64At Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }
  *(GR::u64*)( ( (GR::u8*)m_pData ) + Index ) = QWord;
}



void ByteBuffer::SetU64NetworkOrderAt( size_t Index, GR::u64 QWord )
{
  if ( Index + 7 >= m_Size )
  {
    dh::Log( "ByteBuffer::SetU64NetworkOrderAt Index zu hoch (%d > %d)", Index, m_Size );
    return;
  }

  ( (GR::u8*)m_pData )[Index + 0] = (GR::u8)( ( QWord >> 56 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 1] = (GR::u8)( ( QWord >> 48 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 2] = (GR::u8)( ( QWord >> 40 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 3] = (GR::u8)( ( QWord >> 32 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 4] = (GR::u8)( ( QWord >> 24 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 5] = (GR::u8)( ( QWord >> 16 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 6] = (GR::u8)( ( QWord >> 8 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 7] = (GR::u8)( QWord & 0xff );
}



GR::u8 ByteBuffer::ByteAt( size_t Index ) const
{
  if ( Index >= m_Size )
  {
    dh::Log( "ByteBuffer::ByteAt Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }
  return ( (GR::u8*)m_pData )[Index];
}



GR::u16 ByteBuffer::U16At( size_t Index ) const
{
  if ( Index + 1 >= m_Size )
  {
    dh::Log( "ByteBuffer::U16At Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }

  GR::u16     wData = *(GR::u16*)( ( (GR::u8*)m_pData ) + Index );

#ifdef __TANDEM
  return (GR::u16)( ( ( wData & 0xff ) << 8 ) + ( ( wData >> 8 ) & 0xff ) );
#else
  return wData;
#endif
}



GR::u16 ByteBuffer::U16NetworkOrderAt( size_t Index ) const
{
  if ( Index + 1 >= m_Size )
  {
    dh::Log( "ByteBuffer::U16NetworkOrderAt Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }

  GR::u16     wData = *(GR::u16*)( ( (GR::u8*)m_pData ) + Index );

#ifdef __TANDEM
  return wData;
#else
  return (GR::u16)( ( ( wData & 0xff ) << 8 ) + ( ( wData >> 8 ) & 0xff ) );
#endif
}



GR::u32 ByteBuffer::U32At( size_t Index ) const
{
  if ( Index + 3 >= m_Size )
  {
    dh::Log( "ByteBuffer::U32At Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }

#ifdef __TANDEM
  GR::u32       dwData = *(GR::u32*)( ( (GR::u8*)m_pData ) + Index );

  return ( ( dwData >> 24 )
         | ( ( dwData & 0x00ff0000 ) >> 8 )
         | ( ( dwData & 0x0000ff00 ) << 8 )
         | ( ( dwData & 0x000000ff ) << 24 ) );
#else
  return ( (GR::u32*)m_pData )[Index];
#endif
}



GR::u32 ByteBuffer::U32NetworkOrderAt( size_t Index ) const
{
  if ( Index + 3 >= m_Size )
  {
    dh::Log( "ByteBuffer::U32At Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }

#ifdef __TANDEM
  return ( (GR::u32*)m_pData )[Index];
#else

  GR::u32       dwData = *(GR::u32*)( ( (GR::u8*)m_pData ) + Index );

  return ( ( dwData >> 24 )
       | ( ( dwData & 0x00ff0000 ) >> 8 )
       | ( ( dwData & 0x0000ff00 ) << 8 )
       | ( ( dwData & 0x000000ff ) << 24 ) );
#endif
}



GR::u64 ByteBuffer::U64At( size_t Index ) const
{
  if ( Index + 7 >= m_Size )
  {
    dh::Log( "ByteBuffer::U64At Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }

#ifdef __TANDEM
  GR::u64       dwData = *(GR::u64*)( ( (GR::u8*)m_pData ) + Index );

  return ( ( dwData >> 56 )
         | ( ( dwData & 0x00ff000000000000 ) >> 40 )
         | ( ( dwData & 0x0000ff0000000000 ) >> 24 )
         | ( ( dwData & 0x000000ff00000000 ) >>  8 )
         | ( ( dwData & 0x00000000ff000000 ) <<  8 )
         | ( ( dwData & 0x0000000000ff0000 ) << 24 )
         | ( ( dwData & 0x000000000000ff00 ) << 40 )
         | ( ( dwData & 0x00000000000000ff ) << 56 ) );
#else
  return ( (GR::u64*)m_pData )[Index];
#endif
}



GR::u64 ByteBuffer::U64NetworkOrderAt( size_t Index ) const
{
  if ( Index + 7 >= m_Size )
  {
    dh::Log( "ByteBuffer::U64At Index zu hoch (%d > %d)", Index, m_Size );
    return 0;
  }
#ifdef __TANDEM
  return ( (GR::u64*)m_pData )[Index];
#else
  GR::u64       dwData = *(GR::u64*)( ( (GR::u8*)m_pData ) + Index );

  return ( ( dwData >> 56 )
         | ( ( dwData & 0x00ff000000000000 ) >> 40 )
         | ( ( dwData & 0x0000ff0000000000 ) >> 24 )
         | ( ( dwData & 0x000000ff00000000 ) >>  8 )
         | ( ( dwData & 0x00000000ff000000 ) <<  8 )
         | ( ( dwData & 0x0000000000ff0000 ) << 24 )
         | ( ( dwData & 0x000000000000ff00 ) << 40 )
         | ( ( dwData & 0x00000000000000ff ) << 56 ) );
#endif
}



int ByteBuffer::Compare( const ByteBuffer& Buffer ) const
{
  return Compare( Buffer.Data(), Buffer.Size() );
}



int ByteBuffer::Compare( const void* pData, size_t DataLength ) const
{
  if ( Size() == 0 )
  {
    if ( DataLength == 0 )
    {
      return 0;
    }
    return -1;
  }
  if ( DataLength == 0 )
  {
    return 1;
  }

  if ( Size() < DataLength )
  {
    return -1;
  }
  if ( Size() > DataLength )
  {
    return 1;
  }
  return memcmp( Data(), pData, Size() );
}



GR::String ByteBuffer::ToString( size_t Offset, size_t Length ) const
{
  GR::String   result = "";
  
  if ( Length == (size_t)-1 )
  {
    Length = Size();
  }
  
  if ( ( Offset < m_Size )
  &&   ( Offset + Length <= m_Size ) )
  {
    result.append( (char*)m_pData + Offset, Length );
  }
  return result;
}



bool ByteBuffer::AppendHex( const GR::String& HexData )
{
  if ( HexData.length() & 1 )
  {
    return false;
  }

  Reserve( HexData.length() / 2 );

  size_t    curPos = 0;

  while ( curPos < HexData.length() )
  {
    GR::u8    byte = (GR::u8)HexData[curPos];

    GR::u32   value = 0;

    if ( ( byte >= '0' )
    &&   ( byte <= '9' ) )
    {
      value += byte - '0';
    }
    else if ( ( byte >= 'a' )
    &&        ( byte <= 'f' ) )
    {
      value += byte + 10 - 'a';
    }
    else if ( ( byte >= 'A' )
    &&        ( byte <= 'F' ) )
    {
      value += byte + 10 - 'A';
    }
    else
    {
      return false;
    }

    value <<= 4;
    ++curPos;

    byte = (GR::u8)HexData[curPos];
    if ( ( byte >= '0' )
    &&   ( byte <= '9' ) )
    {
      value += byte - '0';
    }
    else if ( ( byte >= 'a' )
    &&        ( byte <= 'f' ) )
    {
      value += byte + 10 - 'a';
    }
    else if ( ( byte >= 'A' )
    &&        ( byte <= 'F' ) )
    {
      value += byte + 10 - 'A';
    }
    else
    {
      return false;
    }

    AppendU8( (GR::u8)value );

    ++curPos;
  }
  return true;
}



bool ByteBuffer::FromAsciiString( const GR::String& AsciiData )
{
  Reserve( AsciiData.length() );

  for ( size_t i = 0; i < AsciiData.length(); ++i )
  {
    char    cByte = AsciiData[i];

    if ( ( cByte >= '0' )
    &&   ( cByte <= '9' ) )
    {
      AppendU8( (GR::u8)( cByte - '0' ) );
    }
    else
    {
      AppendU8( 0 );
    }
  }
  return true;
}



bool ByteBuffer::FromString( const GR::String& Data )
{
  AppendData( Data.c_str(), Data.length() );
  return true;
}



GR::String ByteBuffer::ToHexString( size_t StartOffset, size_t Length ) const
{
  GR::String   result = "";

  if ( Length == (size_t)-1 )
  {
    Length = Size();
  }
  if ( StartOffset >= Size() )
  {
    return result;
  }
  if ( StartOffset + Length >= Size() )
  {
    Length = Size() - StartOffset;
  }
  result.reserve( Length * 2 );
  for ( size_t i = 0; i < Length; ++i )
  {
    GR::u8    byte = ( (char*)m_pData )[StartOffset + i];

    GR::u8    nibble = (GR::u8)( byte >> 4 );

    if ( nibble < 10 )
    {
      result += (char)( '0' + nibble );
    }
    else
    {
      result += (char)( 'A' + nibble - 10 );
    }
    nibble = (GR::u8)( byte & 0x0f );
    if ( nibble < 10 )
    {
      result += (char)( '0' + nibble );
    }
    else
    {
      result += (char)( 'A' + nibble - 10 );
    }
  }
  return result;
}



GR::String ByteBuffer::ToAsciiString() const
{
  GR::String   result;

  result.reserve( Size() * 2 );
  for ( size_t i = 0; i < Size(); ++i )
  {
    result += (char)( '0' + ( ( ( (char*)m_pData )[i] & 0xf0 ) >> 4 ) );
    result += (char)( 48 + ( ( (char*)m_pData )[i] & 0x0f ) );
  }

  return result;
}



size_t ByteBuffer::FindU8( const GR::u8 Byte, const size_t Offset ) const
{
  if ( ( m_pData == NULL )
  ||   ( Offset >= m_Size ) )
  {
    return (size_t)-1;
  }
  for ( size_t i = Offset; i < m_Size; ++i )
  {
    if ( ( (GR::u8*)m_pData )[i] == Byte )
    {
      return i;
    }
  }
  return (size_t)-1;
}



size_t ByteBuffer::Find( const ByteBuffer& FindBlock ) const
{
  if ( FindBlock.Size() > Size() )
  {
    return (size_t)-1;
  }

  for ( size_t searchPos = 0; searchPos < Size() - FindBlock.Size() + 1; ++searchPos )
  {
    size_t    tempPos = 0;

    while ( tempPos < FindBlock.Size() )
    {
      if ( ByteAt( searchPos + tempPos ) != FindBlock.ByteAt( tempPos ) )
      {
        break;
      }
      ++tempPos;
    }
    if ( tempPos == FindBlock.Size() )
    {
      return searchPos;
    }
  }

  return (size_t)-1;
}



void ByteBuffer::Reserve( size_t NewSize )
{
  if ( NewSize < m_AllocatedSize )
  {
    return;
  }
  GR::u8*     pTemp = new GR::u8[NewSize];

  m_AllocatedSize = NewSize;

  if ( m_Size )
  {
    memcpy( pTemp, m_pData, Size() );
  }
  if ( NewSize > Size() )
  {
    // mit 0 füllen
    memset( pTemp + m_Size, 0, NewSize - Size() );
  }
  delete[] m_pData;
  m_pData = pTemp;
}



void ByteBuffer::Resize( size_t NewSize )
{
  if ( NewSize == m_Size )
  {
    return;
  }
  if ( NewSize < m_Size )
  {
    Truncate( m_Size - NewSize );
    return;
  }

  Reserve( NewSize );
  m_Size = NewSize;
}



bool ByteBuffer::Empty() const
{
  return ( m_Size == 0 );
}



ByteBuffer ByteBuffer::SubBuffer( size_t Index ) const
{
  ByteBuffer     bbResult;

  if ( Index >= m_Size )
  {
    return bbResult;
  }
  bbResult.Resize( Size() - Index );
  memcpy( bbResult.Data(), DataAt( Index ), bbResult.Size() );

  return bbResult;
}



ByteBuffer ByteBuffer::SubBuffer( size_t Index, size_t Count ) const
{
  ByteBuffer     bbResult;

  if ( ( Index >= m_Size )
  ||   ( Index + Count > m_Size ) )
  {
    return bbResult;
  }
  bbResult.Resize( Count );
  memcpy( bbResult.Data(), DataAt( Index ), Count );

  return bbResult;
}



bool ByteBuffer::operator==( const ByteBuffer& rhs ) const
{
  return ( Compare( rhs ) == 0 );
}



bool ByteBuffer::operator!=( const ByteBuffer& rhs ) const
{
  return ( Compare( rhs ) != 0 );
}



ByteBuffer ByteBuffer::operator+( const ByteBuffer& rhs ) const
{
  ByteBuffer   bbResult( *this );
  bbResult.AppendBuffer( rhs );

  return bbResult;
}



ByteBuffer& ByteBuffer::operator+=( const ByteBuffer& rhs )
{
  AppendBuffer( rhs );

  return *this;
}



GR::u8& ByteBuffer::operator[] ( const size_t Index )
{
  if ( Index >= m_Size )
  {
    static GR::u8   dummy;
    dummy = 0;
    return dummy;
  }
  return m_pData[Index];
}



const GR::u8& ByteBuffer::operator[] ( const size_t Index ) const
{
  if ( Index >= m_Size )
  {
    static GR::u8   dummy;
    dummy = 0;
    return dummy;
  }
  return m_pData[Index];
}



Misc::CFormat operator<< ( Misc::CFormat Format, const ByteBuffer& Param )
{
  return Format << Param.ToHexString();
}