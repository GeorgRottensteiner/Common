#pragma once

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
*         http://csrc.nist.gov/groups/ST/toolkit/BCM/documents/proposedmodes/ \
*         gcm/gcm-revised-spec.pdf
*
* NO COPYRIGHT IS CLAIMED IN THIS WORK, HOWEVER, NEITHER IS ANY WARRANTY MADE
* REGARDING ITS FITNESS FOR ANY PARTICULAR PURPOSE. USE IT AT YOUR OWN RISK.
*
*******************************************************************************/


#define GCM_AUTH_FAILURE    0x55555555  // authentication failure

#include "Rijndael.h"

#include <Interface/IAuthenticatedCipher.h>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Codec
  {

    class GCM : public GR::Codec::IAuthenticatedCipher
    {
      private:

        bool                m_Encrypting;

#define GCM_ENCRYPT         1       // specify whether we're encrypting
#define GCM_DECRYPT         0       // or decrypting

        struct gcm_context
        {
          int                   mode;               // cipher direction: encrypt/decrypt
          GR::u64               len;           // cipher data length processed so far
          GR::u64               add_len;       // total add data length
          GR::u64               HL[16];        // precalculated lo-half HTable
          GR::u64               HH[16];        // precalculated hi-half HTable
          GR::u8                base_ectr[16];    // first counter-mode cipher output for tag
          GR::u8                y[16];            // the current cipher-input IV|Counter value
          GR::u8                buf[16];          // buf working value
          GR::Codec::Rijndael   aes_ctx;    // cipher context used
        };

        gcm_context         m_Context;



        void gcm_mult( const GR::u8 x[16],    // pointer to 128-bit input vector
                       GR::u8 output[16] );    // pointer to 128-bit output vector


        /******************************************************************************
         *  GCM_SETKEY : sets the GCM (and AES) keying material for use
         ******************************************************************************/
        int gcm_setkey( const GR::u8* key,   // pointer to cipher key
                        const GR::u32 keysize  // size in bytes (must be 16, 24, 32 for
                                // 128, 192 or 256-bit keys respectively)
        ); // returns 0 for success


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
        int gcm_crypt_and_tag(
          int mode,               // cipher direction: ENCRYPT (1) or DECRYPT (0)
          const GR::u8* iv,        // pointer to the 12-byte initialization vector
          size_t iv_len,          // byte length if the IV. should always be 12
          const GR::u8* add,       // pointer to the non-ciphered additional data
          size_t add_len,         // byte length of the additional AEAD data
          const GR::u8* input,     // pointer to the cipher data source
          GR::u8* output,          // pointer to the cipher data destination
          size_t length,          // byte length of the cipher data
          GR::u8* tag,             // pointer to the tag to be generated
          size_t tag_len );       // byte length of the tag to be generated


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
        int gcm_auth_decrypt(
          const GR::u8* iv,        // pointer to the 12-byte initialization vector
          size_t iv_len,          // byte length if the IV. should always be 12
          const GR::u8* add,       // pointer to the non-ciphered additional data
          size_t add_len,         // byte length of the additional AEAD data
          const GR::u8* input,     // pointer to the cipher data source
          GR::u8* output,          // pointer to the cipher data destination
          size_t length,          // byte length of the cipher data
          const GR::u8* tag,       // pointer to the tag to be authenticated
          size_t tag_len );       // byte length of the tag <= 16


        /******************************************************************************
         *
         *  GCM_START
         *
         *  Given a user-provided GCM context, this initializes it, sets the encryption
         *  mode, and preprocesses the initialization vector and additional AEAD data.
         *
         ******************************************************************************/
        int gcm_start( int mode,            // ENCRYPT (1) or DECRYPT (0)
                       const GR::u8* iv,     // pointer to initialization vector
                       size_t iv_len,       // IV length in bytes (should == 12)
                       const GR::u8* add,    // pointer to additional AEAD data (NULL if none)
                       size_t add_len );    // length of additional AEAD data (bytes)


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
        int gcm_update( size_t length,          // length, in bytes, of data to process
                        const GR::u8* input,     // pointer to source data
                        GR::u8* output );        // pointer to destination data


                      /******************************************************************************
                       *
                       *  GCM_FINISH
                       *
                       *  This is called once after all calls to GCM_UPDATE to finalize the GCM.
                       *  It performs the final GHASH to produce the resulting authentication TAG.
                       *
                       ******************************************************************************/
        int gcm_finish( GR::u8* tag,         // ptr to tag buffer - NULL if tag_len = 0
                        size_t tag_len );   // length, in bytes, of the tag-receiving buf


                      /******************************************************************************
                       *
                       *  GCM_ZERO_CTX
                       *
                       *  The GCM context contains both the GCM context and the AES context.
                       *  This includes keying and key-related material which is security-
                       *  sensitive, so it MUST be zeroed after use. This function does that.
                       *
                       ******************************************************************************/
        void gcm_zero_ctx();


        int aes_gcm_encrypt( unsigned char* output, const unsigned char* input, int input_length, const unsigned char* key, const size_t key_len, const unsigned char* iv, const size_t iv_len );
        int aes_gcm_decrypt( unsigned char* output, const unsigned char* input, int input_length, const unsigned char* key, const size_t key_len, const unsigned char* iv, const size_t iv_len );


      public:

        GCM();



        virtual int         BlockSize() const;

        // single call to calculate
        virtual bool        Encrypt( const ByteBuffer& Key,
                                     const ByteBuffer& InitialisationVector,
                                     const ByteBuffer& Data,
                                     const ByteBuffer& AuthenticationData,
                                     int AuthenticatedTagLength,
                                     ByteBuffer& EncryptedData,
                                     ByteBuffer& AuthenticatedTag );
        virtual bool        Decrypt( const ByteBuffer& Key,
                                     const ByteBuffer& InitialisationVector,
                                     const ByteBuffer& Data,
                                     const ByteBuffer& AuthenticationData,
                                     int AuthenticatedTagLength,
                                     ByteBuffer& DecryptedData,
                                     ByteBuffer& AuthenticatedTag );

        // reset state
        virtual bool        Initialise( bool Encrypt, const ByteBuffer& Key );

        virtual bool        Start( const ByteBuffer& InitialisationVector, const ByteBuffer& AuthenticationData );

        // update with new data
        virtual bool        TransformBlock( const ByteBuffer& Data,
                                            ByteBuffer& Target );

        // final call
        virtual bool        TransformFinalBlock( ByteBuffer& AuthenticationTag );

    };



  }
}

