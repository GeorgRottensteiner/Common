#ifndef _INCLUDE_BASE64_H__
#define _INCLUDE_BASE64_H__

#include <Memory/ByteBuffer.h>



class Base64
{

	public:

    static ByteBuffer    Encode( const ByteBuffer& memSource );
    static ByteBuffer    Decode( const ByteBuffer& memSource );
		

};



#endif // _INCLUDE_BASE64_H__
