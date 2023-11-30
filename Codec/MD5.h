// MD5.CC - source code for the C++/object oriented translation and
//          modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug

// This translation/ modification is provided "as is," without express or
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think
// it does; (2) that this translation/ modification is accurate; or (3) that
// this software is "merchantible."  (Language for this disclaimer partially
// copied from the disclaimer below).

/* based on:

   MD5.H - header file for MD5C.C
   MDDRIVER.C - test driver for MD2, MD4 and MD5

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#include <stdio.h>
#include <GR/GRTypes.h>

#include <Interface/IHash.h>



class ByteBuffer;

namespace GR
{
  namespace Codec
  {

    class MD5 : public GR::Codec::IHash
    {

      public:


        MD5();

        virtual ByteBuffer    Hash() const;

        static bool           Calculate( const ByteBuffer& Data, ByteBuffer& Hash );
        static bool           CalcKeyedHash( const ByteBuffer& Data,
                                              const ByteBuffer& Key,
                                              ByteBuffer& Hash );

        virtual ByteBuffer    Calculate( const ByteBuffer& Data );

        virtual void          Initialise();
        virtual bool          Update( const ByteBuffer& Data, GR::u32 NumOfBytes = 0 );
        virtual ByteBuffer    Finalize();

        virtual int           HashSize() const;


      private:

        const GR::u32         S11 = 7;
        const GR::u32         S12 = 12;
        const GR::u32         S13 = 17;
        const GR::u32         S14 = 22;
        const GR::u32         S21 = 5;
        const GR::u32         S22 = 9;
        const GR::u32         S23 = 14;
        const GR::u32         S24 = 20;
        const GR::u32         S31 = 4;
        const GR::u32         S32 = 11;
        const GR::u32         S33 = 16;
        const GR::u32         S34 = 23;
        const GR::u32         S41 = 6;
        const GR::u32         S42 = 10;
        const GR::u32         S43 = 15;
        const GR::u32         S44 = 21;



        GR::u32               m_State[4];
        GR::u32               m_CountOfBits[2];   // mod 2^64
        GR::u8                m_InputBuffer[64];
        GR::u8                m_Digest[16];
        bool                  m_Finalized;



        void                  TransformBlock( GR::u8 buffer[64] );

        static void           encode( GR::u8* dest, GR::u32* src, GR::u32 length );
        static void           decode( GR::u32* dest, GR::u8* src, GR::u32 length );
        static void           memcpy( GR::u8* dest, const GR::u8* src, GR::u32 length );
        static void           memset( GR::u8* start, GR::u8 val, GR::u32 length );



        static inline GR::u32 RotateLeft( GR::u32 x, GR::u32 n );
        static inline GR::u32 F( GR::u32 x, GR::u32 y, GR::u32 z );
        static inline GR::u32 G( GR::u32 x, GR::u32 y, GR::u32 z );
        static inline GR::u32 H( GR::u32 x, GR::u32 y, GR::u32 z );
        static inline GR::u32 I( GR::u32 x, GR::u32 y, GR::u32 z );
        static inline void    FF( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac );
        static inline void    GG( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac );
        static inline void    HH( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac );
        static inline void    II( GR::u32& a, GR::u32 b, GR::u32 c, GR::u32 d, GR::u32 x, GR::u32 s, GR::u32 ac );

    };

  }

}
