/******************************************************************************
*
* THIS SOURCE CODE IS HEREBY PLACED INTO THE PUBLIC DOMAIN FOR THE GOOD OF ALL
*
* This is a simple and straightforward implementation of AES-GCM authenticated
* encryption. The focus of this work was correctness & accuracy. It is written
* in straight 'C' without any particular focus upon optimization or speed. It
* should be endian (memory byte order) neutral since the few places that care
* are handled explicitly.
*
* This implementation of AES-GCM was created by Steven M. Gibson of GRC.com.
*
* It is intended for general purpose use, but was written in support of GRC's
* reference implementation of the SQRL (Secure Quick Reliable Login) client.
*
* See:    http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf
*         http://csrc.nist.gov/groups/ST/toolkit/BCM/documents/proposedmodes/
*         gcm/gcm-revised-spec.pdf
*
* NO COPYRIGHT IS CLAIMED IN THIS WORK, HOWEVER, NEITHER IS ANY WARRANTY MADE
* REGARDING ITS FITNESS FOR ANY PARTICULAR PURPOSE. USE IT AT YOUR OWN RISK.
*
*******************************************************************************/

#include "GCM.h"

/******************************************************************************
 *                      ==== IMPLEMENTATION WARNING ====
 *
 *  This code was developed for use within SQRL's fixed environmnent. Thus, it
 *  is somewhat less "general purpose" than it would be if it were designed as
 *  a general purpose AES-GCM library. Specifically, it bothers with almost NO
 *  error checking on parameter limits, buffer bounds, etc. It assumes that it
 *  is being invoked by its author or by someone who understands the values it
 *  expects to receive. Its behavior will be undefined otherwise.
 *
 *  All functions that might fail are defined to return 'ints' to indicate a
 *  problem. Most do not do so now. But this allows for error propagation out
 *  of internal functions if robust error checking should ever be desired.
 *
 ******************************************************************************/


namespace GR
{
  namespace Codec
  {

    /* Calculating the "GHASH"
     *
     * There are many ways of calculating the so-called GHASH in software, each with
     * a traditional size vs performance tradeoff.  The GHASH (Galois field hash) is
     * an intriguing construction which takes two 128-bit strings (also the cipher's
     * block size and the fundamental operation size for the system) and hashes them
     * into a third 128-bit result.
     *
     * Many implementation solutions have been worked out that use large precomputed
     * table lookups in place of more time consuming bit fiddling, and this approach
     * can be scaled easily upward or downward as needed to change the time/space
     * tradeoff. It's been studied extensively and there's a solid body of theory and
     * practice.  For example, without using any lookup tables an implementation
     * might obtain 119 cycles per byte throughput, whereas using a simple, though
     * large, key-specific 64 kbyte 8-bit lookup table the performance jumps to 13
     * cycles per byte.
     *
     * And Intel's processors have, since 2010, included an instruction which does
     * the entire 128x128->128 bit job in just several 64x64->128 bit pieces.
     *
     * Since SQRL is interactive, and only processing a few 128-bit blocks, I've
     * settled upon a relatively slower but appealing small-table compromise which
     * folds a bunch of not only time consuming but also bit twiddling into a simple
     * 16-entry table which is attributed to Victor Shoup's 1996 work while at
     * Bellcore: "On Fast and Provably Secure MessageAuthentication Based on
     * Universal Hashing."  See: http://www.shoup.net/papers/macs.pdf
     * See, also section 4.1 of the "gcm-revised-spec" cited above.
     */

    /*
     *  This 16-entry table of pre-computed constants is used by the
     *  GHASH multiplier to improve over a strictly table-free but
     *  significantly slower 128x128 bit multiple within GF(2^128).
     */
    static const uint64_t last4[16] = {
        0x0000, 0x1c20, 0x3840, 0x2460, 0x7080, 0x6ca0, 0x48c0, 0x54e0,
        0xe100, 0xfd20, 0xd940, 0xc560, 0x9180, 0x8da0, 0xa9c0, 0xb5e0 };


    GCM::GCM() :
      m_Encrypting( true )
    {
    }



    /*
     * Platform Endianness Neutralizing Load and Store Macro definitions
     * GCM wants platform-neutral Big Endian (BE) byte ordering
     */
#define GET_UINT32_BE(n,b,i) {                      \
        (n) = ( (uint32_t) (b)[(i)    ] << 24 )         \
            | ( (uint32_t) (b)[(i) + 1] << 16 )         \
            | ( (uint32_t) (b)[(i) + 2] <<  8 )         \
            | ( (uint32_t) (b)[(i) + 3]       ); }

#define PUT_UINT32_BE(n,b,i) {                      \
        (b)[(i)    ] = (GR::u8) ( (n) >> 24 );   \
        (b)[(i) + 1] = (GR::u8) ( (n) >> 16 );   \
        (b)[(i) + 2] = (GR::u8) ( (n) >>  8 );   \
        (b)[(i) + 3] = (GR::u8) ( (n)       ); }



     /******************************************************************************
     *
     *  GCM_MULT
     *
     *  Performs a GHASH operation on the 128-bit input vector 'x', setting
     *  the 128-bit output vector to 'x' times H using our precomputed tables.
     *  'x' and 'output' are seen as elements of GCM's GF(2^128) Galois field.
     *
     ******************************************************************************/
    void GCM::gcm_mult( const GR::u8 x[16],    // pointer to 128-bit input vector
                        GR::u8 output[16] )    // pointer to 128-bit output vector
    {
      int i;
      GR::u8 lo, hi, rem;
      uint64_t zh, zl;

      lo = (GR::u8)( x[15] & 0x0f );
      hi = (GR::u8)( x[15] >> 4 );
      zh = m_Context.HH[lo];
      zl = m_Context.HL[lo];

      for ( i = 15; i >= 0; i-- )
      {
        lo = (GR::u8)( x[i] & 0x0f );
        hi = (GR::u8)( x[i] >> 4 );

        if ( i != 15 )
        {
          rem = (GR::u8)( zl & 0x0f );
          zl = ( zh << 60 ) | ( zl >> 4 );
          zh = ( zh >> 4 );
          zh ^= (uint64_t)last4[rem] << 48;
          zh ^= m_Context.HH[lo];
          zl ^= m_Context.HL[lo];
        }
        rem = (GR::u8)( zl & 0x0f );
        zl = ( zh << 60 ) | ( zl >> 4 );
        zh = ( zh >> 4 );
        zh ^= (uint64_t)last4[rem] << 48;
        zh ^= m_Context.HH[hi];
        zl ^= m_Context.HL[hi];
      }
      PUT_UINT32_BE( zh >> 32, output, 0 );
      PUT_UINT32_BE( zh, output, 4 );
      PUT_UINT32_BE( zl >> 32, output, 8 );
      PUT_UINT32_BE( zl, output, 12 );
    }


    /******************************************************************************
     *
     *  GCM_SETKEY
     *
     *  This is called to set the AES-GCM key. It initializes the AES key
     *  and populates the gcm context's pre-calculated HTables.
     *
     ******************************************************************************/
    int GCM::gcm_setkey( const GR::u8* key,   // pointer to the AES encryption key
                         const GR::u32 keysize ) // size in bytes (must be 16, 24, 32 for
                            // 128, 192 or 256-bit keys respectively)
    {
      int i, j;
      uint64_t hi, lo;
      uint64_t vl, vh;
      unsigned char h[16];

      m_Context.mode = 0;
      m_Context.len = 0;
      m_Context.add_len = 0;
      memset( m_Context.HL, 0, sizeof( m_Context.HL ) );
      memset( m_Context.HH, 0, sizeof( m_Context.HH ) );
      memset( m_Context.base_ectr, 0, sizeof( m_Context.base_ectr ) );
      memset( m_Context.y, 0, sizeof( m_Context.y ) );
      memset( m_Context.buf, 0, sizeof( m_Context.buf ) );

      memset( h, 0, 16 );                     // initialize the block to encrypt

      // encrypt the null 128-bit block to generate a key-based value
      // which is then used to initialize our GHASH lookup tables

      if ( !m_Context.aes_ctx.Initialise( true, ByteBuffer( key, keysize ), ByteBuffer() ) )
      {
        return 1;
      }

      ByteBuffer    data( h, sizeof( h ) );

      if ( !m_Context.aes_ctx.TransformBlock( data, data ) )
      {
        return 1;
      }
      memcpy( h, data.Data(), sizeof( h ) );

      GET_UINT32_BE( hi, h, 0 );    // pack h as two 64-bit ints, big-endian
      GET_UINT32_BE( lo, h, 4 );
      vh = (uint64_t)hi << 32 | lo;

      GET_UINT32_BE( hi, h, 8 );
      GET_UINT32_BE( lo, h, 12 );
      vl = (uint64_t)hi << 32 | lo;

      m_Context.HL[8] = vl;                // 8 = 1000 corresponds to 1 in GF(2^128)
      m_Context.HH[8] = vh;
      m_Context.HH[0] = 0;                 // 0 corresponds to 0 in GF(2^128)
      m_Context.HL[0] = 0;

      for ( i = 4; i > 0; i >>= 1 )
      {
        uint32_t T = (uint32_t)( vl & 1 ) * 0xe1000000U;
        vl = ( vh << 63 ) | ( vl >> 1 );
        vh = ( vh >> 1 ) ^ ( (uint64_t)T << 32 );
        m_Context.HL[i] = vl;
        m_Context.HH[i] = vh;
      }
      for ( i = 2; i < 16; i <<= 1 )
      {
        uint64_t* HiL = m_Context.HL + i, * HiH = m_Context.HH + i;
        vh = *HiH;
        vl = *HiL;
        for ( j = 1; j < i; j++ )
        {
          HiH[j] = vh ^ m_Context.HH[j];
          HiL[j] = vl ^ m_Context.HL[j];
        }
      }
      return( 0 );
    }


    /******************************************************************************
     *
     *    GCM processing occurs four phases: SETKEY, START, UPDATE and FINISH.
     *
     *  SETKEY:
     *
     *   START: Sets the Encryption/Decryption mode.
     *          Accepts the initialization vector and additional data.
     *
     *  UPDATE: Encrypts or decrypts the plaintext or ciphertext.
     *
     *  FINISH: Performs a final GHASH to generate the authentication tag.
     *
     ******************************************************************************
     *
     *  GCM_START
     *
     *  Given a user-provided GCM context, this initializes it, sets the encryption
     *  mode, and preprocesses the initialization vector and additional AEAD data.
     *
     ******************************************************************************/
    int GCM::gcm_start( 
                  int mode,            // GCM_ENCRYPT or GCM_DECRYPT
                   const GR::u8* iv,     // pointer to initialization vector
                   size_t iv_len,       // IV length in bytes (should == 12)
                   const GR::u8* add,    // ptr to additional AEAD data (NULL if none)
                   size_t add_len )     // length of additional AEAD data (bytes)
    {
      GR::u8 work_buf[16]; // XOR source built from provided IV if len != 16
      const GR::u8* p;     // general purpose array pointer
      size_t use_len;     // byte count to process, up to 16 bytes
      size_t i;           // local loop iterator

      // since the context might be reused under the same key
      // we zero the working buffers for this next new process
      memset( m_Context.y, 0x00, sizeof( m_Context.y ) );
      memset( m_Context.buf, 0x00, sizeof( m_Context.buf ) );
      m_Context.len = 0;
      m_Context.add_len = 0;

      m_Context.mode = mode;               // set the GCM encryption/decryption mode
      //m_Context.aes_ctx.mode = ENCRYPT;    // GCM *always* runs AES in ENCRYPTION mode

      if ( iv_len == 12 )
      {                // GCM natively uses a 12-byte, 96-bit IV
        memcpy( m_Context.y, iv, iv_len );   // copy the IV to the top of the 'y' buff
        m_Context.y[15] = 1;                 // start "counting" from 1 (not 0)
      }
      else    // if we don't have a 12-byte IV, we GHASH whatever we've been given
      {
        memset( work_buf, 0x00, 16 );               // clear the working buffer
        PUT_UINT32_BE( iv_len * 8, work_buf, 12 );  // place the IV into buffer

        p = iv;
        while ( iv_len > 0 )
        {
          use_len = ( iv_len < 16 ) ? iv_len : 16;
          for ( i = 0; i < use_len; i++ ) m_Context.y[i] ^= p[i];
          gcm_mult( m_Context.y, m_Context.y );
          iv_len -= use_len;
          p += use_len;
        }
        for ( i = 0; i < 16; i++ ) m_Context.y[i] ^= work_buf[i];
        gcm_mult( m_Context.y, m_Context.y );
      }

      ByteBuffer  data( m_Context.y, 16 );
      ByteBuffer  out( 16 );
      if ( !m_Context.aes_ctx.TransformBlock( data, out ) )
      {
        return 1;
      }
      memcpy( m_Context.base_ectr, out.Data(), sizeof( m_Context.base_ectr ) );

      m_Context.add_len = add_len;
      p = add;
      while ( add_len > 0 )
      {
        use_len = ( add_len < 16 ) ? add_len : 16;
        for ( i = 0; i < use_len; i++ ) m_Context.buf[i] ^= p[i];
        gcm_mult( m_Context.buf, m_Context.buf );
        add_len -= use_len;
        p += use_len;
      }
      return( 0 );
    }

    /******************************************************************************
     *
     *  GCM_UPDATE
     *
     *  This is called once or more to process bulk plaintext or ciphertext data.
     *  We give this some number of bytes of input and it returns the same number
     *  of output bytes. If called multiple times (which is fine) all but the final
     *  invocation MUST be called with length mod 16 == 0. (Only the final call can
     *  have a partial block length of < 128 bits.)
     *
     ******************************************************************************/
    int GCM::gcm_update( size_t length,          // length, in bytes, of data to process
                    const GR::u8* input,     // pointer to source data
                    GR::u8* output )         // pointer to destination data
    {
      GR::u8 ectr[16];     // counter-mode cipher output for XORing
      size_t use_len;     // byte count to process, up to 16 bytes
      size_t i;           // local loop iterator

      m_Context.len += length; // bump the GCM context's running length count

      while ( length > 0 )
      {
    // clamp the length to process at 16 bytes
        use_len = ( length < 16 ) ? length : 16;

        // increment the context's 128-bit IV||Counter 'y' vector
        for ( i = 16; i > 12; i-- ) if ( ++m_Context.y[i - 1] != 0 ) break;

        // encrypt the context's 'y' vector under the established key
        ByteBuffer  data( m_Context.y, 16 );
        ByteBuffer  out( 16 );
        if ( !m_Context.aes_ctx.TransformBlock( data, out ) )
        {
          return 1;
        }
        memcpy( ectr, out.Data(), sizeof( ectr ) );

      // encrypt or decrypt the input to the output
        if ( m_Context.mode == GCM_ENCRYPT )
        {
          for ( i = 0; i < use_len; i++ )
          {
    // XOR the cipher's ouptut vector (ectr) with our input
            output[i] = (GR::u8)( ectr[i] ^ input[i] );
            // now we mix in our data into the authentication hash.
            // if we're ENcrypting we XOR in the post-XOR (output) 
            // results, but if we're DEcrypting we XOR in the input 
            // data
            m_Context.buf[i] ^= output[i];
          }
        }
        else
        {
          for ( i = 0; i < use_len; i++ )
          {
    // but if we're DEcrypting we XOR in the input data first, 
    // i.e. before saving to ouput data, otherwise if the input 
    // and output buffer are the same (inplace decryption) we 
    // would not get the correct auth tag

            m_Context.buf[i] ^= input[i];

            // XOR the cipher's ouptut vector (ectr) with our input
            output[i] = (GR::u8)( ectr[i] ^ input[i] );
          }
        }
        gcm_mult( m_Context.buf, m_Context.buf );    // perform a GHASH operation

        length -= use_len;  // drop the remaining byte count to process
        input += use_len;  // bump our input pointer forward
        output += use_len;  // bump our output pointer forward
      }
      return( 0 );
    }

    /******************************************************************************
     *
     *  GCM_FINISH
     *
     *  This is called once after all calls to GCM_UPDATE to finalize the GCM.
     *  It performs the final GHASH to produce the resulting authentication TAG.
     *
     ******************************************************************************/
    int GCM::gcm_finish( GR::u8* tag,         // pointer to buffer which receives the tag
                    size_t tag_len )    // length, in bytes, of the tag-receiving buf
    {
      GR::u8 work_buf[16];
      uint64_t orig_len = m_Context.len * 8;
      uint64_t orig_add_len = m_Context.add_len * 8;
      size_t i;

      if ( tag_len != 0 ) memcpy( tag, m_Context.base_ectr, tag_len );

      if ( orig_len || orig_add_len )
      {
        memset( work_buf, 0x00, 16 );

        PUT_UINT32_BE( ( orig_add_len >> 32 ), work_buf, 0 );
        PUT_UINT32_BE( ( orig_add_len ), work_buf, 4 );
        PUT_UINT32_BE( ( orig_len >> 32 ), work_buf, 8 );
        PUT_UINT32_BE( ( orig_len ), work_buf, 12 );

        for ( i = 0; i < 16; i++ ) m_Context.buf[i] ^= work_buf[i];
        gcm_mult( m_Context.buf, m_Context.buf );
        for ( i = 0; i < tag_len; i++ ) tag[i] ^= m_Context.buf[i];
      }
      return( 0 );
    }


    /******************************************************************************
     *
     *  GCM_CRYPT_AND_TAG
     *
     *  This either encrypts or decrypts the user-provided data and, either
     *  way, generates an authentication tag of the requested length. It must be
     *  called with a GCM context whose key has already been set with GCM_SETKEY.
     *
     *  The user would typically call this explicitly to ENCRYPT a buffer of data
     *  and optional associated data, and produce its an authentication tag.
     *
     *  To reverse the process the user would typically call the companion
     *  GCM_AUTH_DECRYPT function to decrypt data and verify a user-provided
     *  authentication tag.  The GCM_AUTH_DECRYPT function calls this function
     *  to perform its decryption and tag generation, which it then compares.
     *
     ******************************************************************************/
    int GCM::gcm_crypt_and_tag(
      int mode,               // cipher direction: GCM_ENCRYPT or GCM_DECRYPT
      const GR::u8* iv,        // pointer to the 12-byte initialization vector
      size_t iv_len,          // byte length if the IV. should always be 12
      const GR::u8* add,       // pointer to the non-ciphered additional data
      size_t add_len,         // byte length of the additional AEAD data
      const GR::u8* input,     // pointer to the cipher data source
      GR::u8* output,          // pointer to the cipher data destination
      size_t length,          // byte length of the cipher data
      GR::u8* tag,             // pointer to the tag to be generated
      size_t tag_len )        // byte length of the tag to be generated
    {   /*
           assuming that the caller has already invoked gcm_setkey to
           prepare the gcm context with the keying material, we simply
           invoke each of the three GCM sub-functions in turn...
        */
      gcm_start( mode, iv, iv_len, add, add_len );
      gcm_update( length, input, output );
      gcm_finish( tag, tag_len );
      return( 0 );
    }


    /******************************************************************************
     *
     *  GCM_AUTH_DECRYPT
     *
     *  This DECRYPTS a user-provided data buffer with optional associated data.
     *  It then verifies a user-supplied authentication tag against the tag just
     *  re-created during decryption to verify that the data has not been altered.
     *
     *  This function calls GCM_CRYPT_AND_TAG (above) to perform the decryption
     *  and authentication tag generation.
     *
     ******************************************************************************/
    int GCM::gcm_auth_decrypt(
      const GR::u8* iv,        // pointer to the 12-byte initialization vector
      size_t iv_len,          // byte length if the IV. should always be 12
      const GR::u8* add,       // pointer to the non-ciphered additional data
      size_t add_len,         // byte length of the additional AEAD data
      const GR::u8* input,     // pointer to the cipher data source
      GR::u8* output,          // pointer to the cipher data destination
      size_t length,          // byte length of the cipher data
      const GR::u8* tag,       // pointer to the tag to be authenticated
      size_t tag_len )        // byte length of the tag <= 16
    {
      GR::u8 check_tag[16];        // the tag generated and returned by decryption
      int diff;                   // an ORed flag to detect authentication errors
      size_t i;                   // our local iterator
      /*
         we use GCM_DECRYPT_AND_TAG (above) to perform our decryption
         (which is an identical XORing to reverse the previous one)
         and also to re-generate the matching authentication tag
      */
      gcm_crypt_and_tag( GCM_DECRYPT, iv, iv_len, add, add_len,
                         input, output, length, check_tag, tag_len );

                     // now we verify the authentication tag in 'constant time'
      for ( diff = 0, i = 0; i < tag_len; i++ )
        diff |= tag[i] ^ check_tag[i];

      if ( diff != 0 )
      {                   // see whether any bits differed?
        memset( output, 0, length );    // if so... wipe the output data
        return( GCM_AUTH_FAILURE );     // return GCM_AUTH_FAILURE
      }
      return( 0 );
    }

    /******************************************************************************
     *
     *  GCM_ZERO_CTX
     *
     *  The GCM context contains both the GCM context and the AES context.
     *  This includes keying and key-related material which is security-
     *  sensitive, so it MUST be zeroed after use. This function does that.
     *
     ******************************************************************************/
    void GCM::gcm_zero_ctx()
    {
        // zero the context originally provided to us
      //memset( ctx, 0, sizeof( gcm_context ) );
      m_Context.mode = 0;
      m_Context.len = 0;
      m_Context.add_len = 0;
      memset( m_Context.HL, 0, sizeof( m_Context.HL ) );
      memset( m_Context.HH, 0, sizeof( m_Context.HH ) );
      memset( m_Context.base_ectr, 0, sizeof( m_Context.base_ectr ) );
      memset( m_Context.y, 0, sizeof( m_Context.y ) );
      memset( m_Context.buf, 0, sizeof( m_Context.buf ) );
    }



    int GCM::aes_gcm_encrypt( unsigned char* output, const unsigned char* input, int input_length, const unsigned char* key, const size_t key_len, const unsigned char* iv, const size_t iv_len )
    {

      int ret = 0;                // our return value

      GCM     gcm;

      size_t tag_len = 0;
      unsigned char* tag_buf = NULL;

      gcm.gcm_setkey( key, (const GR::u32)key_len );

      ret = gcm.gcm_crypt_and_tag( GCM_ENCRYPT, iv, iv_len, NULL, 0,
                               input, output, input_length, tag_buf, tag_len );

      gcm.gcm_zero_ctx();

      return( ret );
    }

    int GCM::aes_gcm_decrypt( unsigned char* output, const unsigned char* input, int input_length, const unsigned char* key, const size_t key_len, const unsigned char* iv, const size_t iv_len )
    {

      int ret = 0;                // our return value
      GCM   gcm;

      size_t tag_len = 0;
      unsigned char* tag_buf = NULL;

      gcm.gcm_setkey( key, (const GR::u32)key_len );

      ret = gcm.gcm_crypt_and_tag( GCM_DECRYPT, iv, iv_len, NULL, 0,
                               input, output, input_length, tag_buf, tag_len );

      gcm.gcm_zero_ctx();

      return( ret );

    }



    int GCM::BlockSize() const
    {
      // inherited from Rijndael/AES
      return 16;
    }



        // single call to calculate
    bool GCM::Encrypt( const ByteBuffer& Key,
                       const ByteBuffer& InitialisationVector,
                       const ByteBuffer& Data,
                       const ByteBuffer& AuthenticationData,
                       int AuthenticatedTagLength,
                       ByteBuffer& EncryptedData,
                       ByteBuffer& AuthenticatedTag )
    {
      GCM   gcm;

      EncryptedData.Resize( Data.Size() );
      AuthenticatedTag.Resize( AuthenticatedTagLength );

      if ( ( gcm.gcm_setkey( (GR::u8*)Key.Data(), (const GR::u32)Key.Size() ) )
      ||   ( gcm.gcm_start( GCM_ENCRYPT,
                            (const GR::u8*)InitialisationVector.Data(), InitialisationVector.Size(),
                            (const GR::u8*)AuthenticationData.Data(), AuthenticationData.Size() ) )
      ||   ( gcm.gcm_update( Data.Size(), (const GR::u8*)Data.Data(), (GR::u8*)EncryptedData.Data() ) )
      ||   ( gcm.gcm_finish( (GR::u8*)AuthenticatedTag.Data(), AuthenticatedTag.Size() ) ) )
      {
        gcm.gcm_zero_ctx();
        return false;
      }

      gcm.gcm_zero_ctx();
      return true;
    }



    bool GCM::Decrypt( const ByteBuffer& Key,
                       const ByteBuffer& InitialisationVector,
                       const ByteBuffer& Data,
                       const ByteBuffer& AuthenticationData,
                       int AuthenticatedTagLength,
                       ByteBuffer& DecryptedData,
                       ByteBuffer& AuthenticatedTag )
    {
      GCM   gcm;

      DecryptedData.Resize( Data.Size() );
      AuthenticatedTag.Resize( AuthenticatedTagLength );

      if ( ( gcm.gcm_setkey( (GR::u8*)Key.Data(), (const GR::u32)Key.Size() ) )
      ||   ( gcm.gcm_start( GCM_DECRYPT,
                            (const GR::u8*)InitialisationVector.Data(), InitialisationVector.Size(), 
                            (const GR::u8*)AuthenticationData.Data(), AuthenticationData.Size() ) )
      ||   ( gcm.gcm_update( Data.Size(), (const GR::u8*)Data.Data(), (GR::u8*)DecryptedData.Data() ) )
      ||   ( gcm.gcm_finish( (GR::u8*)AuthenticatedTag.Data(), AuthenticatedTag.Size() ) ) )
      {
        gcm.gcm_zero_ctx();
        return false;
      }

      return true;
    }



    bool GCM::Initialise( bool Encrypt, const ByteBuffer& Key )
    {
      m_Encrypting = Encrypt;
      if ( gcm_setkey( (GR::u8*)Key.Data(), (const GR::u32)Key.Size() ) )
      {
        return false;
      }
      return true;
    }



    bool GCM::Start( const ByteBuffer& InitialisationVector, const ByteBuffer& AuthenticationData )
    {
      if ( gcm_start( m_Encrypting ? GCM_ENCRYPT : GCM_DECRYPT,
                      (const GR::u8*)InitialisationVector.Data(), InitialisationVector.Size(),
                      (const GR::u8*)AuthenticationData.Data(), AuthenticationData.Size() ) )
      {
        return false;
      }
      return true;
    }



    bool GCM::TransformBlock( const ByteBuffer& Data, ByteBuffer& Target )
    {
      Target.Resize( Data.Size() );
      if ( gcm_update( Data.Size(), (const GR::u8*)Data.Data(), (GR::u8*)Target.Data() ) )
      {
        return false;
      }
      return true;
    }



    bool GCM::TransformFinalBlock( ByteBuffer& AuthenticationTag )
    {
      if ( gcm_finish( (GR::u8*)AuthenticationTag.Data(), AuthenticationTag.Size() ) )
      {
        gcm_zero_ctx();
        return false;
      }
      gcm_zero_ctx();

      return true;
    }



  }

}