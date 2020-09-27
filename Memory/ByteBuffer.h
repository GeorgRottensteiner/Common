#ifndef LANG_BYTEBUFFER_H
#define LANG_BYTEBUFFER_H

#include <GR/GRTypes.h>



class ByteBuffer
{

  protected:

    GR::u8*         m_pData;

    size_t          m_iSize,
                    m_iAllocatedSize;


  public:


    ByteBuffer();
    explicit ByteBuffer( size_t iSize );
    explicit ByteBuffer( const GR::String& HexData );
    ByteBuffer( const void* pData, size_t iSize );

    ~ByteBuffer();

    size_t          Size() const;
    size_t          AllocatedSize() const;
    bool            Empty() const;
    void*           Data();
    const void*     Data() const;
    void*     DataAt( size_t iIndex );
    const void*     DataAt( size_t iIndex ) const;

    void            Clear();
    void            Set( const void* pData, const size_t iSize );

    void            Grow( size_t iSize, bool bSetPos = false );
    void            Reserve( size_t iSize );
    void            Resize( size_t iSize );
    void            AppendBuffer( const ByteBuffer& Buffer );
    void            AppendData( const void* pData, const size_t iSize );
    void            AppendI32( const GR::i32 Value );
    void            AppendI32NetworkOrder( const GR::i32 Value );
    void            AppendU32( const GR::u32 Data );
    void            AppendU32NetworkOrder( const GR::u32 Data );
    void            AppendU64( const GR::u64 dwData );
    void            AppendU8( const GR::u8 ucData );
    void            AppendU16( const GR::u16 wData );
    void            AppendU16NetworkOrder( const GR::u16 wData );
    void            AppendF32( const GR::f32 fFloat );
    void            AppendString( const GR::String& strText );
    void            AppendString( const GR::WString& strText );

    int             Compare( const ByteBuffer& Buffer ) const;
    int             Compare( const void* pData, size_t iDataLength ) const;

    size_t          Find( const ByteBuffer& bbFindBlock ) const;
    size_t          FindU8( const GR::u8 ucByte, const size_t Offset = 0 ) const;

    bool            Truncate( size_t iBytesToCut );
    bool            TruncateAt( size_t iRemainingLength );
    bool            TruncateFront( size_t iBytesToCut );


    GR::String      ToString() const;
    GR::String      ToHexString( size_t StartOffset = 0, size_t Length = (size_t)-1 ) const;
    GR::String      ToAsciiString() const;
    bool            AppendHex( const GR::String& HexData );
    bool            FromAsciiString( const GR::String& AsciiData );
    bool            FromString( const GR::String& Data );

    GR::u8          ByteAt( size_t Index ) const;
    GR::u16         U16At( size_t Index ) const;          
    GR::u16         U16NetworkOrderAt( size_t Index ) const;          
    GR::u32         U32At( size_t Index ) const;          
    GR::u32         U32NetworkOrderAt( size_t Index ) const;          
    GR::u64         U64At( size_t Index ) const;          
    GR::u64         U64NetworkOrderAt( size_t dwIndex ) const;
    void            SetByteAt( size_t Index, GR::u8 Byte );
    void            SetU16At( size_t Index, GR::u16 Word );
    void            SetU16NetworkOrderAt( size_t Index, GR::u16 Word );
    void            SetU32At( size_t Index, GR::u32 ord );
    void            SetDataAt( size_t Index, const void* pData, const size_t Size );

    ByteBuffer     SubBuffer( size_t dwIndex ) const;
    ByteBuffer     SubBuffer( size_t dwIndex, size_t dwCount ) const;


    ByteBuffer     operator+( const ByteBuffer& rhs ) const;
    ByteBuffer&    operator+=( const ByteBuffer& rhs );
    ByteBuffer&    operator=( const ByteBuffer& rhs );
    bool            operator==( const ByteBuffer& rhs );
    bool            operator!=( const ByteBuffer& rhs );

    ByteBuffer( const ByteBuffer& rhs );

};

#endif // LANG_BYTEBUFFER_H
