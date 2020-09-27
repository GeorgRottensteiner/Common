#ifndef PALETTE_H
#define PALETTE_H



#include <GR/GRTypes.h>

#include <Interface/IIOStream.h>



#ifndef HANDLE
typedef void* HANDLE;
#endif

namespace GR
{
  namespace Graphic
  {

    class Palette
    {

      protected:

        unsigned char*      m_pData;

        size_t              m_Entries;


      public:


        Palette();
        Palette( size_t Entries );
        Palette( const Palette& palette );
        Palette& operator=( const Palette& rhs );

        ~Palette();

        static Palette&     AlphaPalette( size_t Entries = 256 );

        bool                Create( size_t Entries = 256 );

        bool                Release();

        bool                Load( const GR::Char* FileName );

        bool                Load( IIOStream& ioIn );

        bool                LoadAt( IIOStream& ioIn );

        bool                LoadPAL( const GR::Char* FileName );

        bool                LoadPALAt( IIOStream& ioIn );

        bool                Save( const GR::Char* FileName );

        bool                SaveAt( IIOStream& ioIn );

        unsigned char*      Data();

        size_t              Entries() const;

        bool                SetColor( size_t Index, 
                                      unsigned char Red,
                                      unsigned char Green, 
                                      unsigned char Blue );

        unsigned char       Red( size_t Index ) const;

        unsigned char       Green( size_t Index ) const;

        unsigned char       Blue( size_t Index ) const;

        size_t              FindNearestIndex( unsigned long Color );

        bool                IsIdentic( GR::Graphic::Palette& OtherPalette ) const;

    };

  }

}


#endif // PALETTE_H



