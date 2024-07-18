#ifndef DATETIME_H
#define DATETIME_H

#include <ctime>

#include <GR/GRTypes.h>

#include <OS/OS.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <tchar.h>
#else
#ifdef UNICODE
#define _T(x)      L ## x
#else
#define _T(x)      x
#endif
#endif



namespace GR
{
  namespace DateTime
  {
    namespace StartOf
    {
      enum Value
      {
        THIS_MINUTE,
        NEXT_MINUTE,
        THIS_HOUR,
        NEXT_HOUR,
        THIS_DAY,
        NEXT_DAY,
        THIS_WEEK,
        NEXT_WEEK,
        THIS_MONTH,
        NEXT_MONTH,
        THIS_YEAR,
        NEXT_YEAR
      };
    }

    class DateTime
    {
      private:

        std::tm                     m_Date;

        int                         m_MicroSeconds;

        bool                        m_IsUTC;


        time_t                      MakeTimeFromUTCTM() const;

        static int                  tmcomp( std::tm* atmp, std::tm* btmp );


      public:


        DateTime( bool IsUTC = false );

        DateTime( int Day, int Month, int Year, int Hour = 0, int Minute = 0, int Seconds = 0, int MicroSeconds = 0, bool IsUTC = false );


        static DateTime Now();


        GR::i64                     DifferenceInSeconds( const DateTime& OtherTime ) const;
        GR::i64                     DifferenceInMilliSeconds( const DateTime& OtherTime ) const;
        int                         DifferenceInDays( const DateTime& ReferenceDate ) const;

        GR::i64                     InSeconds() const;
        GR::i64                     InMilliSeconds() const;
        GR::u64                     InMicroSecondsSince01011970() const;
        GR::u64                     InMicroSecondsSince01014713BC() const;


        void                        AddDays( GR::i32 Days );

        GR::u16                     Day() const;
        GR::u16                     Month() const;
        GR::u16                     Year() const;
        GR::u16                     DayInYear() const;
        GR::u8                      DayInWeek() const;    // returns 0 for Monday, 6 for Sunday
        GR::u8                      Hour() const;
        GR::u8                      Minute() const;
        GR::u8                      Second() const;
        GR::u32                     MicroSecond() const;
        bool                        IsUTC() const;
        bool                        IsDaylightSavingActive() const;

        time_t                      ToTime() const;
        std::tm                     ToTM() const;

        void                        SetTime( const std::tm& Time, int MicroSeconds = 0 );

        static DateTime             IsoWeekOne( GR::u16 Year );

        GR::String                  WeekDayAsName() const;
        GR::String                  MonthAsName() const;

        GR::String                  ToString( const GR::String& Format = "<DD>.<MM>.<YYYY> <HH>:<mm>:<ss>" ) const;

        // parses format YYYY-MM-DD HH:mm:ss.msmsms
        bool                        FromString( const GR::String& DateString, bool IsUTC = false );

        bool                        FromTime( time_t UTCTime );
        bool                        FromMicrosecondsSince01011970( GR::u64 UTCMicrosecondsSince01011970 );
        bool                        FromMicrosecondsSince01014713BC( GR::u64 UTCMicrosecondsSince01014713BC );

        bool                        ConvertToLocalTime();
        bool                        ConvertToUTC();


        time_t                      MakeTimeFromUTCTM2() const;

        static bool                 IsLeapYear( GR::u16 Year );
        static bool                 IsValidDate( GR::u16 Day, GR::u16 Month, GR::u16 Year );

        DateTime                    GetStartOf( StartOf::Value Start );

        // normalizes (e.g. 32nd January becomes 1st February, etc.)
        bool                        Normalize();



        bool operator==( const DateTime& RHS ) const;
        bool operator!=( const DateTime& RHS ) const;

        bool operator< ( const DateTime& RHS ) const;
        bool operator<= ( const DateTime& RHS ) const;
        bool operator> ( const DateTime& RHS ) const;
        bool operator>= ( const DateTime& RHS ) const;
    };

  }
}

#endif // DATETIME_H
