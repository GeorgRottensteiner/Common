#include "SHA-1.h"



namespace GR
{

  namespace Codec
  {

    SHA1::SHA1()
    {
      m_pWorkspaceAsBlock = (SHA1_WORKSPACE_BLOCK*)m_Workspace;
      Initialise();
    }



    SHA1::~SHA1()
    {
      Initialise();
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
      return ( m_pWorkspaceAsBlock->l[i] = ( ROL32( m_pWorkspaceAsBlock->l[i], 24 ) & 0xFF00FF00 ) | ( ROL32( m_pWorkspaceAsBlock->l[i], 8 ) & 0x00FF00FF ) );
#else
      return m_pWorkspaceAsBlock->l[i];
#endif
    }



    GR::u32 SHA1::SHABLK( int i )
    {
      return ( m_pWorkspaceAsBlock->l[i & 15] = ROL32( m_pWorkspaceAsBlock->l[( i + 13 ) & 15] ^ m_pWorkspaceAsBlock->l[( i + 8 ) & 15] ^ m_pWorkspaceAsBlock->l[( i + 2 ) & 15] ^ m_pWorkspaceAsBlock->l[i & 15], 1 ) );
    }



    void SHA1::_R0( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i )
    {
      z += ( ( w & ( x ^ y ) ) ^ y ) + SHABLK0( i ) + 0x5A827999 + ROL32( v, 5 );
      w = ROL32( w, 30 );
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



    void SHA1::Initialise()
    {
      // SHA1 initialization constants
      m_State[0] = 0x67452301;
      m_State[1] = 0xEFCDAB89;
      m_State[2] = 0x98BADCFE;
      m_State[3] = 0x10325476;
      m_State[4] = 0xC3D2E1F0;

      m_Count[0] = 0;
      m_Count[1] = 0;
    }



    void SHA1::Transform( GR::u32* pState, const GR::u8* pBuffer )
    {
      GR::u32         a = pState[0];
      GR::u32         b = pState[1];
      GR::u32         c = pState[2];
      GR::u32         d = pState[3];
      GR::u32         e = pState[4];

      memcpy( m_pWorkspaceAsBlock, pBuffer, 64 );

      // 4 rounds of 20 operations each. Loop unrolled.
      _R0( a, b, c, d, e, 0 ); _R0( e, a, b, c, d, 1 ); _R0( d, e, a, b, c, 2 ); _R0( c, d, e, a, b, 3 );
      _R0( b, c, d, e, a, 4 ); _R0( a, b, c, d, e, 5 ); _R0( e, a, b, c, d, 6 ); _R0( d, e, a, b, c, 7 );
      _R0( c, d, e, a, b, 8 ); _R0( b, c, d, e, a, 9 ); _R0( a, b, c, d, e, 10 ); _R0( e, a, b, c, d, 11 );
      _R0( d, e, a, b, c, 12 ); _R0( c, d, e, a, b, 13 ); _R0( b, c, d, e, a, 14 ); _R0( a, b, c, d, e, 15 );
      _R1( e, a, b, c, d, 16 ); _R1( d, e, a, b, c, 17 ); _R1( c, d, e, a, b, 18 ); _R1( b, c, d, e, a, 19 );
      _R2( a, b, c, d, e, 20 ); _R2( e, a, b, c, d, 21 ); _R2( d, e, a, b, c, 22 ); _R2( c, d, e, a, b, 23 );
      _R2( b, c, d, e, a, 24 ); _R2( a, b, c, d, e, 25 ); _R2( e, a, b, c, d, 26 ); _R2( d, e, a, b, c, 27 );
      _R2( c, d, e, a, b, 28 ); _R2( b, c, d, e, a, 29 ); _R2( a, b, c, d, e, 30 ); _R2( e, a, b, c, d, 31 );
      _R2( d, e, a, b, c, 32 ); _R2( c, d, e, a, b, 33 ); _R2( b, c, d, e, a, 34 ); _R2( a, b, c, d, e, 35 );
      _R2( e, a, b, c, d, 36 ); _R2( d, e, a, b, c, 37 ); _R2( c, d, e, a, b, 38 ); _R2( b, c, d, e, a, 39 );
      _R3( a, b, c, d, e, 40 ); _R3( e, a, b, c, d, 41 ); _R3( d, e, a, b, c, 42 ); _R3( c, d, e, a, b, 43 );
      _R3( b, c, d, e, a, 44 ); _R3( a, b, c, d, e, 45 ); _R3( e, a, b, c, d, 46 ); _R3( d, e, a, b, c, 47 );
      _R3( c, d, e, a, b, 48 ); _R3( b, c, d, e, a, 49 ); _R3( a, b, c, d, e, 50 ); _R3( e, a, b, c, d, 51 );
      _R3( d, e, a, b, c, 52 ); _R3( c, d, e, a, b, 53 ); _R3( b, c, d, e, a, 54 ); _R3( a, b, c, d, e, 55 );
      _R3( e, a, b, c, d, 56 ); _R3( d, e, a, b, c, 57 ); _R3( c, d, e, a, b, 58 ); _R3( b, c, d, e, a, 59 );
      _R4( a, b, c, d, e, 60 ); _R4( e, a, b, c, d, 61 ); _R4( d, e, a, b, c, 62 ); _R4( c, d, e, a, b, 63 );
      _R4( b, c, d, e, a, 64 ); _R4( a, b, c, d, e, 65 ); _R4( e, a, b, c, d, 66 ); _R4( d, e, a, b, c, 67 );
      _R4( c, d, e, a, b, 68 ); _R4( b, c, d, e, a, 69 ); _R4( a, b, c, d, e, 70 ); _R4( e, a, b, c, d, 71 );
      _R4( d, e, a, b, c, 72 ); _R4( c, d, e, a, b, 73 ); _R4( b, c, d, e, a, 74 ); _R4( a, b, c, d, e, 75 );
      _R4( e, a, b, c, d, 76 ); _R4( d, e, a, b, c, 77 ); _R4( c, d, e, a, b, 78 ); _R4( b, c, d, e, a, 79 );

      // Add the working vars back into state
      pState[0] += a;
      pState[1] += b;
      pState[2] += c;
      pState[3] += d;
      pState[4] += e;

      // Wipe variables
      a = b = c = d = e = 0;
    }



    bool SHA1::Update( const ByteBuffer& Data, GR::u32 NumBytes )
    {
      if ( NumBytes == 0 )
      {
        NumBytes = (GR::u32)Data.Size();
      }


      GR::u32 j = ( ( m_Count[0] >> 3 ) & 0x3F );

      if ( ( m_Count[0] += ( NumBytes << 3 ) ) < ( NumBytes << 3 ) )
      {
        // Overflow
        ++m_Count[1];
      }

      m_Count[1] += ( NumBytes >> 29 );

      GR::u32 i = 0;

      if ( ( j + NumBytes ) > 63 )
      {
        i = 64 - j;
        memcpy( &m_Buffer[j], Data.Data(), i );
        Transform( m_State, m_Buffer );

        for ( ; ( i + 63 ) < NumBytes; i += 64 )
        {
          Transform( m_State, (const GR::u8*)Data.DataAt( i ) );
        }
        j = 0;
      }

      if ( ( NumBytes - i ) != 0 )
      {
        memcpy( &m_Buffer[j], Data.DataAt( i ), NumBytes - i );
      }
      return true;
    }



    ByteBuffer SHA1::Finalize()
    {
      GR::u32     i;

      ByteBuffer  finalCount( 8 );

      for ( i = 0; i < 8; ++i )
      {
        // Endian independent
        finalCount.SetByteAt( i, (GR::u8)( ( m_Count[( ( i >= 4 ) ? 0 : 1 )] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 ) );
      }

      ByteBuffer  dummy1( 1 );
      ByteBuffer  dummy2( 1 );
      dummy1.SetByteAt( 0, '\200' );

      Update( dummy1, 1 );

      while ( ( m_Count[0] & 504 ) != 448 )
      {
        Update( dummy2, 1 );
      }

      Update( finalCount, 8 );

      for ( i = 0; i < 20; ++i )
      {
        m_Digest[i] = (GR::u8)( ( m_State[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 0xFF );
      }

      // Wipe variables for security reasons
      memset( m_Buffer, 0, 64 );
      memset( m_State, 0, 20 );
      memset( m_Count, 0, 8 );
      memset( finalCount.Data(), 0, 8 );
      Transform( m_State, m_Buffer );

      return Hash();
    }



    ByteBuffer SHA1::Hash() const
    {
      ByteBuffer    hash( 20 );
      memcpy( hash.Data(), m_Digest, 20 );

      return hash;
    }



    ByteBuffer SHA1::Calculate( const ByteBuffer& Data )
    {
      SHA1    Instance;

      Instance.Initialise();
      Instance.Update( Data );
      return Instance.Finalize();
    }



    bool SHA1::Calculate( const ByteBuffer& Data, ByteBuffer& Hash )
    {
      SHA1    Instance;

      Instance.Initialise();
      Instance.Update( Data );
      Hash = Instance.Finalize();
      return true;
    }



    bool SHA1::CalcHashKeyed( const ByteBuffer& Data, const ByteBuffer& Key, ByteBuffer& Hash )
    {
      GR::u32     blockSize = 64;

      // if key is longer than 64 bytes reset it to key=MD5(key)
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

      ::memcpy( innerPad.Data(), keyToUse.Data(), keyToUse.Size() );
      ::memcpy( outerPad.Data(), keyToUse.Data(), keyToUse.Size() );

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



    int SHA1::HashSize() const
    {
      return 20;
    }


  }

}
