#include "SHA-1.h"

#include "SHA256-384-512.h"
#include "SHA256-384-512.cpp"


namespace Codec
{

  SHA1::SHA1()
  {
    m_block = (SHA1_WORKSPACE_BLOCK*)m_workspace;
    Reset();
  }

  SHA1::~SHA1()
  {
    Reset();
  }


  GR::u32 SHA1::ROL32( GR::u32 _val32, int _nBits )
  {
#ifdef _MSC_VER
    return _rotl( _val32, _nBits );
#else
    return ( _val32 << _nBits ) | ( _val32 >> ( 32 - _nBits ) );
#endif
  }

  GR::u32 SHA1::SHABLK0( int i )
  {
#if OS_ENDIAN == OS_ENDIAN_LITTLE
    return ( m_block->l[i] = ( ROL32( m_block->l[i], 24 ) & 0xFF00FF00 ) | ( ROL32( m_block->l[i], 8 ) & 0x00FF00FF ) );
#else
    return m_block->l[i];
#endif
  }

  GR::u32 SHA1::SHABLK( int i )
  {
    return ( m_block->l[i & 15] = ROL32( m_block->l[( i + 13 ) & 15 ] ^ m_block->l[( i + 8 ) & 15] ^ m_block->l[( i + 2 ) & 15] ^ m_block->l[i & 15], 1 ) );
  }

  void SHA1::_R0( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i )
  {
    z += ( ( w & ( x ^ y ) ) ^ y ) + SHABLK0( i ) + 0x5A827999 + ROL32( v, 5 );
    w = ROL32( w,30 );
  }

  void SHA1::_R1( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i )
  {
    z += ( ( w & ( x ^ y ) ) ^ y ) + SHABLK( i ) + 0x5A827999 + ROL32( v, 5 );
    w = ROL32( w, 30 );
  }

  void SHA1::_R2( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i )
  {
    z += ( w ^ x ^ y ) + SHABLK( i ) + 0x6ED9EBA1 + ROL32( v, 5 );
    w = ROL32( w, 30 );
  }

  void SHA1::_R3( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i )
  {
    z += ( ( ( w | x ) & y ) | ( w & x ) ) + SHABLK( i ) + 0x8F1BBCDC + ROL32( v, 5 );
    w = ROL32( w, 30 );
  }

  void SHA1::_R4( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i )
  {
    z += ( w ^ x ^ y ) + SHABLK( i ) + 0xCA62C1D6 + ROL32( v, 5 );
    w = ROL32( w, 30 );
  }



  void SHA1::Reset()
  {
    // SHA1 initialization constants
    m_state[0] = 0x67452301;
    m_state[1] = 0xEFCDAB89;
    m_state[2] = 0x98BADCFE;
    m_state[3] = 0x10325476;
    m_state[4] = 0xC3D2E1F0;

    m_count[0] = 0;
    m_count[1] = 0;
  }



  void SHA1::Transform( GR::u32* pState, const GR::u8* pBuffer )
  {
    GR::u32         a = pState[0], 
                    b = pState[1], 
                    c = pState[2], 
                    d = pState[3], 
                    e = pState[4];

    memcpy( m_block, pBuffer, 64 );

    // 4 rounds of 20 operations each. Loop unrolled.
    _R0(a,b,c,d,e, 0); _R0(e,a,b,c,d, 1); _R0(d,e,a,b,c, 2); _R0(c,d,e,a,b, 3);
    _R0(b,c,d,e,a, 4); _R0(a,b,c,d,e, 5); _R0(e,a,b,c,d, 6); _R0(d,e,a,b,c, 7);
    _R0(c,d,e,a,b, 8); _R0(b,c,d,e,a, 9); _R0(a,b,c,d,e,10); _R0(e,a,b,c,d,11);
    _R0(d,e,a,b,c,12); _R0(c,d,e,a,b,13); _R0(b,c,d,e,a,14); _R0(a,b,c,d,e,15);
    _R1(e,a,b,c,d,16); _R1(d,e,a,b,c,17); _R1(c,d,e,a,b,18); _R1(b,c,d,e,a,19);
    _R2(a,b,c,d,e,20); _R2(e,a,b,c,d,21); _R2(d,e,a,b,c,22); _R2(c,d,e,a,b,23);
    _R2(b,c,d,e,a,24); _R2(a,b,c,d,e,25); _R2(e,a,b,c,d,26); _R2(d,e,a,b,c,27);
    _R2(c,d,e,a,b,28); _R2(b,c,d,e,a,29); _R2(a,b,c,d,e,30); _R2(e,a,b,c,d,31);
    _R2(d,e,a,b,c,32); _R2(c,d,e,a,b,33); _R2(b,c,d,e,a,34); _R2(a,b,c,d,e,35);
    _R2(e,a,b,c,d,36); _R2(d,e,a,b,c,37); _R2(c,d,e,a,b,38); _R2(b,c,d,e,a,39);
    _R3(a,b,c,d,e,40); _R3(e,a,b,c,d,41); _R3(d,e,a,b,c,42); _R3(c,d,e,a,b,43);
    _R3(b,c,d,e,a,44); _R3(a,b,c,d,e,45); _R3(e,a,b,c,d,46); _R3(d,e,a,b,c,47);
    _R3(c,d,e,a,b,48); _R3(b,c,d,e,a,49); _R3(a,b,c,d,e,50); _R3(e,a,b,c,d,51);
    _R3(d,e,a,b,c,52); _R3(c,d,e,a,b,53); _R3(b,c,d,e,a,54); _R3(a,b,c,d,e,55);
    _R3(e,a,b,c,d,56); _R3(d,e,a,b,c,57); _R3(c,d,e,a,b,58); _R3(b,c,d,e,a,59);
    _R4(a,b,c,d,e,60); _R4(e,a,b,c,d,61); _R4(d,e,a,b,c,62); _R4(c,d,e,a,b,63);
    _R4(b,c,d,e,a,64); _R4(a,b,c,d,e,65); _R4(e,a,b,c,d,66); _R4(d,e,a,b,c,67);
    _R4(c,d,e,a,b,68); _R4(b,c,d,e,a,69); _R4(a,b,c,d,e,70); _R4(e,a,b,c,d,71);
    _R4(d,e,a,b,c,72); _R4(c,d,e,a,b,73); _R4(b,c,d,e,a,74); _R4(a,b,c,d,e,75);
    _R4(e,a,b,c,d,76); _R4(d,e,a,b,c,77); _R4(c,d,e,a,b,78); _R4(b,c,d,e,a,79);

    // Add the working vars back into state
    pState[0] += a;
    pState[1] += b;
    pState[2] += c;
    pState[3] += d;
    pState[4] += e;

    // Wipe variables
    a = b = c = d = e = 0;
  }



  // Use this function to hash in binary data and strings
  void SHA1::Update( const GR::u8* pbData, GR::u32 uLen )
  {
    GR::u32 j = ( ( m_count[0] >> 3 ) & 0x3F );

    if ( ( m_count[0] += ( uLen << 3 ) ) < ( uLen << 3 ) )
    {
      // Overflow
      ++m_count[1]; 
    }

    m_count[1] += ( uLen >> 29 );

    GR::u32 i = 0;

    if ( ( j + uLen ) > 63 )
    {
      i = 64 - j;
      memcpy( &m_buffer[j], pbData, i );
      Transform( m_state, m_buffer );

      for ( ; ( i + 63 ) < uLen; i += 64 )
      {
        Transform( m_state, &pbData[i] );
      }
      j = 0;
    }

    if ( ( uLen - i ) != 0 )
    {
      memcpy( &m_buffer[j], &pbData[i], uLen - i );
    }
  }



  void SHA1::Final()
  {
    GR::u32     i;

    GR::u8      finalcount[8];

    for ( i = 0; i < 8; ++i )
    {
      // Endian independent
      finalcount[i] = (GR::u8)( ( m_count[( ( i >= 4 ) ? 0 : 1 )] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 ); 
    }

    Update( (GR::u8*)"\200", 1 );

    while ( ( m_count[0] & 504 ) != 448 )
    {
      Update( (GR::u8*)"\0", 1 );
    }

    // Cause a SHA1Transform()
    Update( finalcount, 8 ); 

    for ( i = 0; i < 20; ++i )
    {
      m_digest[i] = (GR::u8)( ( m_state[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 0xFF );
    }

    // Wipe variables for security reasons
    memset( m_buffer, 0, 64 );
    memset( m_state, 0, 20 );
    memset( m_count, 0, 8 );
    memset( finalcount, 0, 8 );
    Transform( m_state, m_buffer );
  }

  // Get the raw message digest
  bool SHA1::GetHash( GR::u8* pbDest ) const
  {
    if ( pbDest == NULL )
    {
      return false;
    }
    memcpy( pbDest, m_digest, 20 );
    return true;
  }



  bool SHA1::CalcHash( const ByteBuffer& Data, ByteBuffer& Hash )
  {
    SHA1    Instance;

    Instance.Reset();
    Instance.Update( (const GR::u8*)Data.Data(), (GR::u32)Data.Size() );
    Instance.Final();

    Hash.Resize( 20 );
    memcpy( Hash.Data(), Instance.m_digest, 20 );
    return true;
  }



  bool SHA1::CalcHash256( const ByteBuffer& Data, ByteBuffer& Hash )
  {
    SHA256_CTX    context;
    GR::u8        digest[32];

    SHA256_Init( &context );
    SHA256_Update(  &context, (GR::u8*)Data.Data(), Data.Size() );
    SHA256_Final( digest, &context );

    Hash.Resize( 32 );
    memcpy( Hash.Data(), digest, 32 );
    return true;
  }



  bool SHA1::CalcHash384( const ByteBuffer& Data, ByteBuffer& Hash )
  {
    SHA384_CTX    context;
    GR::u8        digest[48];

    SHA384_Init( &context );
    SHA384_Update(  &context, (GR::u8*)Data.Data(), Data.Size() );
    SHA384_Final( digest, &context );

    Hash.Resize( 48 );
    memcpy( Hash.Data(), digest, 48 );
    return true;
  }



  bool SHA1::CalcHash512( const ByteBuffer& Data, ByteBuffer& Hash )
  {
    SHA512_CTX    context;
    GR::u8        digest[64];

    SHA512_Init( &context );
    SHA512_Update(  &context, (GR::u8*)Data.Data(), Data.Size() );
    SHA512_Final( digest, &context );

    Hash.Resize( 64 );
    memcpy( Hash.Data(), digest, 64 );
    return true;
  }

  
  
  bool SHA1::CalcHashKeyed( const ByteBuffer& Data, 
                            const ByteBuffer& Key,
                            ByteBuffer& Hash )
  {
    GR::u32     blockSize = 64;
    
    // if key is longer than 64 bytes reset it to key=MD5(key)
    ByteBuffer    keyToUse = Key;
    if ( keyToUse.Size() > blockSize )
    {
      if ( !CalcHash( Key, keyToUse ) )
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
    
    ::memcpy( innerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    ::memcpy( outerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    
    // XOR key with ipad and opad values
    for ( GR::u32 i = 0; i < blockSize; i++ ) 
    {
      innerPad.SetByteAt( i, (GR::u8)( innerPad.ByteAt( i ) ^ 0x36 ) ); 
      outerPad.SetByteAt( i, (GR::u8)( outerPad.ByteAt( i ) ^ 0x5c ) );
    }
    
    ByteBuffer    innerResult;
    
    if ( !CalcHash( innerPad + Data, innerResult ) )
    {
      return false;
    }
    return CalcHash( outerPad + innerResult, Hash );
  }



  bool SHA1::CalcHash256Keyed( const ByteBuffer& Data, 
                               const ByteBuffer& Key,
                               ByteBuffer& Hash )
  {
    GR::u32     blockSize = 64;
    
    // if key is longer than 64 bytes reset it to key=MD5(key)
    ByteBuffer    keyToUse = Key;
    if ( keyToUse.Size() > blockSize )
    {
      if ( !CalcHash256( Key, keyToUse ) )
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
    
    ::memcpy( innerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    ::memcpy( outerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    
    // XOR key with ipad and opad values
    for ( GR::u32 i = 0; i < blockSize; i++ ) 
    {
      innerPad.SetByteAt( i, (GR::u8)( innerPad.ByteAt( i ) ^ 0x36 ) ); 
      outerPad.SetByteAt( i, (GR::u8)( outerPad.ByteAt( i ) ^ 0x5c ) );
    }
    
    ByteBuffer    innerResult;
    
    if ( !CalcHash256( innerPad + Data, innerResult ) )
    {
      return false;
    }
    return CalcHash256( outerPad + innerResult, Hash );
  }



  bool SHA1::CalcHash384Keyed( const ByteBuffer& Data, 
                               const ByteBuffer& Key,
                               ByteBuffer& Hash )
  {
    GR::u32     blockSize = 128;
    
    // if key is longer than 64 bytes reset it to key=MD5(key)
    ByteBuffer    keyToUse = Key;
    if ( keyToUse.Size() > blockSize )
    {
      if ( !CalcHash384( Key, keyToUse ) )
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
    
    ::memcpy( innerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    ::memcpy( outerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    
    // XOR key with ipad and opad values
    for ( GR::u32 i = 0; i < blockSize; i++ ) 
    {
      innerPad.SetByteAt( i, (GR::u8)( innerPad.ByteAt( i ) ^ 0x36 ) ); 
      outerPad.SetByteAt( i, (GR::u8)( outerPad.ByteAt( i ) ^ 0x5c ) );
    }
    
    ByteBuffer    innerResult;
    
    if ( !CalcHash384( innerPad + Data, innerResult ) )
    {
      return false;
    }
    return CalcHash384( outerPad + innerResult, Hash );
  }



  bool SHA1::CalcHash512Keyed( const ByteBuffer& Data, 
                               const ByteBuffer& Key,
                               ByteBuffer& Hash )
  {
    GR::u32     blockSize = 128;
    
    // if key is longer than 64 bytes reset it to key=MD5(key)
    ByteBuffer    keyToUse = Key;
    if ( keyToUse.Size() > blockSize )
    {
      if ( !CalcHash512( Key, keyToUse ) )
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
    
    ::memcpy( innerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    ::memcpy( outerPad.Data(), keyToUse.Data(), keyToUse.Size() );
    
    // XOR key with ipad and opad values
    for ( GR::u32 i = 0; i < blockSize; i++ ) 
    {
      innerPad.SetByteAt( i, (GR::u8)( innerPad.ByteAt( i ) ^ 0x36 ) ); 
      outerPad.SetByteAt( i, (GR::u8)( outerPad.ByteAt( i ) ^ 0x5c ) );
    }
    
    ByteBuffer    innerResult;
    
    if ( !CalcHash512( innerPad + Data, innerResult ) )
    {
      return false;
    }
    return CalcHash512( outerPad + innerResult, Hash );
  }

}

