#include "PKCS.h"

#include <debug/debugclient.h>



namespace GR
{
  namespace Codec
  {
    namespace PKCS
    {

      bool V1_5_Encode( const ByteBuffer& Message, BlockType Block, GR::u32 ModulusBitLength, ByteBuffer& Result )
      {
        GR::u32     modulusByteLength = ( ModulusBitLength + 7 ) >> 3;

        if ( Message.Size() + 11 > modulusByteLength )
        {
          dh::Log( "V1_5_Encode - Message size + 11 larger than modulus" );
          return false;
        }
        Result.Resize( modulusByteLength );

        int     psLength = modulusByteLength - (int)Message.Size() - 3;

        if ( Block == BlockType::PKCS_1_EME )
        {
            // TODO - need pseudo rnd, values must be <> 0!
          for ( int i = 0; i < psLength; ++i )
          {
            Result.SetByteAt( 2 + i, (GR::u8)i );
          }
        }
        else
        {
          // BlockType::PKCS_1_EMSA
          for ( int i = 0; i < psLength; ++i )
          {
            Result.SetByteAt( 2 + i, 0xff );
          }
        }

        Result.SetByteAt( 1, (GR::u8)Block );
        memcpy( Result.DataAt( 2 + psLength + 1 ), Message.Data(), Message.Size() );

        return true;
      }



    }
  }
}