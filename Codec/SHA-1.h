#pragma once

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>

#include <OS/OS.h>

#include <Interface/IHash.h>



namespace GR
{

  namespace Codec
  {

    class SHA1 : public IHash
    {

      public:

        SHA1();
        ~SHA1();



        virtual ByteBuffer      Calculate( const ByteBuffer& Data );
        static bool             Calculate( const ByteBuffer& Data, ByteBuffer& Hash );

        virtual void            Initialise();
        virtual bool            Update( const ByteBuffer& Data, GR::u32 NumOfBytes = 0 );
        virtual ByteBuffer      Finalize();

        static bool             CalcHashKeyed( const ByteBuffer& Data, const ByteBuffer& Key, ByteBuffer& Hash );

        virtual ByteBuffer      Hash() const;

        virtual int							HashSize() const;



      private:

        union SHA1_WORKSPACE_BLOCK
        {
          GR::u8  c[64];
          GR::u32 l[16];
        };

        GR::u8                  m_Workspace[64];

        GR::u32                 m_State[5];
        GR::u32                 m_Count[2];
        GR::u8                  m_Buffer[64];
        GR::u8                  m_Digest[20];

        SHA1_WORKSPACE_BLOCK*   m_pWorkspaceAsBlock;


        void                    Transform( GR::u32* pState, const GR::u8* pBuffer );


        void                    _R0( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i );
        void                    _R1( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i );
        void                    _R2( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i );
        void                    _R3( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i );
        void                    _R4( GR::u32 v, GR::u32& w, GR::u32 x, GR::u32 y, GR::u32& z, int i );

        GR::u32                 ROL32( GR::u32 _val32, int _nBits );
        GR::u32                 SHABLK0( int i );
        GR::u32                 SHABLK( int i );

    };

  }

}
