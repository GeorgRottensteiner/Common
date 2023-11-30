#include "Statistics.h"

#include "Timer.h"

#include <DateTime/DateTime.h>

#include <debug/debugclient.h>

#undef max



namespace GR
{
  
  namespace Statistics
  {
  
    Statistics::Statistics() :
      m_LastResetTime( 0.0 )
    {
    }



    Statistics::~Statistics()
    {
    }



    void Statistics::StartTimer( const GR::String& TimerName )
    {
      GR::f64     now = GetStartTimestamp();


      std::map<GR::String,tStatsEntry>::iterator it = m_StatInfos.find( TimerName );
      if ( it == m_StatInfos.end() )
      {
        tStatsEntry   timer;

        timer.Type      = StatsType::ST_TIMER;
        timer.LastCall  = now;
        timer.LastReset = now;
        timer.MaxTime   = now;

        m_StatInfos[TimerName] = timer;
      }
      else
      {
        tStatsEntry& timer( it->second );

        if ( timer.Type == StatsType::ST_TIMER )
        {
          timer.LastCall = now;
        }
      }
    }



    void Statistics::StopTimer( const GR::String& TimerName )
    {
      StopTimer( TimerName, GetStartTimestamp() );
    }



    void Statistics::StopTimer( const GR::String& TimerName, GR::f64 StopTime )
    {
      // Check if we already know this Timer
      std::map<GR::String,tStatsEntry>::iterator it = m_StatInfos.find( TimerName );
      if ( it != m_StatInfos.end() )
      {
        tStatsEntry&   entry( it->second );

        if ( entry.Type == StatsType::ST_TIMER )
        {
          ComputeTime( entry, entry.LastCall, StopTime );
        }
      }
    }



    void Statistics::Dump() const
    {
      std::map<GR::String,tStatsEntry>::const_iterator    it( m_StatInfos.begin() );
      while ( it != m_StatInfos.end() )
      {
        //const tStatsEntry& timeInfo( it->second );
  
        dh::Log( "Timer %s:", it->first.c_str() );
        /*
        if ( timeInfo.numTimed == 0 )
        {
          dh::Log( "-not timed yet" );
        }
        else
        {
          dh::Log( "-called %d times, avg. %.6f sec, min. %.6f sec, max. %.6f sec", timeInfo.numTimed, timeInfo.sumTimes / (GR::f32)timeInfo.numTimed, timeInfo.smallestTime, timeInfo.longestTime );
        }*/
  
        ++it;
      }
    }
  
  
    
    GR::String Statistics::ToString()
    {
      GR::String   result;
      
      std::map<GR::String,tStatsEntry>::iterator it( m_StatInfos.begin() );
      while ( it != m_StatInfos.end() )
      {
        tStatsEntry& timeInfo( it->second );
    
        GR::String     timerInfo = "Timer " + it->first + ": ";
        if ( timeInfo.NumTimed == 0 )
        {
          timerInfo += "not timed yet";
        }
        else
        {
          char      temp[500];
          
#if OPERATING_SYSTEM == OS_WINDOWS
          sprintf_s( temp, "called %llu times, avg. %.6f sec, min. %.6f sec, max. %.6f sec", timeInfo.NumTimed, timeInfo.SumTimes / (GR::f32)timeInfo.NumTimed, timeInfo.Minimum, timeInfo.Maximum );
#else
          sprintf( temp, "called %lu times, avg. %.6f sec, min. %.6f sec, max. %.6f sec", timeInfo.NumTimed, timeInfo.SumTimes / (GR::f32)timeInfo.NumTimed, timeInfo.Minimum, timeInfo.Maximum );
#endif
          timerInfo += temp;
        }
        result += timerInfo + "\n";
    
        ++it;
      }
      return result;
    }
  
    
    
    GR::f64 Statistics::ShortestTime( const GR::String& Timer )
    {
      std::map<GR::String,Statistics::tStatsEntry>::const_iterator    it( m_StatInfos.find( Timer ) );
      if ( it == m_StatInfos.end() )
      {
        return 0;
      }
      return it->second.Minimum;
    }
    
    
    
    GR::f64 Statistics::LongestTime( const GR::String& Timer )
    {
      std::map<GR::String,Statistics::tStatsEntry>::const_iterator    it( m_StatInfos.find( Timer ) );
      if ( it == m_StatInfos.end() )
      {
        return 0;
      }
      return it->second.Maximum;
    }
    
    
    
    GR::f64 Statistics::SumTimes( const GR::String& Timer )
    {
      std::map<GR::String,Statistics::tStatsEntry>::const_iterator    it( m_StatInfos.find( Timer ) );
      if ( it == m_StatInfos.end() )
      {
        return 0.0;
      }
      return it->second.SumTimes;
    }
    
    
    
    GR::u64 Statistics::NumTimed( const GR::String& Timer )
    {
      std::map<GR::String,Statistics::tStatsEntry>::const_iterator    it( m_StatInfos.find( Timer ) );
      if ( it == m_StatInfos.end() )
      {
        return 0;
      }
      return it->second.NumTimed;
    }
    


    size_t Statistics::GetNumOfEntries()
    {
      return m_StatInfos.size();
    }



    GR::f64 Statistics::GetLastReset()
    {
      return m_LastResetTime;
    }




    void Statistics::IncrementCounter( const GR::String& Name, unsigned long Increment )
   {
     GR::f64 now = GetStartTimestamp();

     // Check if we already know this Counter
     std::map<GR::String,tStatsEntry>::iterator it = m_StatInfos.find( Name );
     if ( it != m_StatInfos.end() )
     {
       // Found an entry
       tStatsEntry& entry = it->second;
       if ( entry.Type == StatsType::ST_COUNTER )
       {
         // Update the counter
         if ( entry.Count <= std::numeric_limits<GR::u64>::max() - Increment )
         {
           entry.Count += Increment;
         }
         else
         {
           // Overflow, reset this counter
           entry.Count     = Increment;
           entry.LastReset = now;
         }
         entry.LastCall = now;
       }
     }
     else
     {
       // New counter
       tStatsEntry    entry;

       entry.Type         = StatsType::ST_COUNTER;
       entry.Count        = 1;
       entry.LastCall     = now;
       entry.LastReset    = now;
       entry.MaxTime      = now;

       m_StatInfos[Name] = entry;
     }
   }



   void Statistics::DecrementCounter( const GR::String& Name, unsigned long Decrement )
   {
     GR::f64 now = GetStartTimestamp();

     // Check if we already know this Counter
     std::map<GR::String,tStatsEntry>::iterator it = m_StatInfos.find( Name );
     if ( it != m_StatInfos.end() )
     {
       // Found an entry
       tStatsEntry& entry = it->second;

       if ( entry.Type == StatsType::ST_COUNTER )
       {
         // Update the counter
         if ( entry.Count >= Decrement )
         {
           entry.Count -= Decrement;
         }
         entry.LastCall = now;
       }
     }
     else
     {
       // New counter
       tStatsEntry      entry;

       entry.Type         = StatsType::ST_COUNTER;
       entry.Count        = 0;
       entry.LastCall     = now;
       entry.LastReset    = now;
       entry.MaxTime      = now;

       m_StatInfos[Name] = entry;
     }
   }



   GR::f64 Statistics::GetStartTimestamp()
   {
     // Measure the actual timestamp and return it as a GR::f64
     GR::DateTime::DateTime     now = GR::DateTime::DateTime::Now();

     GR::i64    ms = now.InMilliSeconds();

     //return (GR::f64)now.InMilliSeconds();
     return (GR::f64)ms;
   }



   void Statistics::BuildAverage( const GR::String& Name, GR::f64 Value )
   {
     GR::f64 now = GetStartTimestamp();

     // Check if we already know this Timer
     std::map<GR::String,tStatsEntry>::iterator  it( m_StatInfos.find( Name ) );
     if ( it != m_StatInfos.end() )
     {
       // Found an entry
       tStatsEntry& entry( it->second );

       if ( entry.Type == StatsType::ST_AVERAGE )
       {
         // Compute timer values
         Average( entry, Value, now );
       }
     }
     else
     {
       // New Timer - compute the timer values
       tStatsEntry entry;
       entry.Type        = StatsType::ST_AVERAGE;
       entry.LastReset   = now;
       Average( entry, Value, now );

       m_StatInfos[Name] = entry;
     }
   }



   void Statistics::Reset()
   {
     m_LastResetTime = GetStartTimestamp();

     m_StatInfos.clear();
   }



   void Statistics::Reset( const GR::String& Name )
   {
     std::map<GR::String,tStatsEntry>::iterator  it( m_StatInfos.find( Name ) );
     if ( it != m_StatInfos.end() )
     {
       ResetEntry( it->second, GetStartTimestamp() );
     }
   }



   Statistics::tStatsEntry Statistics::GetStats( const GR::String& Name )
   {
     std::map<GR::String,tStatsEntry>::iterator  it( m_StatInfos.find( Name ) );
     if ( it != m_StatInfos.end() )
     {
       return it->second;
     }

     return tStatsEntry();
   }



   void Statistics::RewindList()
   {
     m_Enumerator = m_StatInfos.begin();
   }



   bool Statistics::GetNextEntry( const GR::String*& Name, const Statistics::tStatsEntry*& Entry )
   {
     bool retVal = false;

     if ( m_Enumerator != m_StatInfos.end() )
     {
       Name    = &m_Enumerator->first;
       Entry   = &m_Enumerator->second;
       retVal  = true;

       ++m_Enumerator;
     }
     return retVal;
   }



   void Statistics::ComputeTime( Statistics::tStatsEntry& Entry, GR::f64 StartTime, GR::f64 StopTime )
   {
     GR::f64 diff = StopTime - StartTime;

     // Security
     if ( diff < 0 )
     {
       return;
     }

     // Check for new or freshly reset instance
     if ( Entry.Count == 0 )
     {
       Entry.Maximum   = Entry.Minimum = Entry.Average = diff;
       Entry.LastCall  = StopTime;
       Entry.MaxTime   = StopTime;
       Entry.Count     = 1;

       Entry.TopTenMaxTimes.insert( std::pair<GR::f64,GR::f64>( diff, StopTime ) );
       if ( Entry.TopTenMaxTimes.size() > 10 )
       {
         // remove first (lowest) entry
         Entry.TopTenMaxTimes.erase( Entry.TopTenMaxTimes.begin() );
       }
       return;
     }

     // Check if we did not hit the ceiling
     if ( Entry.Count < std::numeric_limits<GR::u64>::max() )
     {
       Entry.TopTenMaxTimes.insert( std::pair<GR::f64,GR::f64>( diff, StopTime ) );
       if ( Entry.TopTenMaxTimes.size() > 10 )
       {
         // remove first (lowest) entry
         Entry.TopTenMaxTimes.erase( Entry.TopTenMaxTimes.begin() );
       }

       if ( Entry.Maximum < diff )
       {
         // New max
         Entry.Maximum = diff;
         Entry.MaxTime = StopTime;
       }
       Entry.Minimum   = ( Entry.Minimum < diff) ? Entry.Minimum : diff;
       Entry.Average   = (GR::f64)( ( Entry.Average * (GR::f64)Entry.Count + diff ) / (GR::f64)( Entry.Count + 1 ) );
       Entry.LastCall  = StopTime;
       ++Entry.Count;
     }
     else
     {
       // No room to move, reset
       ResetEntry( Entry, StopTime );
     }
   }



   void Statistics::Average( Statistics::tStatsEntry& Entry, GR::f64 Value, GR::f64 StopTime )
   {
     // Check for new or freshly reset instance

     if ( Entry.Count == 0 )
     {
       Entry.Maximum    = Entry.Minimum = Entry.Average = Value;
       Entry.LastCall   = StopTime;
       Entry.MaxTime    = StopTime;
       Entry.Count      = 1;

       return;
     }

     // Check if we did not hit the ceiling
     if ( Entry.Count < std::numeric_limits<GR::u64>::max() )
     {
       if ( Entry.Maximum < Value )
       {
         // New max
         Entry.Maximum = Value;
         Entry.MaxTime = StopTime;
       }
       Entry.Minimum   = ( Entry.Minimum < Value ) ? Entry.Minimum : Value;
       Entry.Average   = (GR::f64)( Entry.Average * (GR::f64)Entry.Count + Value ) / (GR::f64)( Entry.Count + 1 );
       Entry.LastCall  = StopTime;
       ++Entry.Count;
     }
     else
     {
       // No room to move, reset
       ResetEntry( Entry, StopTime );
     }
   }



   void Statistics::ResetEntry( Statistics::tStatsEntry& Entry, GR::f64 ResetTimestamp )
   {
     Entry.Count     = 0;
     Entry.Maximum   = 0.0;
     Entry.Minimum   = 0.0;
     Entry.Average   = 0.0;
     Entry.LastCall  = ResetTimestamp;
     Entry.LastReset = ResetTimestamp;
     Entry.MaxTime   = ResetTimestamp;
     Entry.NumTimed  = 0;
     Entry.SumTimes  = 0;
   }


  }

}
