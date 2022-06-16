#ifndef STATISTICS_H
#define STATISTICS_H


#include <GR/GRTypes.h>

#include <limits>
#include <map>

#undef min
#undef max


namespace GR
{
  namespace Statistics
  {

    namespace StatsType
    {
      enum Value
      {
        ST_COUNTER,
        ST_TIMER,
        ST_AVERAGE
      };
    }

    class Statistics
    {
      public:

        /// Defines content of one stats entry
        struct tStatsEntry
        {
          StatsType::Value  Type;
          GR::u64           Count;
          GR::f64           Minimum;    ///< Minimum measured time or value
          GR::f64           Maximum;    ///< Maximum measured time or value
          GR::f64           Average;    ///< Average measured time or value
          GR::f64           LastCall;   ///< Time of the last invocation
          GR::f64           LastReset;  ///< Time of the last reset
          GR::f64           MaxTime;    ///< When did the maximum happen?
          GR::u64           NumTimed;   /// how many times timed
          GR::f64           SumTimes;   /// total sum of times

          std::multimap<GR::f64,GR::f64>     TopTenMaxTimes;


          tStatsEntry() :
            Type( StatsType::ST_COUNTER ),
            Count( 0 ),
            Minimum( std::numeric_limits<GR::f64>::max() ),
            Maximum( std::numeric_limits<GR::f64>::min() ),
            Average( 0.0 ),
            LastCall( 0.0 ),
            LastReset( 0.0 ),
            MaxTime( 0.0 ),
            NumTimed( 0 ),
            SumTimes( 0.0 )
          {
          }

        };



        Statistics();
        virtual ~Statistics();



      private:
  
        std::map<GR::String,tStatsEntry>         m_StatInfos;

        std::map<GR::String,tStatsEntry>::iterator   m_Enumerator;

        GR::f64                                   m_LastResetTime;


      public:
  
  
        // Synchronous Timer
        /// \brief Synchronous timer start method
        ///
        /// This method starts a timer selected by its name
        /// \param name is the name under which  the timer is stored
        void                      StartTimer( const GR::String& Timer );



        /// \brief Synchronous timer stop method
        ///
        /// This method stops a timer selected by its name
        /// \param name is the name under which  the timer is stored
        void                      StopTimer( const GR::String& Timer );
  
        void                      Dump() const;
        
        GR::String               ToString();
        
        GR::f64                   ShortestTime( const GR::String& Timer );
        GR::f64                   LongestTime( const GR::String& Timer );
        GR::f64                   SumTimes( const GR::String& Timer );
        GR::u64                   NumTimed( const GR::String& Timer );

        // Stats methods
        /// \brief Increment a Counters
        ///
        /// This method increments a counter selected by its name
        /// \param name is the name under which  the counter is stored
        /// \param inc is the increment, default is 1
        /// When the counter exceed its maximum (ULONG_MAX), it is reset
        void                      IncrementCounter( const GR::String& Name, unsigned long Increment = 1 );



        /// \brief Decrement a Counters
        ///
        /// This method decrements a counter selected by its name
        /// \param name is the name under which  the counter is stored
        /// \param dec is the decrement, default is 1
        /// When the counter exceed its minimum (0), it is reset
        void                      DecrementCounter( const GR::String& Name, unsigned long Decrement = 1 );




        // Asynchronous Timer
        /// \brief Asynchronous Timer, Get current time
        ///
        /// Use this method to get the current time and store it somewhere
        GR::f64                   GetStartTimestamp();



        /// \brief "Stop" asynchronous timer
        ///
        /// Call this method with the timestamp given by the GetStartTimestamp() method
        /// to measure the time since then.
        /// \param name is the name under which  the timer is stored
        /// \param startTimestamp is the time when the measurement started
        void StopTimer( const GR::String& Name, GR::f64 StartTimestamp );



        /// \brief Build average of a multiple times measured value
        ///
        /// Call this method to build the average of items that occur multiple times
        /// like for example size of messages or number of results of a select statement etc...
        /// \param name is the name under which  the average is stored
        /// \param value value of the item for this run
        void BuildAverage( const GR::String& Name, GR::f64 Value );



        // Reset
        /// Resets all statistical items
        void Reset();

        /// Resets statistical item given by name
        void Reset( const GR::String& Name );



        // Output
        /// \brief Returns the measured values for the given statistical item
        ///
        /// This method looks for the statistical item given by the name and
        /// returns a structure with the measured values
        /// \param name [in] name of the statistical item
        /// \return a StatsEntry structure with the computed values
        tStatsEntry GetStats( const GR::String& Name );



        /// \brief Sets the list of statistical items to the first item
        ///
        /// Use this method before you walk through the list of statistical
        /// items with the getNextEntry() method
        void RewindList();



        /// \brief Show data for next statistical item in list
        ///
        /// This method can be used to walk through the list of statistical items and
        /// retrieve the measured data for them. before starting call rewindList()!
        /// \param name [out] name of the current statistical item
        /// \param entry [out] StatsEntry structure with computed data
        /// \return While TRUE, there's a next item to retrieve
        bool GetNextEntry( const GR::String*& Name, const Statistics::tStatsEntry*& Entry );



        /// Returns the number of StatsEntries in the object
        size_t GetNumOfEntries();



        /// returns the time of the last complete reset of statistics
        GR::f64 GetLastReset();



      protected:



        /// \brief Compute minimum, maximum, average with new timer values
        ///
        /// \param entry references the StatsEntry to modify
        /// \param start The start timestamp of the last timer measurement
        /// \param stop The stop timestamp of the last timer measurement
        void ComputeTime( tStatsEntry& Entry, GR::f64 Start, GR::f64 Stop );



        /// \brief Compute average with new value
        ///
        /// \param entry references the StatsEntry to modify
        /// \param value The value of the last measurement
        /// \param stop The timestamp of the last measurement
        void Average( tStatsEntry& Entry, GR::f64 Value, GR::f64 Stop );



        /// \brief Utility function to reset values of one entry
        ///
        /// \param entry references the StatsEntry to modify
        /// \param resetTimestamp is the timestamp to store in StatsEntry::lastReset
        void ResetEntry( tStatsEntry& Entry, GR::f64 ResetTimestamp );

    };
  
  }
}

#endif // STATISTICS_H
  
  
  
  
