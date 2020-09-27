#ifndef CODEC_SHA1
#define CODEC_SHA1

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>

#include <OS/OS.h>



namespace Codec
{

  class SHA1
  {

    public:

	    SHA1();
	    ~SHA1();



      static bool             CalcHash( const ByteBuffer& Data, ByteBuffer& Hash );

      static bool             CalcHash256( const ByteBuffer& Data, ByteBuffer& Hash );
      static bool             CalcHash384( const ByteBuffer& Data, ByteBuffer& Hash );
      static bool             CalcHash512( const ByteBuffer& Data, ByteBuffer& Hash );

      
      
      static bool             CalcHashKeyed( const ByteBuffer& Data, 
                                             const ByteBuffer& Key,
                                             ByteBuffer& Hash );

      static bool             CalcHash256Keyed( const ByteBuffer& Data, 
                                                const ByteBuffer& Key,
                                                ByteBuffer& Hash );
      static bool             CalcHash384Keyed( const ByteBuffer& Data, 
                                                const ByteBuffer& Key,
                                                ByteBuffer& Hash );
      static bool             CalcHash512Keyed( const ByteBuffer& Data, 
                                                const ByteBuffer& Key,
                                                ByteBuffer& Hash );
      

    private:

      typedef union
      {
	      GR::u8  c[64];
	      GR::u32 l[16];
      } SHA1_WORKSPACE_BLOCK;

      GR::u8                  m_workspace[64];

	    GR::u32                 m_state[5];
	    GR::u32                 m_count[2];
	    GR::u32                 m_reserved0[1]; // Memory alignment padding
	    GR::u8                  m_buffer[64];
	    GR::u8                  m_digest[20];
	    GR::u32                 m_reserved1[3]; // Memory alignment padding

      // SHA1 pointer to the byte array above
      SHA1_WORKSPACE_BLOCK*   m_block; 


	    // Private SHA-1 transformation
	    void                    Transform( GR::u32* pState, const GR::u8* pBuffer );


	    void                    Reset();

	    void                    Update(const GR::u8* pbData, GR::u32 uLen);

	    // Finalize hash
	    void                    Final();

	    bool                    GetHash( GR::u8* pbDest ) const;



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
#endif // CODEC_SHA1
