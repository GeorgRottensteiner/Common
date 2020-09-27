#pragma once

#include <GR/GRTypes.h>

#include <vector>



namespace GR
{
  namespace Graphic
  {
    class Gradient
    {

      public:

        struct tPeg
        {
          enum ePegType
          {
            PEG_START,
            PEG_END,
            PEG_CUSTOM,
          };

          ePegType      Type;

          GR::u32       Color;
          float         Position;

          tPeg( GR::u32 Color = 0xffffffff, float Position = 0.0f ) :
            Type( PEG_CUSTOM ),
            Color( Color ),
            Position( Position )
          {
          }
        };

        typedef std::vector<tPeg>    tPegs;

        tPegs                     m_Pegs;

        float                     m_Length;


        Gradient();
        ~Gradient();


        void                      Clear();
        tPeg*                     AddPeg( GR::u32 Color = 0xffffffff, float Position = 0.0f );
        void                      RemovePeg( Gradient::tPeg* pPeg );

        Gradient::tPeg*           GetPreviousPeg( Gradient::tPeg* pPeg );
        Gradient::tPeg*           GetNextPeg( Gradient::tPeg* pPeg );

        Gradient::tPeg*           GetPreviousPeg( float Pos );
        Gradient::tPeg*           GetNextPeg( float Pos );

        float                     Length() const;
        GR::u32                   GetColorAt( float Position );

        size_t                    PegCount() const;
        Gradient::tPeg*           GetPeg( size_t Index );

        void                      SetPegColor( size_t Index, GR::u32 Color );

    };

  }
}
