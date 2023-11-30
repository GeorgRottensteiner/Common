#include "ASN1Format.h"

#include <Math/BigInteger.h>

#include <debug/debugclient.h>



bool ASN1Format::ReadTag( IIOStream& IOIn, Tag& Tag )
{
	GR::u8		nextByte = IOIn.ReadU8();

	bool			tagIsConstructed = ( nextByte >> 5 ); // 0 = primitive

	Tag.Class					= (ASN1Format::TagClass)( ( nextByte >> 6 ) & 0x03 );
	Tag.Type					= nextByte & 0x1f;
	Tag.HeaderLength	= 1;

	switch ( Tag.Class )
	{
		case TagClass::APPLICATION:
			break;
		case TagClass::UNIVERSAL:
			break;
		case TagClass::CONTEXT_SPECIFIC:
			break;
		case TagClass::PRIVATE:
			break;
	}

	// If the tag number( 6 LSB of first byte ) we already extracted is less
	// than 0x1f, then it directly represents the tag number( which is <=
	// 30 ) and our work is over.Otherwise( t == 0x1f ) the real tag number
	// is encoded on multiple bytes following the first byte.Note that we
	// limit ourselves to tag encoded on less than 28 bits, i.e.only accept
	// at most 4 bytes( only 7 LSB of each byte will count because MSB tells
	// if this is the last ).
	if ( Tag.Type == 0x1f )
	{
		do
		{
			nextByte = IOIn.ReadU8();
			++Tag.HeaderLength;

			Tag.Type <<= 7;
			Tag.Type |= nextByte & 0x7f;
		}
		while ( nextByte & 0x80 );
	}

	GR::u32 tagLength = IOIn.ReadU8();
	++Tag.HeaderLength;
	if ( !( tagLength & 0x80 ) )
	{
		Tag.TagData.Resize( tagLength );
		return IOIn.ReadBlock( Tag.TagData.Data(), tagLength ) == tagLength;
	}

	int		tagLengthLength = tagLength & 0x7f;
	if ( ( tagLengthLength < 1 )
  ||	 ( tagLengthLength > 4 ) )
	{
		return false;
	}

	switch ( tagLengthLength )
	{
		case 1:
			tagLength = IOIn.ReadU8();
			break;
		case 2:
			tagLength = IOIn.ReadU16NetworkOrder();
			break;
		case 3:
			tagLength = ( IOIn.ReadU16NetworkOrder() << 8 ) + IOIn.ReadU8();
			break;
		case 4:
			tagLength = IOIn.ReadU32NetworkOrder();
			break;
		default:
			return false;
	}
	Tag.TagData.Resize( tagLength );

	return IOIn.ReadBlock( Tag.TagData.Data(), tagLength ) == tagLength;
}



bool ASN1Format::OnNextTagRead( TagClass& TagClass, GR::u32& Tag, GR::u32& TagLength )
{
	//printf( "Tag %x, length %d: ", Tag, TagLength );
	if ( TagReadHandler )
	{
		return TagReadHandler( TagClass, Tag, TagLength );
	}
	return true;
}



bool ASN1Format::Read( IIOStream& IOIn )
{
	Tag					tag;

	while ( IOIn.DataAvailable() )
	{
		if ( !ReadTag( IOIn, tag ) )
		{
			return false;
		}

		TagStack.push_back( tag.Type );

		/*
		if ( !OnNextTagRead( tag.Class, tag.TagNumber, tag.TagData.Size() ) )
		{
			return false;
		}*/
		GR::u32			tagLength = 0;

		switch ( (ASNType)tag.Type )
		{
			case ASNType::ASN1_TYPE_SEQUENCE:
				if ( !Read( IOIn ) )
				{
					TagStack.pop_back();
					return false;
				}
				break;
			case ASNType::ASN1_TYPE_INTEGER:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_OID:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_PrintableString:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_UTF8_STRING:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_OCTET_STRING:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_SET:
				if ( !Read( IOIn ) )
				{
					TagStack.pop_back();
					return false;
				}
				break;
			case ASNType::ASN1_TYPE_UTCTime:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_BOOLEAN:
				{
					ByteBuffer		integer( tagLength );

					IOIn.ReadBlock( integer.Data(), tagLength );
				}
				break;
			case ASNType::ASN1_TYPE_BIT_STRING:
				if ( tag.Class != TagClass::CONTEXT_SPECIFIC )
				{
					{
						ByteBuffer		integer( tagLength );

						IOIn.ReadBlock( integer.Data(), tagLength );
					}
				}
				else
				{
					// is that so? always with inner blocks?
					if ( !Read( IOIn ) )
					{
						TagStack.pop_back();
						return false;
					}
				}
				break;
			case ASNType::ASN1_TYPE_NULL:
				if ( tagLength != 0 )
				{
					TagStack.pop_back();
					return false;
				}
				break;
			default:
				printf( "something\n" );
				if ( !Read( IOIn ) )
				{
					TagStack.pop_back();
					return false;
				}
				TagStack.pop_back();
				return false;
		}
		/*
		ASN1_TYPE_BOOLEAN = 0x01,
			ASN1_TYPE_INTEGER = 0x02,
			ASN1_TYPE_BIT_STRING = 0x03,
			ASN1_TYPE_OCTET_STRING = 0x04,
			ASN1_TYPE_NULL = 0x05,
			ASN1_TYPE_OID = 0x06,
			ASN1_TYPE_ENUMERATED = 0x0a,
			ASN1_TYPE_SEQUENCE = 0x10,
			ASN1_TYPE_SET = 0x11,
			ASN1_TYPE_PrintableString = 0x13,
			ASN1_TYPE_T61String = 0x14,
			ASN1_TYPE_IA5String = 0x16,
			ASN1_TYPE_UTCTime = 0x17,
			ASN1_TYPE_GeneralizedTime = 0x18,
			*/


	}

	return true;
}



int ASN1Format::LengthOfLength( int Length )
{
	if ( Length < 128 )
	{
		return 1;
	}
	if ( Length < 0x10000 )
	{
		return 2;
	}
	if ( Length < 0x1000000 )
	{
		return 3;
	}
	if ( Length < 0x100000000 )
	{
		return 4;
	}
	dh::Log( "ASN1Format::LengthOfLength exceeds expected length %d", Length );
	return 0;
}



ByteBuffer ASN1Format::EncodeLength( int Length )
{
	ByteBuffer		result;
	int						length = LengthOfLength( Length );

	if ( length == 1 )
	{
		result.AppendU8( (GR::u8)Length );
		return result;
	}
	result.AppendU8( (GR::u8)( 0x80 | length ) );
	if ( length == 2 )
	{
		result.AppendU16NetworkOrder( (GR::u16)length );
	}
	else if ( length == 3 )
	{
		result.AppendU8( (GR::u8)( length >> 16 ) );
		result.AppendU16NetworkOrder( (GR::u16)length );
	}
	else if ( length == 4 )
	{
		result.AppendU32NetworkOrder( (GR::u32)length );
	}
	else
	{
		dh::Log( "ASN1Format::EncodeLength length exceeds max value" );
	}
	return result;
}



ByteBuffer ASN1Format::EncodeObjectIdentifer( const GR::String& OIDHex )
{
	ByteBuffer		oidData( OIDHex );

	return EncodeObject( ASN1Format::ASNType::ASN1_TYPE_OID, ASN1Format::TagClass::UNIVERSAL, oidData );
}



ByteBuffer ASN1Format::EncodeInteger( const BigInteger& Integer )
{
	return EncodeObject( ASN1Format::ASNType::ASN1_TYPE_INTEGER, ASN1Format::TagClass::UNIVERSAL, Integer.ToBuffer() );
}



ByteBuffer ASN1Format::EncodeNull()
{
	ByteBuffer		result = EncodeTag( ASN1Format::ASNType::ASN1_TYPE_NULL, ASN1Format::TagClass::UNIVERSAL );
	result.AppendU8( 0 );
	return result;
}



ByteBuffer ASN1Format::EncodeBitString( const ByteBuffer& Data )
{
	ByteBuffer		bitStringData = EncodeTag( ASN1Format::ASNType::ASN1_TYPE_BIT_STRING, ASN1Format::TagClass::UNIVERSAL );
	bitStringData += EncodeLength( (int)Data.Size() + 1 );

	// for some weird reason bit string requires this
	bitStringData.AppendU8( 0 );
	bitStringData.AppendBuffer( Data );

	return bitStringData;
}



ByteBuffer ASN1Format::EncodeTag( ASN1Format::ASNType Type, ASN1Format::TagClass TagClass )
{
	ByteBuffer		result;

	if ( ( Type == ASN1Format::ASNType::ASN1_TYPE_SEQUENCE )
	||	 ( Type == ASN1Format::ASNType::ASN1_TYPE_SET ) )
	{
		// constructed bit set
		result.AppendU8( ( (GR::u8)Type ) | 0x20 | ( ( (GR::u8)TagClass ) << 6 ) );
	}
	else
	{
		result.AppendU8( ( (GR::u8)Type ) | ( ( (GR::u8)TagClass ) << 6 ) );
	}
	return result;
}



ByteBuffer ASN1Format::AppendContent( ByteBuffer& Source, const ByteBuffer& Data )
{
	Source.AppendBuffer( EncodeLength( (int)Data.Size() ) );
	Source.AppendBuffer( Data );

	return Source;
}



ByteBuffer ASN1Format::EncodeObject( ASN1Format::ASNType Type, ASN1Format::TagClass TagClass, const ByteBuffer& Data )
{
	int		lengthOfData		= LengthOfLength( (int)Data.Size() );
	int		requiredLength	= 1 + (int)Data.Size();

	ByteBuffer		result = EncodeTag( Type, TagClass );

	return AppendContent( result, Data );
}