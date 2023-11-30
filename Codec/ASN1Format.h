#pragma once


#include <Interface/IIOStream.h>

#include <vector>
#include <functional>



class BigInteger;

class ASN1Format
{
  public:

    enum class TagClass
    {
      UNIVERSAL					= 0x00,
      APPLICATION				= 0x01,
      CONTEXT_SPECIFIC	= 0x02,
      PRIVATE						= 0x03
    };

    enum class ASNType
    {
      ASN1_TYPE_BOOLEAN						= 0x01,
      ASN1_TYPE_INTEGER						= 0x02,
      ASN1_TYPE_BIT_STRING				= 0x03,
      ASN1_TYPE_OCTET_STRING			= 0x04,
      ASN1_TYPE_NULL			        = 0x05,
      ASN1_TYPE_OID								= 0x06,	
      ASN1_TYPE_ENUMERATED				= 0x0a,
      ASN1_TYPE_UTF8_STRING				= 0x0c,
      ASN1_TYPE_SEQUENCE					= 0x10,
      ASN1_TYPE_SET								= 0x11,
      ASN1_TYPE_PrintableString		= 0x13,
      ASN1_TYPE_T61String					= 0x14,
      ASN1_TYPE_IA5String					= 0x16,
      ASN1_TYPE_UTCTime						= 0x17,
      ASN1_TYPE_GeneralizedTime		= 0x18,
    };

    struct Tag
    {
      GR::u32       Type;
      TagClass      Class;
      GR::u32       HeaderLength;
      ByteBuffer    TagData;

      Tag() :
        Type( 0 ),
        Class( TagClass::UNIVERSAL ),
        HeaderLength( 0 )
      {
      }
    };


  private:

    std::vector<GR::u32>				TagStack;




    bool OnNextTagRead( TagClass& TagClass, GR::u32& Tag, GR::u32& TagLength );


  public:


    std::function<bool( TagClass&, GR::u32&, GR::u32& )>			TagReadHandler;


    bool Read( IIOStream& IOIn );

    static bool ReadTag( IIOStream& IOIn, Tag& Tag );


    static int        LengthOfLength( int Length );
    static ByteBuffer EncodeLength( int Length );

    static ByteBuffer EncodeObjectIdentifer( const GR::String& OIDHex );
    static ByteBuffer EncodeNull();
    static ByteBuffer EncodeInteger( const BigInteger& Integer );
    static ByteBuffer EncodeBitString( const ByteBuffer& Data );

    static ByteBuffer EncodeObject( ASN1Format::ASNType Type, ASN1Format::TagClass TagClass, const ByteBuffer& Data );
    static ByteBuffer EncodeTag( ASN1Format::ASNType Type, ASN1Format::TagClass TagClass );

    static ByteBuffer AppendContent( ByteBuffer& Source, const ByteBuffer& Data );


};

