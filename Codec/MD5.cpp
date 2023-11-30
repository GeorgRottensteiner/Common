// MD5.CC - source code for the C++/object oriented translation and
//          modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug

// This translation/ modification is provided "as is," without express or
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think
// it does; (2) that this translation/ modification is accurate; or (3) that
// this software is "merchantible."  (Language for this disclaimer partially
// copied from the disclaimer below).

/* based on:

   MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
   MDDRIVER.C - test driver for MD2, MD4 and MD5


   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

 */



#include "md5.h"

#include <debug/debugclient.h>

#include <Memory/ByteBuffer.h>

#include <OS/OS.h>



namespace GR
{

  namespace Codec
  {

    MD5::MD5() :
      m_Finalized( false )
    {
      Initialise();
    }



    ByteBuffer MD5::Hash() const
    {
      ByteBuffer    digestBuffer( 16 );

      if ( !m_Finalized )
      {
        dh::Log( "MD5::raw_digest:  Can't get digest if you haven't finalized the digest!\n" );
        return ByteBuffer();
      }
      memcpy( (unsigned char*)digestBuffer.Data(), m_Digest, 16 );
      return digestBuffer;
    }



    void MD5::TransformBlock( GR::u8 block[64] )
    {
      GR::u32 a = m_State[0]; 
      GR::u32 b = m_State[1];
      GR::u32 c = m_State[2];
      GR::u32 d = m_State[3];
      GR::u32 x[16];

      decode( x, block, 64 );

      // Round 1
      FF( a, b, c, d,  x[0], S11, 0xd76aa478 ); 
      FF( d, a, b, c,  x[1], S12, 0xe8c7b756 ); 
      FF( c, d, a, b,  x[2], S13, 0x242070db ); 
      FF( b, c, d, a,  x[3], S14, 0xc1bdceee ); 
      FF( a, b, c, d,  x[4], S11, 0xf57c0faf ); 
      FF( d, a, b, c,  x[5], S12, 0x4787c62a ); 
      FF( c, d, a, b,  x[6], S13, 0xa8304613 ); 
      FF( b, c, d, a,  x[7], S14, 0xfd469501 ); 
      FF( a, b, c, d,  x[8], S11, 0x698098d8 ); 
      FF( d, a, b, c,  x[9], S12, 0x8b44f7af ); 
      FF( c, d, a, b, x[10], S13, 0xffff5bb1 );
      FF( b, c, d, a, x[11], S14, 0x895cd7be );
      FF( a, b, c, d, x[12], S11, 0x6b901122 );
      FF( d, a, b, c, x[13], S12, 0xfd987193 );
      FF( c, d, a, b, x[14], S13, 0xa679438e );
      FF( b, c, d, a, x[15], S14, 0x49b40821 );

      // Round 2
      GG( a, b, c, d,  x[1], S21, 0xf61e2562 ); 
      GG( d, a, b, c,  x[6], S22, 0xc040b340 ); 
      GG( c, d, a, b, x[11], S23, 0x265e5a51 );
      GG( b, c, d, a,  x[0], S24, 0xe9b6c7aa ); 
      GG( a, b, c, d,  x[5], S21, 0xd62f105d ); 
      GG( d, a, b, c, x[10], S22, 0x2441453 ); 
      GG( c, d, a, b, x[15], S23, 0xd8a1e681 );
      GG( b, c, d, a,  x[4], S24, 0xe7d3fbc8 ); 
      GG( a, b, c, d,  x[9], S21, 0x21e1cde6 ); 
      GG( d, a, b, c, x[14], S22, 0xc33707d6 );
      GG( c, d, a, b,  x[3], S23, 0xf4d50d87 ); 
      GG( b, c, d, a,  x[8], S24, 0x455a14ed ); 
      GG( a, b, c, d, x[13], S21, 0xa9e3e905 );
      GG( d, a, b, c,  x[2], S22, 0xfcefa3f8 ); 
      GG( c, d, a, b,  x[7], S23, 0x676f02d9 ); 
      GG( b, c, d, a, x[12], S24, 0x8d2a4c8a );

      // Round 3
      HH( a, b, c, d,  x[5], S31, 0xfffa3942 ); 
      HH( d, a, b, c,  x[8], S32, 0x8771f681 ); 
      HH( c, d, a, b, x[11], S33, 0x6d9d6122 );
      HH( b, c, d, a, x[14], S34, 0xfde5380c );
      HH( a, b, c, d,  x[1], S31, 0xa4beea44 ); 
      HH( d, a, b, c,  x[4], S32, 0x4bdecfa9 ); 
      HH( c, d, a, b,  x[7], S33, 0xf6bb4b60 ); 
      HH( b, c, d, a, x[10], S34, 0xbebfbc70 );
      HH( a, b, c, d, x[13], S31, 0x289b7ec6 );
      HH( d, a, b, c,  x[0], S32, 0xeaa127fa ); 
      HH( c, d, a, b,  x[3], S33, 0xd4ef3085 ); 
      HH( b, c, d, a,  x[6], S34, 0x4881d05 ); 
      HH( a, b, c, d,  x[9], S31, 0xd9d4d039 ); 
      HH( d, a, b, c, x[12], S32, 0xe6db99e5 );
      HH( c, d, a, b, x[15], S33, 0x1fa27cf8 );
      HH( b, c, d, a,  x[2], S34, 0xc4ac5665 ); 

      // Round 4
      II( a, b, c, d,  x[0], S41, 0xf4292244 ); 
      II( d, a, b, c,  x[7], S42, 0x432aff97 ); 
      II( c, d, a, b, x[14], S43, 0xab9423a7 );
      II( b, c, d, a,  x[5], S44, 0xfc93a039 ); 
      II( a, b, c, d, x[12], S41, 0x655b59c3 );
      II( d, a, b, c,  x[3], S42, 0x8f0ccc92 ); 
      II( c, d, a, b, x[10], S43, 0xffeff47d );
      II( b, c, d, a,  x[1], S44, 0x85845dd1 ); 
      II( a, b, c, d,  x[8], S41, 0x6fa87e4f ); 
      II( d, a, b, c, x[15], S42, 0xfe2ce6e0 );
      II( c, d, a, b,  x[6], S43, 0xa3014314 ); 
      II( b, c, d, a, x[13], S44, 0x4e0811a1 );
      II( a, b, c, d,  x[4], S41, 0xf7537e82 ); 
      II( d, a, b, c, x[11], S42, 0xbd3af235 );
      II( c, d, a, b,  x[2], S43, 0x2ad7d2bb ); 
      II( b, c, d, a,  x[9], S44, 0xeb86d391 ); 

      m_State[0] += a;
      m_State[1] += b;
      m_State[2] += c;
      m_State[3] += d;

      // Zeroize sensitive information.
      memset( (GR::u8*)x, 0, sizeof( x ) );
    }



    // Encodes input (GR::u32) into output (unsigned char). Assumes len is a multiple of 4.
    void MD5::encode( GR::u8* output, GR::u32* input, GR::u32 len )
    {
      unsigned int i, j;

      for ( i = 0, j = 0; j < len; i++, j += 4 )
      {
        output[j]     = (GR::u8)( input[i] & 0xff );
        output[j + 1] = (GR::u8)( ( input[i] >> 8 ) & 0xff );
        output[j + 2] = (GR::u8)( ( input[i] >> 16 ) & 0xff );
        output[j + 3] = (GR::u8)( ( input[i] >> 24 ) & 0xff );
      }
    }



    // Decodes input (unsigned char) into output (GR::u32). Assumes len is a multiple of 4.
    void MD5::decode( GR::u32* output, GR::u8* input, GR::u32 len )
    {
      unsigned int i, j;

      for ( i = 0, j = 0; j < len; i++, j += 4 )
      {
        output[i] = ( (GR::u32)input[j] ) 
                | ( ( (GR::u32)input[j + 1] ) << 8 ) 
                | ( ( (GR::u32)input[j + 2] ) << 16 ) 
                | ( ( (GR::u32)input[j + 3] ) << 24 );
      }
    }



    // Note: Replace "for loop" with standard memcpy if possible.
    void MD5::memcpy( GR::u8* output, const GR::u8* input, GR::u32 len )
    {
      for ( GR::u32 i = 0; i < len; i++ )
      {
        output[i] = input[i];
      }
    }



    // Note: Replace "for loop" with standard memset if possible.
    void MD5::memset( GR::u8* output, GR::u8 value, GR::u32 len )
    {
      for ( GR::u32 i = 0; i < len; i++ )
      {
        output[i] = value;
      }
    }



    inline unsigned int MD5::RotateLeft( GR::u32 x, GR::u32 n )
    {
      return ( x << n ) | ( x >> ( 32 - n ) );
    }



    inline unsigned int MD5::F( GR::u32 x, GR::u32 y, GR::u32 z )
    {
      return ( x & y ) | ( ~x & z );
    }



    inline unsigned int MD5::G( GR::u32 x, GR::u32 y, GR::u32 z )
    {
      return ( x & z ) | ( y & ~z );
    }



    inline unsigned int MD5::H( GR::u32 x, GR::u32 y, GR::u32 z )
    {
      return x ^ y ^ z;
    }



    inline unsigned int MD5::I( GR::u32 x, GR::u32 y, GR::u32 z )
    {
      return y ^ ( x | ~z );
    }



    // FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
    // Rotation is separate from addition to prevent recomputation.
    inline void MD5::FF( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32  s, GR::u32 ac )
    {
      a += F( b, c, d ) + x + ac;
      a = RotateLeft( a, s ) + b;
    }



    inline void MD5::GG( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac )
    {
      a += G( b, c, d ) + x + ac;
      a = RotateLeft( a, s ) + b;
    }



    inline void MD5::HH( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac )
    {
      a += H( b, c, d ) + x + ac;
      a = RotateLeft( a, s ) + b;
    }



    inline void MD5::II( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac )
    {
      a += I( b, c, d ) + x + ac;
      a = RotateLeft( a, s ) + b;
    }



    bool MD5::Calculate( const ByteBuffer& Data, ByteBuffer& Hash )
    {
      MD5    Instance;

      Hash = Instance.Calculate( Data );

      return true;
    }



    bool MD5::CalcKeyedHash( const ByteBuffer& Data, const ByteBuffer& Key, ByteBuffer& Hash )
    {
      GR::u32     blockSize = 64;

      // if key is longer than 64 bytes reset it to key=Hash(key)
      ByteBuffer    keyToUse = Key;
      if ( keyToUse.Size() > blockSize )
      {
        if ( !Calculate( Key, keyToUse ) )
        {
          return false;
        }
      }

      // the HMAC_MD5 transform looks like:
      // MD5(K XOR opad, MD5(K XOR ipad, text))
      //  where K is an n byte key
      //  ipad is the byte 0x36 repeated 64 times
      //  opad is the byte 0x5c repeated 64 times
      //  and text is the data being protected

      // start out by storing key in pads
      ByteBuffer    innerPad( blockSize );
      ByteBuffer    outerPad( blockSize );

      ::memcpy( innerPad.Data(), Key.Data(), Key.Size() );
      ::memcpy( outerPad.Data(), Key.Data(), Key.Size() );

      // XOR key with ipad and opad values
      for ( GR::u32 i = 0; i < blockSize; i++ )
      {
        innerPad.SetByteAt( i, (GR::u8)( innerPad.ByteAt( i ) ^ 0x36 ) );
        outerPad.SetByteAt( i, (GR::u8)( outerPad.ByteAt( i ) ^ 0x5c ) );
      }

      ByteBuffer    innerResult;

      if ( !Calculate( innerPad + Data, innerResult ) )
      {
        return false;
      }
      return Calculate( outerPad + innerResult, Hash );
    }



    ByteBuffer MD5::Calculate( const ByteBuffer& Data )
    {
      MD5    Instance;

      Instance.Initialise();
      Instance.Update( Data );
      Instance.Finalize();

      ByteBuffer hash( 16 );
      ::memcpy( (void*)hash.Data(), Instance.m_Digest, 16 );
      return hash;
    }



    void MD5::Initialise()
    {
      m_Finalized = false;

      // Nothing counted, so count=0
      m_CountOfBits[0] = 0;
      m_CountOfBits[1] = 0;

      // Load magic initialization constants.
      m_State[0] = 0x67452301;
      m_State[1] = 0xefcdab89;
      m_State[2] = 0x98badcfe;
      m_State[3] = 0x10325476;
    }



    bool MD5::Update( const ByteBuffer& Data, GR::u32 NumOfBytes )
    {
      GR::u32 input_index, buffer_index;
      GR::u32 buffer_space;                // how much space is left in buffer

      if ( m_Finalized )
      {
        // so we can't update!
        dh::Log( "MD5::update:  Can't update a finalized digest!\n" );
        return false;
      }

      if ( NumOfBytes == 0 )
      {
        NumOfBytes = (GR::u32)Data.Size();
      }

      // Compute number of bytes mod 64
      buffer_index = (unsigned int)( ( m_CountOfBits[0] >> 3 ) & 0x3F );

      // Update number of bits
      if ( ( m_CountOfBits[0] += ( NumOfBytes << 3 ) ) < ( NumOfBytes << 3 ) )
        m_CountOfBits[1]++;

      m_CountOfBits[1] += ( NumOfBytes >> 29 );


      buffer_space = 64 - buffer_index;  // how much space is left in buffer

      // Transform as many times as possible.
      if ( Data.Size() >= buffer_space )
      { 
        // ie. we have enough to fill the buffer
        // fill the rest of the buffer and transform
        memcpy( m_InputBuffer + buffer_index, (GR::u8*)Data.Data(), buffer_space );
        TransformBlock( m_InputBuffer );

        // now, transform each 64-byte piece of the input, bypassing the buffer
        for ( input_index = buffer_space; input_index + 63 < Data.Size(); input_index += 64 )
        {
          TransformBlock( (GR::u8*)Data.Data() + input_index );
        }
        buffer_index = 0;  // so we can buffer remaining
      }
      else
        input_index = 0;     // so we can buffer the whole input


      // and here we do the buffering:
      memcpy( m_InputBuffer + buffer_index, (GR::u8*)Data.Data() + input_index, NumOfBytes - input_index );
      return true;
    }



    ByteBuffer MD5::Finalize()
    {
      unsigned char bits[8];
      unsigned int index, padLen;
      static GR::u8 PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };

      if ( m_Finalized )
      {
        return Hash();
      }

      // Save number of bits
      encode( bits, m_CountOfBits, 8 );

      // Pad out to 56 mod 64.
      index = (GR::u32)( ( m_CountOfBits[0] >> 3 ) & 0x3f );
      padLen = ( index < 56 ) ? ( 56 - index ) : ( 120 - index );

      ByteBuffer  padding( PADDING, padLen );
      Update( padding );

      // Append length (before padding)
      ByteBuffer  bitsData( bits, 8 );
      Update( bitsData );

      // Store state in digest
      encode( m_Digest, m_State, 16 );

      // Zeroize sensitive information
      memset( m_InputBuffer, 0, sizeof( *m_InputBuffer ) );

      m_Finalized = true;

      return Hash();
    }



    int MD5::HashSize() const
    {
      return 16;
    }



  }


}
