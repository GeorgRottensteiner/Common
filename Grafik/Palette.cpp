#include <Grafik/Palette.h>

#include <Misc/Misc.h>

#include <IO/FileStream.h>

#include <debug/debugclient.h>



namespace GR
{

  namespace Graphic
  {

    Palette::Palette() :
      m_pData( NULL ),
      m_Entries( 0 )
    {
      Create( 256 );
    }



    Palette::Palette( size_t Entries ) :
      m_pData( NULL ),
      m_Entries( 0 )
    {
      if ( Entries > 0 )
      {
        Create( Entries );
      }
    }


    Palette::~Palette()
    {
      Release();
    }



    Palette::Palette( const Palette& palRHS )
      : m_pData( NULL ),
        m_Entries( 0 )
    {
      if ( &palRHS == this )
      {
        return;
      }
      Create( palRHS.m_Entries );
      if ( m_pData != NULL )
      {
        memcpy( m_pData,
                const_cast<Palette*>( &palRHS )->Data(),
                3 * palRHS.m_Entries );
      }
    }



    Palette& Palette::operator=( const Palette& rhs )
    {
      if ( this == &rhs )
      {
        return *this;;
      }


      Release();
      if ( (unsigned char *)const_cast<Palette*>(&rhs)->Data() )
      {
        Create( rhs.m_Entries );
        memcpy( m_pData,
                const_cast<Palette*>(&rhs)->Data(),
                rhs.m_Entries * 3 );
      }

      return *this;
    }



    bool Palette::Create( size_t dwEntries )
    {
      Release();
      m_Entries = dwEntries;
      m_pData = new unsigned char[m_Entries * 3];
      memset( m_pData, 0, m_Entries * 3 );

      return true;
    }



    bool Palette::Release()
    {
      if ( m_pData )
      {
        delete[] m_pData;
        m_pData = NULL;
      }
      m_Entries = 0;

      return true;
    }



    bool Palette::Load( const GR::Char* FileName )
    {
      unsigned char   fileType,
                      bpp;

      unsigned long   counter;

      GR::IO::FileStream     ioIn;


      if ( ioIn.Open( FileName ) )
      {
        fileType  = ioIn.ReadU8();
        bpp       = ioIn.ReadU8();
        counter   = ioIn.ReadU32();
        if ( ( bpp == 8 )
        &&   ( counter == 768 ) )
        {
          LoadAt( ioIn );
        }
        else
        {
          dh::Log( "Fehler: Palette.Load Wrong Paletteformat." );
        }
        ioIn.Close();
        return true;
      }
      else
      {
        dh::Log( "Fehler: Palette.Load Could'nt load %s.", FileName );
      }
      return false;
    }



    bool Palette::Load( IIOStream& ioIn )
    {
      unsigned char   fileType,
                      bpp;

      unsigned long   counter;

      if ( ioIn.IsGood() )
      {
        fileType  = ioIn.ReadU8();
        bpp       = ioIn.ReadU8();
        counter   = ioIn.ReadU32();
        if ( ( bpp == 8 )
        &&   ( counter == 768 ) )
        {
          LoadAt( ioIn );
        }
        else
        {
          dh::Log( "Fehler: Palette.Load Wrong Paletteformat." );
        }
        return true;
      }
      return false;
    }



    bool Palette::LoadAt( IIOStream& ioIn )
    {
      if ( ioIn.IsGood() )
      {
        Release();
        Create();
        ioIn.ReadBlock( m_pData, 768 );
        return true;
      }
      else
      {
        dh::Log( "Fehler: Palette.LoadAt Could'nt load Palette." );
      }
      return false;
    }



    bool Palette::LoadPAL( const GR::Char* FileName )
    {
      bool                  result     = false;
      GR::IO::FileStream    ioIn;

      if ( ioIn.Open( FileName ) )
      {
        result = LoadPALAt( ioIn );
        ioIn.Close();
        return result;
      }
      else
      {
        dh::Log( "Fehler: Palette.LoadPAL Could'nt load %s.", FileName );
      }
      return false;
    }



    bool Palette::LoadPALAt( IIOStream& ioIn )
    {
      if ( ioIn.IsGood() )
      {
        Release();
        Create();
        ioIn.ReadBlock( m_pData, 768 );
        return true;
      }
      else
      {
        dh::Log( "Fehler: Palette.LoadPALAt Could'nt load Palette." );
      }
      return false;
    }



    bool Palette::Save( const GR::Char* FileName )
    {
      unsigned char   fileType      = 4,//IGF_TYPE_PALETTE,
                      bpp           = 8;

      unsigned long   counter       = 768;

      GR::IO::FileStream  ioOut;

      if ( ioOut.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        ioOut.WriteU8( fileType );
        ioOut.WriteU8( bpp );
        ioOut.WriteU32( counter );
        SaveAt( ioOut );
        ioOut.Close();
        return true;
      }
      else
      {
        dh::Log( "Fehler: Palette.Save Could'nt save %s.", FileName );
      }

      return false;
    }



    bool Palette::SaveAt( IIOStream& ioOut )
    {
      if ( ioOut.IsGood() )
      {
        ioOut.WriteBlock( m_pData, ( GR::u32 )( m_Entries * 3 ) );
        return true;
      }
      else
      {
        dh::Log( "Fehler: Palette.SaveAt Could'nt save Palette." );
      }
      return false;
    }



    unsigned char* Palette::Data()
    {
      return m_pData;
    }



    bool Palette::SetColor( size_t Index, unsigned char Red,
                            unsigned char Green, unsigned char Blue )
    {
      if ( Index >= m_Entries )
      {
        dh::Log( "Palette::SetColor Index out of bounds (%d>%d)", Index, m_Entries );
        return false;
      }
      if ( m_pData != NULL )
      {
        m_pData[3 * Index]      = Red;
        m_pData[3 * Index + 1]  = Green;
        m_pData[3 * Index + 2]  = Blue;
        return true;
      }
      else
      {
        dh::Log( "Fehler: Palette.SetColor Keine Palette da." );
      }

      return false;
    }



    unsigned char Palette::Red( size_t Index ) const
    {
      if ( Index >= m_Entries )
      {
        dh::Log( "Palette::Red Index out of bounds (%d > %d)", Index, m_Entries );
        return 0;
      }
      if ( m_pData != NULL )
      {
        return *( m_pData + 3 * Index );
      }
      else
      {
        dh::Log( "Fehler: Palette.GetRed Keine Palette da." );
      }
      return 0;
    }



    unsigned char Palette::Green( size_t Index ) const
    {
      if ( Index >= m_Entries )
      {
        dh::Log( "Palette::Green Index out of bounds (%d > %d)", Index, m_Entries );
        return 0;
      }
      if ( m_pData != NULL )
      {
        return *( m_pData + 3 * Index + 1 );
      }
      else
      {
        dh::Log( "Fehler: Palette.GetGreen Keine Palette da." );
      }
      return 0;
    }



    unsigned char Palette::Blue( size_t Index ) const
    {
      if ( Index >= m_Entries )
      {
        dh::Log( "Palette::Blue Index out of bounds (%d > %d)", Index, m_Entries );
        return 0;
      }
      if ( m_pData != NULL )
      {
        return *( m_pData + 3 * Index + 2 );
      }
      else
      {
        dh::Log( "Fehler: Palette.GetBlue Keine Palette da." );
      }
      return 0;
    }



    size_t Palette::Entries() const
    {
      return m_Entries;
    }



    Palette& Palette::AlphaPalette( size_t Entries )
    {
      static Palette   g_AlphaPalette;

      g_AlphaPalette.Create( Entries );

      for ( size_t i = 0; i < Entries; ++i )
      {
        g_AlphaPalette.SetColor( i, 
                        (unsigned char)( ( 255 * i ) / ( Entries - 1 ) ),
                        (unsigned char)( ( 255 * i ) / ( Entries - 1 ) ),
                        (unsigned char)( ( 255 * i ) / ( Entries - 1 ) ) );
      }

      return g_AlphaPalette;
    }



    size_t Palette::FindNearestIndex( unsigned long Color )
    {
      int           r = ( Color & 0xff0000 ) >> 16,
                    g = ( Color & 0xff00 ) >> 8,
                    b = ( Color & 0xff ),
                    deltaR,
                    deltaG,
                    deltaB,
                    unt;

      size_t        which_color;


      // TODO - geht besser!!

      for ( size_t i = 0; i < m_Entries; i++ )
      {
        if ( ( Red( i ) == r )
        &&   ( Green( i ) == g )
        &&   ( Blue( i ) == b ) )
        {
          // die perfekte Farbe!
          return i;
        }
      }
      which_color = 0;
      unt = 945;
      for ( size_t i = 0; i < m_Entries; i++ )
      {
        deltaR = 3 * abs( (int)( Red( i ) - r ) );
        deltaG = 3 * abs( (int)( Green( i ) - g ) );
        deltaB = 3 * abs( (int)( Blue( i ) - b ) );

        if ( deltaR + deltaG + deltaB < unt )
        {
          // näher dran an der Farbe als der alte Wert
          which_color = i;
          unt = deltaR + deltaG + deltaB;
          if ( unt == 0 )
          {
            // besser kann es nicht mehr werden
            return which_color;
          }
        }

      }
      return which_color;
    }



    bool Palette::IsIdentic( GR::Graphic::Palette& OtherPalette ) const
    {
      if ( m_Entries != OtherPalette.Entries() )
      {
        return false;
      }
      return ( memcmp( m_pData, OtherPalette.Data(), m_Entries * 3 ) == 0 );
    }


  }

}




