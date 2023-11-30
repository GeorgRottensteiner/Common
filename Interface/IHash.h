#pragma once

#include <string>
#include <map>

#include <GR/GRTypes.h>



class ByteBuffer;


namespace GR
{
  namespace Codec
  {
    class IHash
    {
      public:

        virtual ~IHash()
        {
        }


        // single call to calculate
        virtual ByteBuffer  Calculate( const ByteBuffer& Data ) = 0;

        // reset state
        virtual void        Initialise() = 0;
        // update with new data
        virtual bool        Update( const ByteBuffer& Data, GR::u32 NumOfBytes = 0 ) = 0;
        // final call
        virtual ByteBuffer  Finalize() = 0;

        // final call
        virtual ByteBuffer  Hash() const = 0;


        virtual int         HashSize() const = 0;

    };

  }
}


