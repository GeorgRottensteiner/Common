#pragma once

#include <Memory/ByteBuffer.h>

#include <Interface/IHash.h>
#include <Interface/ICipher.h>



namespace math
{

  class SecureRandom
  {

    private:

      // max size of either a cipher/hash block or symmetric key [largest of the two] */
      const int MAX_BLOCK_SIZE = 144;

      ByteBuffer          m_Pool;

      bool                m_CipherInitialised;

      GR::Codec::ICipher& m_Cipher;
      GR::Codec::IHash&   m_Hash;



    public:

      SecureRandom( GR::Codec::IHash& Hash, GR::Codec::ICipher& Cipher );


      void        AddEntropy( const ByteBuffer& Data );
      bool        GetRandom( ByteBuffer& Target, int NumBytes );

  };



}

