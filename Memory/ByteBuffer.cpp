#include <memory.h>

#include <debug/debugclient.h>

#include "ByteBuffer.h"


#ifdef __TANDEM
#pragma nowarn (262)
#endif



ByteBuffer::ByteBuffer() :
  m_iSize( 0 ),
  m_iAllocatedSize( 0 ),
  m_pData( NULL )
{
}

ByteBuffer::ByteBuffer( size_t iSize ) :
  m_iSize( iSize ),
  m_iAllocatedSize( iSize ),
  m_pData( NULL )
{
  if ( iSize )
  {
    m_pData = new GR::u8[iSize];
    memset( m_pData, 0, iSize );
  }
}



ByteBuffer::ByteBuffer( const GR::String& HexData ) :
  m_iSize( 0 ),
  m_iAllocatedSize( 0 ),
  m_pData( NULL )
{
  AppendHex( HexData );
}



ByteBuffer::ByteBuffer( const void* pData, size_t iSize ) :
  m_pData( NULL ),
  m_iSize( iSize ),
  m_iAllocatedSize( iSize )
{
  if ( ( pData )
  &&   ( iSize ) )
  {
    m_pData = new GR::u8[iSize];
    memcpy( m_pData, pData, iSize );
  }
  else
  {
    m_iSize = 0;
    m_iAllocatedSize = 0;
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
  return m_iSize;
}



size_t ByteBuffer::AllocatedSize() const
{
  return m_iAllocatedSize;
}



bool ByteBuffer::TruncateFront( size_t iBytesToCut )
{
  if ( iBytesToCut > m_iSize )
  {
    dh::Log( "ByteBuffer::TruncateFront Trying to trunc %d bytes of %d", iBytesToCut, m_iSize );
    return false;
  }
  if ( m_iSize == iBytesToCut )
  {
    m_iSize = 0;
    return true;
  }
  memmove( m_pData, (GR::u8*)m_pData + iBytesToCut, m_iSize - iBytesToCut );
  m_iSize -= iBytesToCut;

  return true;
}



bool ByteBuffer::Truncate( size_t iBytesToCut )
{
  if ( iBytesToCut > m_iSize )
  {
    dh::Log( "ByteBuffer::Truncate Trying to trunc %d bytes of %d", iBytesToCut, m_iSize );
    return false;
  }

  m_iSize -= iBytesToCut;
  return true;
}



bool ByteBuffer::TruncateAt( size_t iRemainingLength )
{
  if ( iRemainingLength > m_iSize )
  {
    dh::Log( "ByteBuffer::TruncateAt Trying to trunc %d to %d bytes", m_iSize, iRemainingLength );
    return false;
  }

  m_iSize = iRemainingLength;
  return true;
}



const void* ByteBuffer::DataAt( size_t iIndex ) const
{
  if ( m_pData == NULL )
  {
    return NULL;
  }
  if ( iIndex >= m_iSize )
  {
    return NULL;
  }

  return ( (GR::u8*)m_pData ) + iIndex;
}



void* ByteBuffer::DataAt( size_t iIndex )
{
  if ( m_pData == NULL )
  {
    return NULL;
  }
  if ( iIndex >= m_iSize )
  {
    return NULL;
  }
  return ( (GR::u8*)m_pData ) + iIndex;
}



const void* ByteBuffer::Data() const
{
  return m_pData;
}



void* ByteBuffer::Data()
{
  return m_pData;
}



void ByteBuffer::Set( const void* pData, const size_t iSize )
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }
  m_iSize           = iSize;
  m_iAllocatedSize  = iSize;

  if ( m_iSize )
  {
    m_pData = new GR::u8[m_iSize];

    memcpy( m_pData, pData, m_iSize );
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

  m_iSize           = rhs.m_iSize;
  m_iAllocatedSize  = rhs.m_iSize;

  if ( m_iSize )
  {
    m_pData = new GR::u8[m_iSize];
    memcpy( m_pData, rhs.m_pData, m_iSize );
  }

  return *this;
}



ByteBuffer::ByteBuffer( const ByteBuffer& rhs ) :
  m_pData( NULL ),
  m_iSize( 0 ),
  m_iAllocatedSize( 0 )
{
  m_iSize           = rhs.m_iSize;
  m_iAllocatedSize  = m_iSize;
  if ( m_iSize )
  {
    m_pData = new GR::u8[m_iSize];
    memcpy( m_pData, rhs.m_pData, m_iSize );
  }
}



void ByteBuffer::AppendBuffer( const ByteBuffer& Buffer )
{
  AppendData( Buffer.Data(), Buffer.Size() );
}



void ByteBuffer::AppendData( const void* pData, const size_t iSize )
{
  if ( ( pData == NULL )
  ||   ( iSize == 0 ) )
  {
    return;
  }

  if ( m_iAllocatedSize - m_iSize < iSize )
  {
    GR::u8*   pNewData = new GR::u8[m_iSize + iSize];

    memcpy( pNewData, m_pData, m_iSize );
    memcpy( ( (GR::u8*)pNewData ) + m_iSize, pData, iSize );

    delete[] m_pData;
    m_iAllocatedSize = m_iSize + iSize;

    m_pData = pNewData;
  }
  else
  {
    memcpy( ( (GR::u8*)m_pData ) + m_iSize, pData, iSize );
  }

  m_iSize += iSize;
}



void ByteBuffer::AppendU8( const GR::u8 ucData )
{
  AppendData( &ucData, 1 );
}



void ByteBuffer::AppendU16( const GR::u16 Data )
{
  AppendU8( (GR::u8)( Data & 0xff ) );
  AppendU8( (GR::u8)( ( Data >> 8 ) & 0xff ) );
}



void ByteBuffer::AppendU16NetworkOrder( const GR::u16 Data )
{
  Reserve( Size() + 2 );
  AppendU8( (GR::u8)( ( Data >> 8 ) & 0xff ) );
  AppendU8( (GR::u8)( Data & 0xff ) );
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



void ByteBuffer::AppendF32( const GR::f32 fFloat )
{
  AppendData( &fFloat, 4 );
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
  GR::u8*     pTemp = new GR::u8[m_iAllocatedSize + NewSize];

  m_iAllocatedSize += NewSize;

  if ( m_iSize )
  {
    memcpy( pTemp, m_pData, Size() );
  }
  // mit 0 füllen
  memset( pTemp + m_iSize, 0, NewSize );

  delete[] m_pData;

  m_pData = pTemp;

  if ( SetPos )
  {
    m_iSize = m_iAllocatedSize;
  }
}



void ByteBuffer::Clear()
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }
  m_iSize           = 0;
  m_iAllocatedSize  = 0;
}



void ByteBuffer::SetDataAt( size_t Index, const void* pData, const size_t Size )
{
  if ( Index + Size > m_iSize )
  {
    dh::Log( "ByteBuffer::SetDataAt Index zu hoch (%d > %d)", Index, m_iSize );
    return;
  }
  memcpy( ( (GR::u8*)m_pData ) + Index, pData, Size );
}



void ByteBuffer::SetByteAt( size_t Index, GR::u8 ucByte )
{
  if ( Index >= m_iSize )
  {
    dh::Log( "ByteBuffer::SetByteAt Index zu hoch (%d > %d)", Index, m_iSize );
    return;
  }

  ( (GR::u8*)m_pData )[Index] = ucByte;
}



void ByteBuffer::SetU16At( size_t Index, GR::u16 Word )
{
  if ( Index + 1 >= m_iSize )
  {
    dh::Log( "ByteBuffer::SetU16At Index zu hoch (%d > %d)", Index, m_iSize );
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
  if ( Index + 1 >= m_iSize )
  {
    dh::Log( "ByteBuffer::SetU16At Index zu hoch (%d > %d)", Index, m_iSize );
    return;
  }

  ( (GR::u8*)m_pData )[Index] = (GR::u8)( ( Word >> 8 ) & 0xff );
  ( (GR::u8*)m_pData )[Index + 1] = (GR::u8)( Word & 0xff );
}



void ByteBuffer::SetU32At( size_t Index, GR::u32 DWord )
{
  if ( Index + 3 >= m_iSize )
  {
    dh::Log( "ByteBuffer::SetU32At Index zu hoch (%d > %d)", Index, m_iSize );
    return;
  }
  *(GR::u32*)( ( (GR::u8*)m_pData ) + Index ) = DWord;
}



GR::u8 ByteBuffer::ByteAt( size_t Index ) const
{
  if ( Index >= m_iSize )
  {
    dh::Log( "ByteBuffer::ByteAt Index zu hoch (%d > %d)", Index, m_iSize );
    return 0;
  }
  return ( (GR::u8*)m_pData )[Index];
}



GR::u16 ByteBuffer::U16At( size_t Index ) const
{
  if ( Index + 1 >= m_iSize )
  {
    dh::Log( "ByteBuffer::U16At Index zu hoch (%d > %d)", Index, m_iSize );
    return 0;
  }

  //return *(GR::u16*)( ( (GR::u8*)m_pData ) + dwIndex );
  GR::u16     wData = *(GR::u16*)( ( (GR::u8*)m_pData ) + Index );

#ifdef __TANDEM
  return (GR::u16)( ( ( wData & 0xff ) << 8 ) + ( ( wData >> 8 ) & 0xff ) );
#else
  return wData;
#endif
}



GR::u16 ByteBuffer::U16NetworkOrderAt( size_t Index ) const
{
  if ( Index + 1 >= m_iSize )
  {
    dh::Log( "ByteBuffer::U16NetworkOrderAt Index zu hoch (%d > %d)", Index, m_iSize );
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
  if ( Index + 3 >= m_iSize )
  {
    dh::Log( "ByteBuffer::U32At Index zu hoch (%d > %d)", Index, m_iSize );
    return 0;
  }

#ifdef __TANDEM
  GR::u32       dwData = ( (GR::u32*)m_pData )[Index];

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
  if ( Index + 3 >= m_iSize )
  {
    dh::Log( "ByteBuffer::U32At Index zu hoch (%d > %d)", Index, m_iSize );
    return 0;
  }

#ifdef __TANDEM
  return ( (GR::u32*)m_pData )[Index];
#else

  GR::u32       dwData = ( (GR::u32*)m_pData )[Index];

  return ( ( dwData >> 24 )
       | ( ( dwData & 0x00ff0000 ) >> 8 )
       | ( ( dwData & 0x0000ff00 ) << 8 )
       | ( ( dwData & 0x000000ff ) << 24 ) );
#endif
}



GR::u64 ByteBuffer::U64At( size_t Index ) const
{
  if ( Index + 7 >= m_iSize )
  {
    dh::Log( "ByteBuffer::U64At Index zu hoch (%d > %d)", Index, m_iSize );
    return 0;
  }

#ifdef __TANDEM
  GR::u64       dwData = ( (GR::u64*)m_pData )[Index];

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
  if ( Index + 7 >= m_iSize )
  {
    dh::Log( "ByteBuffer::U64At Index zu hoch (%d > %d)", Index, m_iSize );
    return 0;
  }
#ifdef __TANDEM
  return ( (GR::u64*)m_pData )[Index];
#else
  GR::u64       dwData = ( (GR::u64*)m_pData )[Index];

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



GR::String ByteBuffer::ToString() const
{
  GR::String   result = "";

  result.append( (char*)m_pData, Size() );

  return result;
}



bool ByteBuffer::AppendHex( const GR::String& HexData )
{
  if ( HexData.length() & 1 )
  {
    return false;
  }

  Reserve( HexData.length() / 2 );

  size_t    iCurPos = 0;

  while ( iCurPos < HexData.length() )
  {
    GR::u8    ucByte = (GR::u8)HexData[iCurPos];

    GR::u32   value = 0;

    if ( ( ucByte >= '0' )
    &&   ( ucByte <= '9' ) )
    {
      value += ucByte - '0';
    }
    else if ( ( ucByte >= 'a' )
    &&        ( ucByte <= 'f' ) )
    {
      value += ucByte + 10 - 'a';
    }
    else if ( ( ucByte >= 'A' )
    &&        ( ucByte <= 'F' ) )
    {
      value += ucByte + 10 - 'A';
    }
    else
    {
      return false;
    }

    value <<= 4;
    ++iCurPos;

    ucByte = (GR::u8)HexData[iCurPos];
    if ( ( ucByte >= '0' )
    &&   ( ucByte <= '9' ) )
    {
      value += ucByte - '0';
    }
    else if ( ( ucByte >= 'a' )
    &&        ( ucByte <= 'f' ) )
    {
      value += ucByte + 10 - 'a';
    }
    else if ( ( ucByte >= 'A' )
    &&        ( ucByte <= 'F' ) )
    {
      value += ucByte + 10 - 'A';
    }
    else
    {
      return false;
    }

    AppendU8( (GR::u8)value );

    ++iCurPos;
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
    GR::u8    ucByte = ( (char*)m_pData )[StartOffset + i];

    GR::u8    ucNibble = (GR::u8)( ucByte >> 4 );

    if ( ucNibble < 10 )
    {
      result += (char)( '0' + ucNibble );
    }
    else
    {
      result += (char)( 'A' + ucNibble - 10 );
    }
    ucNibble = (GR::u8)( ucByte & 0x0f );
    if ( ucNibble < 10 )
    {
      result += (char)( '0' + ucNibble );
    }
    else
    {
      result += (char)( 'A' + ucNibble - 10 );
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
  ||   ( Offset >= m_iSize ) )
  {
    return (size_t)-1;
  }
  for ( size_t i = Offset; i < m_iSize; ++i )
  {
    if ( ( (GR::u8*)m_pData )[i] == Byte )
    {
      return i;
    }
  }
  return (size_t)-1;
}



size_t ByteBuffer::Find( const ByteBuffer& bbFindBlock ) const
{
  if ( bbFindBlock.Size() > Size() )
  {
    return (size_t)-1;
  }

  for ( size_t searchPos = 0; searchPos < Size() - bbFindBlock.Size() + 1; ++searchPos )
  {
    size_t    dwTempPos = 0;

    while ( dwTempPos < bbFindBlock.Size() )
    {
      if ( ByteAt( searchPos + dwTempPos ) != bbFindBlock.ByteAt( dwTempPos ) )
      {
        break;
      }
      ++dwTempPos;
    }
    if ( dwTempPos == bbFindBlock.Size() )
    {
      return searchPos;
    }
  }

  return (size_t)-1;
}



void ByteBuffer::Reserve( size_t NewSize )
{
  if ( NewSize < m_iAllocatedSize )
  {
    return;
  }
  GR::u8*     pTemp = new GR::u8[NewSize];

  m_iAllocatedSize = NewSize;

  if ( m_iSize )
  {
    memcpy( pTemp, m_pData, Size() );
  }
  if ( NewSize > Size() )
  {
    // mit 0 füllen
    memset( pTemp + m_iSize, 0, NewSize - Size() );
  }
  delete[] m_pData;
  m_pData = pTemp;
}



void ByteBuffer::Resize( size_t NewSize )
{
  if ( NewSize == m_iSize )
  {
    return;
  }
  if ( NewSize < m_iSize )
  {
    Truncate( m_iSize - NewSize );
    return;
  }

  Reserve( NewSize );
  m_iSize = NewSize;
}



bool ByteBuffer::Empty() const
{
  return ( m_iSize == 0 );
}



ByteBuffer ByteBuffer::SubBuffer( size_t Index ) const
{
  ByteBuffer     bbResult;

  if ( Index >= m_iSize )
  {
    return bbResult;
  }
  bbResult.Resize( Size() - Index );
  memcpy( bbResult.Data(), DataAt( Index ), bbResult.Size() );

  return bbResult;
}



ByteBuffer ByteBuffer::SubBuffer( size_t Index, size_t dwCount ) const
{
  ByteBuffer     bbResult;

  if ( ( Index >= m_iSize )
  ||   ( Index + dwCount > m_iSize ) )
  {
    return bbResult;
  }
  bbResult.Resize( dwCount );
  memcpy( bbResult.Data(), DataAt( Index ), dwCount );

  return bbResult;
}



bool ByteBuffer::operator==( const ByteBuffer& rhs )
{
  return ( Compare( rhs ) == 0 );
}



bool ByteBuffer::operator!=( const ByteBuffer& rhs )
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
