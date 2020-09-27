#ifndef GR_LEVELSET_H
#define GR_LEVELSET_H



#include <GR/GRTypes.h>
#include <map>
#include <list>
#include <set>

#include <Misc/Misc.h>



namespace GR
{
  namespace Game
  {
    typedef std::list<GR::String>            tListLevels;
    typedef std::list<GR::String>::iterator  itListLevels;


    class LevelSet
    {

      public:

        GR::String                    Name;

        tListLevels                   Level;



        LevelSet();



        virtual ~LevelSet()
        {
          Clear();
        }



        void Clear()
        {
          Level.clear();
        }



        void AddLevel( const GR::String& LevelName )
        {
          Level.push_back( LevelName );
        }



        void RemoveLevel( const GR::String& LevelName )
        {
          Level.remove( LevelName );
        }

    };



    class PlayerSolve
    {
      public:

      GR::String                  Name;

      std::set<GR::String>        Solved;
    };



    typedef std::map<GR::String, PlayerSolve>             tMapPlayers;
    typedef std::map<GR::String, PlayerSolve>::iterator   itMapPlayers;



    class PlayerSolveSet
    {
      public:

        LevelSet                              m_LevelSet;

        tMapPlayers                           m_Players;



        virtual ~PlayerSolveSet()
        {
          Clear();
        }



        void Clear()
        {
          m_Players.clear();
        }



        PlayerSolve* AddPlayer( const GR::String& Name )
        {
          itMapPlayers    it( m_Players.find( Name ) );
          if ( it != m_Players.end() )
          {
            // diesen Spieler gibt es schon
            return &it->second;
          }
          m_Players[Name].Name = Name;

          return &m_Players[Name];
        }



        void EmptyLevelList()
        {
          m_LevelSet.Clear();
        }



        void LevelSolved( PlayerSolve& SolveInfo, const GR::String& LevelName )
        {
          SolveInfo.Solved.insert( LevelName );
        }



        bool IsLevelSolved( const PlayerSolve& SolveInfo, const GR::String& LevelName )
        {
          return ( SolveInfo.Solved.find( LevelName ) != SolveInfo.Solved.end() );
        }



        bool AllowedToPlay( const PlayerSolve& Solve, const GR::String& LevelName )
        {
          if ( IsLevelSolved( Solve, LevelName ) )
          {
            return true;
          }
          // ist der Level der nächste auf der Liste?
          if ( !m_LevelSet.Level.empty() )
          {
            // der erste Level darf immer gespielt werden
            if ( m_LevelSet.Level.front() == LevelName )
            {
              return true;
            }

            // jetzt prüfen, ob wir einen Level davor gelöst haben
            itListLevels  itLevel( m_LevelSet.Level.begin() );
            while ( itLevel != m_LevelSet.Level.end() )
            {
              if ( !IsLevelSolved( Solve, *itLevel ) )
              {
                // dieser Level war noch nicht gelöst, wenn das jetzt nicht der angefragte ist, wars das
                if ( *itLevel == LevelName )
                {
                  return true;
                }
                return false;
              }
              itLevel++;
            }

          }
          return false;
        }



        bool Load( const GR::String& FileName );
        bool Save( const GR::String& FileName );
    };

  }

}

#endif // GR_LEVELSET_H