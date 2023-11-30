#include "RSAKey.h"

#include <debug/debugclient.h>

#include <Codec/Base64.h>
#include <Codec/ASN1Format.h>



namespace GR
{
  namespace Codec
  {
    RSAKey::RSAKey() :
      KeySizeBits( 0 ),
      Public( true ),
      Random( Hash, Cipher )
    { 
    }



    bool RSAKey::CryptRaw( const ByteBuffer& Message, bool WithPublicKey, BigInteger& Result )
    {
      return CryptRaw( BigInteger( Message ), WithPublicKey, Result );
    }



    bool RSAKey::CryptRaw( const BigInteger& Message, bool WithPublicKey, BigInteger& Result )
    {
      if ( Message > ModulusN )
      {
        dh::Log( "RSA::CryptRaw - Message is bigger than modulus!" );
        return false;
      }
      Result = Message.ModuloExponentiation( WithPublicKey ? PublicExponentE : PrivateExponentD, ModulusN );
      return true;
    }



    ByteBuffer RSAKey::Sign( const ByteBuffer& Message )
    {
      // TODO - is correct?
      BigInteger    message( Message );
      BigInteger    ourSignature = message.ModuloExponentiation( PrivateExponentD, ModulusN );

      return ourSignature.ToBuffer();
    }



    bool RSAKey::CryptPKCSV15( const BigInteger& Message, bool WithPublicKey, BigInteger& Result )
    {
      // x = 0x00 || 0x02 || r || 0x00 || m
      // The length of random value (r) is k-Len-3 bytes, and where k is the number of bytes in modulus (N),
      // and Len is the number of bytes in the message

      int numBytesModulus = ( (int)ModulusN.BitCount() + 7 ) / 8;
      ByteBuffer    message = Message.ToBuffer();

      /*
      if ( (int)message.Size() > numBytesModulus - 3 )
      {
        dh::Log( "RSA::CryptPKCSV15 - Message is too big!" );
        return false;
      }*/

      int     randomLength = numBytesModulus - (int)message.Size() - 3;
      if ( randomLength < 8 )
      {
        randomLength = 8;
      }

      ByteBuffer    paddedMessage( 3 + (size_t)randomLength + message.Size() );

      paddedMessage.SetU16NetworkOrderAt( 0, 0x0002 );
      // TODO - fill with secure random!
      memcpy( paddedMessage.DataAt( paddedMessage.Size() - message.Size() ), message.Data(), message.Size() );

      return CryptRaw( paddedMessage, WithPublicKey, Result );
    }



    GR::String RSAKey::ToPEMASN1()
    {
      BigInteger exponent = BigInteger( ByteBuffer( "010001" ) );
      auto buffer = exponent.ToBuffer();

      GR::String    result = "-----BEGIN PUBLIC KEY-----\n";

      ByteBuffer    sequenceMain;

      ByteBuffer    sequEncryption;
      ByteBuffer    sequPublicKey;
      ByteBuffer    publicKeyBitString;

      ByteBuffer    oidEncryption = ASN1Format::EncodeObjectIdentifer( "2a864886f70d010101" );
      ByteBuffer    tagNull = ASN1Format::EncodeNull();
      sequEncryption = ASN1Format::EncodeObject( ASN1Format::ASNType::ASN1_TYPE_SEQUENCE, ASN1Format::TagClass::UNIVERSAL, oidEncryption + tagNull );

      ByteBuffer    tagModulus        = ASN1Format::EncodeInteger( ModulusN );
      ByteBuffer    tagPublicExponent = ASN1Format::EncodeInteger( PublicExponentE );
      sequPublicKey  = ASN1Format::EncodeObject( ASN1Format::ASNType::ASN1_TYPE_SEQUENCE, ASN1Format::TagClass::UNIVERSAL, tagModulus + tagPublicExponent );

      publicKeyBitString = ASN1Format::EncodeBitString( sequPublicKey );

      sequenceMain = ASN1Format::EncodeObject( ASN1Format::ASNType::ASN1_TYPE_SEQUENCE, ASN1Format::TagClass::UNIVERSAL, sequEncryption + publicKeyBitString );

      printf( "Data is %s\n", sequenceMain.ToHexString().c_str() );

      return result;
    }



    bool RSAKey::GenerateRandomPrime( int NumBytes, BigInteger& Result )
    {
      /*
      int            type;

      // 0 + (BlockType) + PS + 0 + Msg
      ByteBuffer    em( 3 + ps.Size() + msg.Size() );
      // LTC_PKCS_1_EMSA  = 1,   Block type 1 (PKCS #1 v1.5 signature padding)
      // LTC_PKCS_1_EME   = 2       Block type 2 (PKCS #1 v1.5 encryption padding)

      // get type (what is this for?) - padding/encryption?
      if ( NumBytes < 0 )
      {
        type = USE_BBS;
        NumBytes = -NumBytes;
      }
      else
      {
        type = 0;
      }*/

      // allow sizes between 2 and 512 bytes for a prime size
      if ( ( NumBytes < 2 )
      ||   ( NumBytes > 512 ) )
      {
        return false;
      }

      ByteBuffer    randomData( NumBytes );


      do
      {
        // generate value
        if ( !Random.GetRandom( randomData, NumBytes ) )
        {
          return false;
        }

        printf( "Gen prime rand %s\n", randomData.ToHexString().c_str() );

        // make sure the top most and least bit are set to keep the key size intact
        randomData[0] |= 0x80 | 0x40;
        //randomData[NumBytes - 1] |= 0x01 | ( ( type & USE_BBS ) ? 0x02 : 0x00 );
        randomData[NumBytes - 1] |= 0x01;

        Result = BigInteger( randomData );
      }
      while ( !Result.IsPrime() );

      printf( "Gen prime %s\n", Result.ToString().c_str() );

      return true;
    }



    bool RSAKey::GenerateKeyPair( int NumBits, const BigInteger& E )
    {
      // Choose two large prime numbers p and q.
      //  To make factoring harder, p and q should be chosen at random, be both large and have a large difference.
      //   [1] For choosing them the standard method is to choose random integers and use a primality test until two primes are found.
      // p and q should be kept secret.

      PublicExponentE = E;

      // make prime p
      BigInteger  p;
      BigInteger  tmp1;
      BigInteger  tmp2;

      // for RSA any prime will not do, it must have special relation with the exponent
      // It's for use with RSA, check the RSA condition that gcd( p - 1, exp ) == 1
      // Since exp is a small prime we can do this efficiently by checking that ( p - 1 ) mod exp != 0
      int numBitsP = ( NumBits + 1 ) / 2;
      int numBitsQ = NumBits - numBitsP;

      do
      {
        if ( !GenerateRandomPrime( numBitsP, p ) )
        {
          return false;
        }
        tmp1 = p - BigInteger::BigInteger1;
        tmp2 = BigInteger::GreatestCommonDivisor( E, tmp1 );

        // while e divides p-1
      }
      while ( tmp2 != BigInteger::BigInteger1 );

      // make prime q
      BigInteger  q;
      do
      {
        if ( !GenerateRandomPrime( numBitsQ, q ) )
        {
          return false;
        }

        tmp1 = q - BigInteger::BigInteger1;
        tmp2 = BigInteger::GreatestCommonDivisor( tmp1, E );
      }
      while ( tmp2 != BigInteger::BigInteger1 );


      // tmp1 = lcm(p-1, q-1)

      // tmp2 = p-1
      tmp2 = p - BigInteger::BigInteger1;

      // tmp1 = q-1 (previous do/while loop)
      // tmp1 = lcm(p-1, q-1)
      tmp1 = BigInteger::LowestCommonMultiple( tmp1, tmp2 );


      PublicExponentE = E;

      // Make sure that p > q, which is required for the CRT decrypt
      if ( p < q )
      {
        auto temp = q;
        q = p;
        p = temp;
      }

      //d = 1 / e mod lcm( p - 1, q - 1 );
      auto pMinusOne = p - BigInteger::BigInteger1;
      auto qMinusOne = q - BigInteger::BigInteger1;
      auto tmp = pMinusOne * qMinusOne;

      if ( !E.ModInverse( tmp, PrivateExponentD ) )
      {
        dh::Log( "ModInverse failed!" );
        return false;
      }

      // Compute e1 = d mod (p - 1), e2 = d mod (q - 1)
      dP = PrivateExponentD % pMinusOne;
      dQ = PrivateExponentD % qMinusOne;

      // Compute n = pq, u = qInv mod p
      ModulusN = p * q;

      BigInteger rsaU;
      
      if ( !q.ModInverse( p, rsaU ) )
      {
        dh::Log( "ModInverse failed 2!" );
        return false;
      }



      return true;
    }


  }
}