#pragma once


#include <GR/GRTypes.h>

#include <Math/BigInteger.h>
#include <Math/SecureRandom.h>

#include <Codec/SHA256.h>
#include <Codec/Rijndael.h>



namespace GR
{
  namespace Codec
  {
    class RSAKey
    {
      public:

        bool            Public;
                        
        BigInteger      ModulusN;           // n
        BigInteger      PublicExponentE;    // e
        BigInteger      PrivateExponentD;   // d

        BigInteger      PFactor;            // p factor of N
        BigInteger      QFactor;            // q factor of N
        BigInteger      qP;                 // 1/q mod p CRT
        BigInteger      dP;                 // d mod (p - 1) CRT
        BigInteger      dQ;                 // d mod (q - 1) CRT

        int             KeySizeBits;

        math::SecureRandom    Random;

        GR::Codec::SHA256     Hash;
        GR::Codec::Rijndael   Cipher;



        RSAKey();


        bool            CryptRaw( const ByteBuffer& Message, bool WithPublicKey, BigInteger& Result );
        bool            CryptRaw( const BigInteger& Message, bool WithPublicKey, BigInteger& Result );

        bool            CryptPKCSV15( const BigInteger& Message, bool WithPublicKey, BigInteger& Result );


        GR::String      ToPEMASN1();

        bool            GenerateKeyPair( int NumBits, const BigInteger& E );
        bool            GenerateRandomPrime( int NumBytes, BigInteger& Result );

        // TODO - verify!
        ByteBuffer      Sign( const ByteBuffer& Message );

    };



  }

}