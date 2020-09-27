#ifndef INCLUDE_BLOWFISH_H
#define INCLUDE_BLOWFISH_H

#include <Memory/ByteBuffer.h>



namespace Codec
{
  class BlowFish
  {

    public:

      static bool           Encode( const ByteBuffer& bbSource, const ByteBuffer& bbKey, ByteBuffer& bbTarget );
      static bool           Decode( const ByteBuffer& bbSource, const ByteBuffer& bbKey, ByteBuffer& bbTarget );


    private:

      #define BF_ROUNDS 16

      struct BF_KEY
      {
        GR::u32 P[BF_ROUNDS+2];
        GR::u32 S[4*256];
      };


      static BF_KEY                      bf_key;
      static BF_KEY                      bf_init;

      static GR::u32                     bf_enc( GR::u32 ll, GR::u32 r, GR::u32 n );
      static std::pair<GR::u32,GR::u32>  bf_encrypt( GR::u32 l, GR::u32 r );
      static std::pair<GR::u32,GR::u32>  bf_decrypt( GR::u32 l, GR::u32 r );
      static void                        bf_set_key( const ByteBuffer& bbKey );

  };
}

#endif // INCLUDE_BLOWFISH_H
