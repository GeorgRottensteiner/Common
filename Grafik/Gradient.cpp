#include "Gradient.h"

#include <Grafik/GFXHelper.h>

#include <debug/debugclient.h>



namespace GR
{
  namespace Graphic
  {
    Gradient::Gradient() :
      m_Length( 0.0f )
    {
    }



    Gradient::~Gradient()
    {
      Clear();
    }



    void Gradient::Clear()
    {
      m_Pegs.clear();
      m_Length = 0.0f;
    }



    size_t Gradient::PegCount() const
    {
      return m_Pegs.size();
    }



    void Gradient::SetPegColor( size_t Index, GR::u32 Color )
    {
      if ( Index >= m_Pegs.size() )
      {
        return;
      }
      m_Pegs[Index].Color = Color;
    }



    Gradient::tPeg* Gradient::GetPeg( size_t Index )
    {
      if ( Index >= m_Pegs.size() )
      {
        return NULL;
      }
      return &m_Pegs[Index];
    }



    Gradient::tPeg* Gradient::AddPeg( GR::u32 Color, float Position )
    {
      if ( m_Pegs.empty() )
      {
        Position = 0.0f;
      }
      if ( Position < 0.0f )
      {
        Position = 0.0f;
      }
      if ( Position > m_Length )
      {
        m_Length = Position;
      }

      tPegs::iterator   it( m_Pegs.begin() );
      while ( it != m_Pegs.end() )
      {
        tPeg&     Peg = *it;

        if ( ( Peg.Position >= Position )
        &&   ( Peg.Type != tPeg::PEG_START ) )
        {
          tPegs::iterator   itNew = m_Pegs.insert( it, tPeg( Color, Position ) );

          return &( *itNew );
        }

        ++it;
      }

      m_Pegs.push_back( tPeg( Color, Position ) );

      return &m_Pegs.back();
    }



    void Gradient::RemovePeg( Gradient::tPeg* pPeg )
    {
      tPegs::iterator   it( m_Pegs.begin() );
      while ( it != m_Pegs.end() )
      {
        tPeg&     Peg = *it;

        if ( &Peg == pPeg )
        {
          it = m_Pegs.erase( it );

          if ( m_Pegs.empty() )
          {
            m_Length = 0.0f;
          }
          else
          {
            m_Length = m_Pegs.back().Position;
          }
          return;
        }

        ++it;
      }
    }



    float Gradient::Length() const
    {
      return m_Length;
    }



    GR::u32 Gradient::GetColorAt( float fPosition )
    {
      if ( m_Pegs.empty() )
      {
        return 0xff000000;
      }

      if ( fPosition <= 0.0f )
      {
        const tPeg&   PegFront = m_Pegs.front();

        return PegFront.Color;
      }
      if ( fPosition >= m_Length )
      {
        const tPeg&   PegBack = m_Pegs.back();

        return PegBack.Color;
      }

      tPegs::const_iterator    it( m_Pegs.begin() );
      while ( it != m_Pegs.end() )
      {
        const tPeg&     Peg = *it;

        if ( Peg.Position == fPosition )
        {
          return Peg.Color;
        }
        ++it;
      }

      const tPeg*   pPrevPeg = GetPreviousPeg( fPosition );
      const tPeg*   pNextPeg = GetNextPeg( fPosition );

      if ( ( pPrevPeg == NULL )
      ||   ( pNextPeg == NULL ) )
      {
        dh::Log( "Gradient::GetColorAt - no Peg found (pos a)?" );
        return 0xff000000;
      }
      return GFX::ColorGradient( pPrevPeg->Color, pNextPeg->Color, (int)( ( fPosition - pPrevPeg->Position ) * 100 / ( pNextPeg->Position - pPrevPeg->Position ) ) );
    }



    Gradient::tPeg* Gradient::GetPreviousPeg( float fPos )
    {
      tPegs::reverse_iterator    it( m_Pegs.rbegin() );
      while ( it != m_Pegs.rend() )
      {
        tPeg&     Peg = *it;

        if ( Peg.Position < fPos )
        {
          return &Peg;
        }

        ++it;
      }
      return &m_Pegs.front();
    }



    Gradient::tPeg* Gradient::GetNextPeg( float fPos )
    {
      tPegs::iterator    it( m_Pegs.begin() );
      while ( it != m_Pegs.end() )
      {
        tPeg&     Peg = *it;

        if ( Peg.Position > fPos )
        {
          return &Peg;
        }

        ++it;
      }
      return &m_Pegs.back();
    }



    Gradient::tPeg* Gradient::GetPreviousPeg( Gradient::tPeg* pPeg )
    {
      size_t  index = m_Pegs.size() - 1;
      tPegs::reverse_iterator    it( m_Pegs.rbegin() );
      while ( it != m_Pegs.rend() )
      {
        tPeg&     Peg = *it;

        if ( &Peg == pPeg )
        {
          if ( index )
          {
            return &m_Pegs[index - 1];
          }
        }

        --index;
        ++it;
      }

      return &m_Pegs.front();
    }



    Gradient::tPeg* Gradient::GetNextPeg( Gradient::tPeg* pPeg )
    {
      size_t   index = 0;
      tPegs::iterator    it( m_Pegs.begin() );
      while ( it != m_Pegs.end() )
      {
        tPeg&     Peg = *it;

        if ( &Peg == pPeg )
        {
          if ( index < m_Pegs.size() )
          {
            return &m_Pegs[index + 1];
          }
        }

        ++index;
        ++it;
      }
      return &m_Pegs.back();
    }

  }

}