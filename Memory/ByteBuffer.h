#pragma once

#include <GR/GRTypes.h>

#include <Misc/Format.h>



class ByteBuffer
{

  protected:

    GR::u8*         m_pData;

    size_t          m_Size,
                    m_AllocatedSize;


  public:


    ByteBuffer();
    explicit ByteBuffer( size_t Size );
    explicit ByteBuffer( const GR::String& HexData );
    ByteBuffer( const void* pData, size_t Size );
    ByteBuffer( const ByteBuffer& rhs );

    ~ByteBuffer();

    size_t          Size() const;
    size_t          AllocatedSize() const;
    bool            Empty() const;
    void*           Data();
    const void*     Data() const;
    void*           DataAt( size_t Index );
    const void*     DataAt( size_t Index ) const;

    void            Clear();
    void            Set( const void* pData, const size_t Size );

    void            Grow( size_t Size, bool SetPos = false );
    void            Reserve( size_t Size );
    void            Resize( size_t Size );
    void            AppendBuffer( const ByteBuffer& Buffer );
    void            AppendData( const void* pData, const size_t Size );
    void            AppendI32( const GR::i32 Value );
    void            AppendI32NetworkOrder( const GR::i32 Value );
    void            AppendU32( const GR::u32 Data );
    void            AppendU32NetworkOrder( const GR::u32 Data );
    void            AppendU64( const GR::u64 Data );
    void            AppendU8( const GR::u8 Data );
    void            AppendU16( const GR::u16 Data );
    void            AppendU16NetworkOrder( const GR::u16 Data );
    void            AppendI16( const GR::i16 Data );
    void            AppendI16NetworkOrder( const GR::i16 Data );
    void            AppendF32( const GR::f32 Float );
    void            AppendString( const GR::String& Text );
    void            AppendString( const GR::WString& Text );

    int             Compare( const ByteBuffer& Buffer ) const;
    int             Compare( const void* pData, size_t DataLength ) const;

    size_t          Find( const ByteBuffer& FindBlock ) const;
    size_t          FindU8( const GR::u8 Byte, const size_t Offset = 0 ) const;

    bool            InsertByteAt( size_t Offset, GR::u8 Byte );
    bool            InsertBytesAt( size_t Offset, size_t NumBytes, GR::u8 ByteValue = 0 );
    bool            Truncate( size_t BytesToCut );
    bool            TruncateAt( size_t RemainingLength );
    bool            TruncateFront( size_t BytesToCut );


    GR::String      ToString( size_t StartOffset = 0, size_t Length = (size_t)-1 ) const;
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
    GR::u64         U64NetworkOrderAt( size_t Index ) const;
    void            SetByteAt( size_t Index, GR::u8 Byte );
    void            SetU16At( size_t Index, GR::u16 Word );
    void            SetU16NetworkOrderAt( size_t Index, GR::u16 Word );
    void            SetU32At( size_t Index, GR::u32 DWord );
    void            SetU32NetworkOrderAt( size_t Index, GR::u32 DWord );
    void            SetU64At( size_t Index, GR::u64 QWord );
    void            SetU64NetworkOrderAt( size_t Index, GR::u64 QWord );
    void            SetDataAt( size_t Index, const void* pData, const size_t Size );

    ByteBuffer      SubBuffer( size_t Index ) const;
    ByteBuffer      SubBuffer( size_t Index, size_t Count ) const;


    ByteBuffer      operator+( const ByteBuffer& rhs ) const;
    ByteBuffer&     operator+=( const ByteBuffer& rhs );
    ByteBuffer&     operator=( const ByteBuffer& rhs );
    bool            operator==( const ByteBuffer& rhs ) const;
    bool            operator!=( const ByteBuffer& rhs ) const;

    GR::u8&         operator[] ( const size_t Index );
    const GR::u8&   operator[] ( const size_t Index ) const;

};



Misc::CFormat operator<< ( Misc::CFormat Format, const ByteBuffer& Param );

