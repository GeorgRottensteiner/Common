#pragma once

#include <string>
#include <map>

#include <Memory/ByteBuffer.h>



namespace GR
{
  namespace Codec
  {
    class ICipher
    {
      public:

        virtual ~ICipher()
        {
        }

        // single call to calculate
        virtual ByteBuffer  Encrypt( const ByteBuffer& Key, const ByteBuffer& InitialisationVector, const ByteBuffer& Data ) = 0;
        virtual ByteBuffer  Decrypt( const ByteBuffer& Key, const ByteBuffer& InitialisationVector, const ByteBuffer& Data ) = 0;

        // reset state
        virtual bool        Initialise( bool Encrypt, const ByteBuffer& Key, const ByteBuffer& InitialisationVector ) = 0;

        // update with new data
        virtual bool        TransformBlock( const ByteBuffer& Data, ByteBuffer& Target, GR::u32 NumOfBytes = 0 ) = 0;

        // final call
        virtual ByteBuffer  TransformFinalBlock( const ByteBuffer& Data ) = 0;

        virtual int         BlockSize() const = 0;

    };

  }
}


