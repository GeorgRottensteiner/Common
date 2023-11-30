#pragma once


#include <GR/GRTypes.h>



class ByteBuffer;

namespace GR
{
  namespace Codec
  {
    namespace PKCS
    {
      enum class BlockType
      {
        PKCS_1_EMSA = 1,    // Block type 1 (PKCS #1 v1.5 signature padding)
        PKCS_1_EME = 2      // Block type 2 (PKCS #1 v1.5 encryption padding)
      };

      bool V1_5_Encode( const ByteBuffer& Message, BlockType Block, GR::u32 ModulusBitLength, ByteBuffer& Result );

    }

  }

}