#pragma once

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>

#include <Interface/ICipher.h>



//
// File : rijndael.h
// Creation date : Sun Nov 5 2000 03:21:05 CEST
// Author : Szymon Stefanek (stefanek@tin.it)
//
// Another implementation of the Rijndael cipher.
// This is intended to be an easily usable library file.
// This code is public domain.
// Based on the Vincent Rijmen and K.U.Leuven implementation 2.4.
//

//
// Original Copyright notice:
//
//    rijndael-alg-fst.c   v2.4   April '2000
//    rijndael-alg-fst.h
//    rijndael-api-fst.c
//    rijndael-api-fst.h
//
//    Optimised ANSI C code
//
//    authors: v1.0: Antoon Bosselaers
//             v2.0: Vincent Rijmen, K.U.Leuven
//             v2.3: Paulo Barreto
//             v2.4: Vincent Rijmen, K.U.Leuven
//
//    This code is placed in the public domain.
//

//
// This implementation works on 128 , 192 , 256 bit keys
// and on 128 bit blocks
//


class ByteBuffer;



namespace GR
{
  namespace Codec
  {
    class Rijndael : public GR::Codec::ICipher
    {
      private:

        const static int MAX_KEY_COLUMNS = ( 256 / 32 );
        const static int MAX_ROUNDS = 14;
        const static int MAX_IV_SIZE = 16;

      public:

        enum class Mode
        {
          ECB,
          CBC,
          CFB1
        };

        enum Result
        {
          RES_SUCCESS = 0,
          RES_UNSUPPORTED_MODE = -1,
          RES_UNSUPPORTED_DIRECTION = -2,
          RES_UNSUPPORTED_KEY_LENGTH = -3,
          RES_BAD_KEY = -4,
          RES_NOT_INITIALIZED = -5,
          RES_BAD_DIRECTION = -6,
          RES_CORRUPTED_DATA = -7,
        };



        Rijndael();



      protected:

        bool                    m_StateValid;
        Mode                    m_Mode;
        bool                    m_Encrypt;
        GR::u8                  m_InitVector[MAX_IV_SIZE];
        GR::u32                 m_NumRounds;
        GR::u8                  m_ExpandedKey[MAX_ROUNDS + 1][4][4];
        ByteBuffer              m_CachedInputData;


      public:


        virtual ByteBuffer      Encrypt( const ByteBuffer& Key, const ByteBuffer& InitialisationVector, const ByteBuffer& Data );
        virtual ByteBuffer      Decrypt( const ByteBuffer& Key, const ByteBuffer& InitialisationVector, const ByteBuffer& Data );

        virtual bool            Initialise( bool Encrypt, const ByteBuffer& Key, const ByteBuffer& InitialisationVector );
        bool                    Initialise( Mode Mode, bool Encrypt, const ByteBuffer& Key, const ByteBuffer& InitialisationVector );

        virtual bool            TransformBlock( const ByteBuffer& Data, ByteBuffer& Target, GR::u32 NumOfBytes = 0 );

        virtual ByteBuffer      TransformFinalBlock( const ByteBuffer& Data );


        static bool             EncryptCBC( const ByteBuffer& In, const ByteBuffer& Key, ByteBuffer& Encrypted, const ByteBuffer& IV = ByteBuffer() );
        static bool             DecryptCBC( const ByteBuffer& In, const ByteBuffer& Key, ByteBuffer& Decrypted, const ByteBuffer& IV = ByteBuffer() );

        ByteBuffer              EncryptECB( const ByteBuffer& Key, const ByteBuffer& InitialisationVector, const ByteBuffer& Data );
        ByteBuffer              DecryptECB( const ByteBuffer& Key, const ByteBuffer& InitialisationVector, const ByteBuffer& Data );



        // init(): Initializes the crypt session
        // Returns RIJNDAEL_SUCCESS or an error code
        // mode      : Rijndael::ECB, Rijndael::CBC or Rijndael::CFB1
        //             You have to use the same mode for encrypting and decrypting
        // key       : array of unsigned octets , it can be 16 , 24 or 32 bytes long
        //             this CAN be binary data (it is not expected to be null terminated)
        // keyLen    : Rijndael::Key16Bytes , Rijndael::Key24Bytes or Rijndael::Key32Bytes
        // initVector: initialization vector, you will usually use 0 here

        int init( Mode mode, bool Encrypt, const GR::u8* key, int keyLen, GR::u8* initVector = 0 );

        // Encrypts the input array (can be binary data)
        // The input array length must be a multiple of 16 bytes, the remaining part
        // is DISCARDED.
        // so it actually encrypts inputLen / 128 blocks of input and puts it in outBuffer
        // Input len is in BITS!
        // outBuffer must be at least inputLen / 8 bytes long.
        // Returns the encrypted buffer length in BITS or an error code < 0 in case of error
        int BlockEncrypt( const GR::u8* input, int inputLen, GR::u8* outBuffer );


        // Encrypts the input array (can be binary data)
        // The input array can be any length , it is automatically padded on a 16 byte boundary.
        // Input len is in BYTES!
        // outBuffer must be at least (inputLen + 16) bytes long
        // Returns the encrypted buffer length in BYTES or an error code < 0 in case of error
        int EncryptPadded( const GR::u8* input, int inputOctets, GR::u8* outBuffer );



        // Decrypts the input vector
        // Input len is in BITS!
        // outBuffer must be at least inputLen / 8 bytes long
        // Returns the decrypted buffer length in BITS and an error code < 0 in case of error
        int BlockDecrypt( const GR::u8* input, int inputLen, GR::u8* outBuffer );


        // Decrypts the input vector
        // Input len is in byte!
        // outBuffer must be at least inputLen bytes long
        // Returns the decrypted buffer length in BYTES and an error code < 0 in case of error
        int padDecrypt( const GR::u8* input, int inputOctets, GR::u8* outBuffer );


      protected:

        void keySched( GR::u8 key[MAX_KEY_COLUMNS][4] );
        void keyEncToDec();
        void encrypt( const GR::u8 a[16], GR::u8 b[16] );
        void decrypt( const GR::u8 a[16], GR::u8 b[16] );

    };

  }


}
