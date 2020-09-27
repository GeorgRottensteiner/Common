#ifndef DES_H_INCLUDED
#define DES_H_INCLUDED

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>

#define ENO	0	// ENC
#define DE1	1	// DEC

typedef struct
{
	unsigned long 	ek[32];
	unsigned long   dk[32];
}	des_ctx;


namespace Codec
{
  namespace DES
  {

    bool    EncodeECB( ByteBuffer& memSource, const ByteBuffer& bbKey );
    bool    DecodeECB( ByteBuffer& memSource, const ByteBuffer& bbKey );

    bool    EncodeCBC( ByteBuffer& memSource, const ByteBuffer& bbKey, const ByteBuffer& bbIV = ByteBuffer(), const GR::u32 dwBytesPerBlock = 8 );
    bool    DecodeCBC( ByteBuffer& memSource, const ByteBuffer& bbKey, const ByteBuffer& bbIV = ByteBuffer(), const GR::u32 dwBytesPerBlock = 8 );

    bool    TripleEncode( ByteBuffer& memSource, const ByteBuffer& bbKey1, const ByteBuffer& bbKey2, const ByteBuffer& bbKey3 );
    bool    TripleDecode( ByteBuffer& memSource, const ByteBuffer& bbKey1, const ByteBuffer& bbKey2, const ByteBuffer& bbKey3 );
		
  }
}



#endif // DES_H_INCLUDED
