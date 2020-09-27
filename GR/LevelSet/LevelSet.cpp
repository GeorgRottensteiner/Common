#include <GR\LevelSet\LevelSet.h>

#include <IO/FileStream.h>
#include <Misc/Misc.h>



namespace GR
{
  namespace Game
  {
    LevelSet::LevelSet()
    {

    }



    bool PlayerSolveSet::Save( const GR::String& FileName )
    {
      GR::IO::FileStream    File;

      if ( !File.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      File.WriteU16( ( GR::u16 )m_Players.size() );

      itMapPlayers  itPlayer( m_Players.begin() );

      while ( itPlayer != m_Players.end() )
      {
        PlayerSolve& solve = itPlayer->second;
        File.WriteLine( solve.Name );

        File.WriteU16( ( GR::u16 )solve.Solved.size() );

        std::set<GR::String>::const_iterator  itSolved( solve.Solved.begin() );
        while ( itSolved != solve.Solved.end() )
        {
          File.WriteLine( *itSolved );

          itSolved++;
        }

        itPlayer++;
      }
      File.Close();

      return true;
    }



    bool PlayerSolveSet::Load( const GR::String& FileName )
    {
      GR::IO::FileStream    File;

      if ( !File.Open( FileName ) )
      {
        return false;
      }

      Clear();

      GR::u16    numPlayers = File.ReadU16();

      for ( GR::u16 i = 0; i < numPlayers; i++ )
      {
        GR::String     line;

        File.ReadLine( line );

        GR::u16 solved = File.ReadU16();

        PlayerSolve* pSolve = AddPlayer( line );

        for ( GR::u16 j = 0; j < solved; j++ )
        {
          File.ReadLine( line );
          pSolve->Solved.insert( line );
        }
      }
      File.Close();

      return true;
    }

  }
}