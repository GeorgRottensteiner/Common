#include "DateTime.h"

#include <Misc/Format.h>
#include <String/StringUtil.h>
#include <String/Convert.h>

#include <memory.h>
#include <ctime>

#if OPERATING_SYSTEM == OS_TANDEM
#include <cextdecs.h>
#endif


namespace GR
{
  namespace DateTime
  {

    DateTime::DateTime( bool IsUTC ) :
      m_MicroSeconds( 0 ),
      m_IsUTC( IsUTC )
    {
      memset( &m_Date, 0, sizeof( m_Date ) );
    }



    DateTime::DateTime( int Day, int Month, int Year, int Hour, int Minute, int Seconds, int MicroSeconds, bool IsUTC ) :
      m_IsUTC( IsUTC )
    {
      memset( &m_Date, 0, sizeof( m_Date ) );
      m_Date.tm_mday  = Day;
      m_Date.tm_mon   = Month - 1;
      m_Date.tm_year  = Year - 1900;
      m_Date.tm_hour  = Hour;
      m_Date.tm_min   = Minute;
      m_Date.tm_sec   = Seconds;
      m_MicroSeconds  = MicroSeconds;

      Normalize();
    }



    bool DateTime::Normalize()
    {
      // normalize
      if ( IsUTC() )
      {
        return FromTime( ToTime() );
      }
      if ( ( ConvertToUTC() )
      &&   ( FromTime( ToTime() ) )
      &&   ( ConvertToLocalTime() ) )
      {
        return true;
      }
      return false;
    }



    GR::DateTime::DateTime DateTime::Now()
    {
      DateTime      NowDateTime( true );

      time_t        CurrentTime = 0;

      std::time( &CurrentTime );

#if OPERATING_SYSTEM == OS_TANDEM
      // fetch tm_wday, tm_yday, tm_isdst
      NowDateTime.m_Date = *std::localtime( &CurrentTime );

      // get everything else from here since the precision is higher
      short ts[8];

      // This creates a local timestamp
      INTERPRETTIMESTAMP( JULIANTIMESTAMP(), ts );

      NowDateTime.m_Date.tm_year  = ts[0] - 1900;
      NowDateTime.m_Date.tm_mon   = ts[1] - 1;
      NowDateTime.m_Date.tm_mday  = ts[2];
      NowDateTime.m_Date.tm_hour  = ts[3];
      NowDateTime.m_Date.tm_min   = ts[4];
      NowDateTime.m_Date.tm_sec   = ts[5];

      NowDateTime.m_MicroSeconds = ts[7] + 1000 * ts[6];
      //int tm_wday;  /* days since Sunday - [0,6] */
      //int tm_yday;  /* days since January 1 - [0,365] */
      //int tm_isdst; /* Daylight Savings Time flag */

      NowDateTime.ConvertToLocalTime();
#elif OPERATING_SYSTEM == OS_ANDROID
      NowDateTime.FromTime( CurrentTime );
      NowDateTime.ConvertToLocalTime();
#elif OPERATING_SYSTEM == OS_WINDOWS
      SYSTEMTIME    sysTime;

      GetLocalTime( &sysTime );

      NowDateTime.m_Date.tm_year  = sysTime.wYear - 1900;
      NowDateTime.m_Date.tm_mon   = sysTime.wMonth - 1;
      NowDateTime.m_Date.tm_mday  = sysTime.wDay;
      NowDateTime.m_Date.tm_hour  = sysTime.wHour;
      NowDateTime.m_Date.tm_min   = sysTime.wMinute;
      NowDateTime.m_Date.tm_sec   = sysTime.wSecond;
      NowDateTime.m_MicroSeconds  = sysTime.wMilliseconds * 1000;
#else   
      localtime_s( &NowDateTime.m_Date, &CurrentTime );
#endif      
      return NowDateTime;
    }



    GR::i64 DateTime::DifferenceInSeconds( const DateTime& OtherTime ) const
    {
      return InSeconds() - OtherTime.InSeconds();
    }



    GR::i64 DateTime::InSeconds() const
    {
      return (GR::i64)ToTime();
    }



    GR::i64 DateTime::DifferenceInMilliSeconds( const DateTime& OtherTime ) const
    {
      return InMilliSeconds() - OtherTime.InMilliSeconds();
    }



    GR::i64 DateTime::InMilliSeconds() const
    {
      std::tm     TempTime( m_Date );
      return std::mktime( &TempTime ) * 1000 + m_MicroSeconds / 1000;
    }



    void DateTime::AddDays( GR::i32 Days )
    {
      std::tm     TempTime( m_Date );

      time_t     TimeInS = std::mktime( &TempTime );

      TimeInS += Days * 60 * 60 * 24;

#if ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_ANDROID )
      m_Date = *std::localtime( &TimeInS );      
#else      
      localtime_s( &m_Date, &TimeInS );
#endif      
    }



    GR::u16 DateTime::Day() const
    {
      return (GR::u16)m_Date.tm_mday;
    }



    GR::u16 DateTime::Month() const
    {
      return (GR::u16)( m_Date.tm_mon + 1 );
    }



    GR::u16 DateTime::Year() const
    {
      return (GR::u16)( m_Date.tm_year + 1900 );
    }



    GR::u16 DateTime::DayInYear() const
    {
      return (GR::u16)( m_Date.tm_yday + 1 );
    }



    GR::u8 DateTime::DayInWeek() const
    {
      GR::u8      Day = (GR::u8)m_Date.tm_wday;

      // returns 0 for Monday, 6 for Sunday

      // let Monday be 0, Sunday be 6
      Day = (GR::u8)( ( Day + 7 - 1 ) % 7 );

      return Day;
    }



    GR::u8 DateTime::Hour() const
    {
      return (GR::u8)m_Date.tm_hour;
    }



    GR::u8 DateTime::Minute() const
    {
      return (GR::u8)m_Date.tm_min;
    }



    GR::u8 DateTime::Second() const
    {
      return (GR::u8)m_Date.tm_sec;
    }



    GR::u32 DateTime::MicroSecond() const
    {
      return (GR::u32)m_MicroSeconds;
    }



    bool DateTime::IsUTC() const
    {
      return m_IsUTC;
    }



    std::tm DateTime::ToTM() const
    {
      return m_Date;
    }



    time_t DateTime::ToTime() const
    {
      if ( m_IsUTC )
      {
        return MakeTimeFromUTCTM();
      }
      std::tm     TempTime( m_Date );
      std::tm     TempTime2( m_Date );

      time_t      NewTime = std::mktime( &TempTime );

      if ( TempTime.tm_isdst != m_Date.tm_isdst )
      {
        TempTime2.tm_isdst = TempTime.tm_isdst;
        NewTime = std::mktime( &TempTime2 );
      }
      return NewTime;
    }



    // expects a std::tm containing an UTC time stamp
    int DateTime::tmcomp( std::tm* atmp, std::tm* btmp )
    {
	    int	result = 0;

	    if ( ( result = ( atmp->tm_year - btmp->tm_year ) ) == 0 
      &&   ( result = ( atmp->tm_mon - btmp->tm_mon ) ) == 0 
      &&   ( result = ( atmp->tm_mday - btmp->tm_mday ) ) == 0 
      &&   ( result = ( atmp->tm_hour - btmp->tm_hour ) ) == 0 
      &&   ( result = ( atmp->tm_min - btmp->tm_min ) ) == 0 )
      {
			  result = atmp->tm_sec - btmp->tm_sec;
      }
	    return result;
    }



    time_t DateTime::MakeTimeFromUTCTM2() const
    {
	    int			      dir;
	    int			      bits;
	    int			      saved_seconds;
	    time_t				t;
      std::tm			  yourtm;
      std::tm       mytm;


	    yourtm        = m_Date;
	    saved_seconds = yourtm.tm_sec;
	    yourtm.tm_sec = 0;

	    // Calculate the number of magnitude bits in a time_t
	    // (this works regardless of whether time_t is
	    // signed or unsigned, though lint complains if unsigned).
	    for ( bits = 0, t = 1; t > 0; ++bits, t <<= 1 );

	    // If time_t is signed, then 0 is the median value,
	    // if time_t is unsigned, then 1 << bits is median.
	    t = ( t < 0) ? 0 : ( (time_t)1 << bits );

	    // Some gmtime() implementations are broken and will return
	    // NULL for time_ts larger than 35 bits even on 64-bit platforms
	    // so we'll just cap it at 35 bits */
	    if ( bits > 35 )
      {
        bits = 35;
      }

	    for ( ; ; ) 
      {
#if ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_ANDROID )
        std::tm* pTM = gmtime( &t );
        if ( !pTM )
        {
          return -1;
        }
        mytm = *pTM;
#else        
		    if ( gmtime_s( &mytm, &t ) )
        {
          return -1;
        }
#endif		    
		    dir = tmcomp( &mytm, &yourtm );
		    if ( dir != 0 ) 
        {
			    if ( bits-- < 0 )
          {
				    return -1;
          }
			    if ( bits < 0 )
          {
				    --t;
          }
			    else if ( dir > 0 )
          {
				    t -= (time_t)1 << bits;
          }
			    else
          {
            t += (time_t)1 << bits;
          }
			    continue;
		    }
		    break;
	    }
	    t += saved_seconds;
	    return t;
    }



#if ( OPERATING_SYSTEM != OS_WINDOWS ) || ( _MSC_VER >= 1400 )
    time_t _mkgmtime( const struct tm* tm )
    {
      // Month-to-day offset for non-leap-years.
      static const int month_day[12] =
      { 
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 
      };

      // Most of the calculation is easy; leap years are the main difficulty.
      int month = tm->tm_mon % 12;
      int year = tm->tm_year + tm->tm_mon / 12;
      if ( month < 0 )
      {   
        // Negative values % 12 are still negative.
        month += 12;
        --year;
      }

      // This is the number of Februaries since 1900.
      const int year_for_leap = ( month > 1 ) ? year + 1 : year;

      time_t rt = tm->tm_sec                             // Seconds
            + 60 * ( tm->tm_min                          // Minute = 60 seconds
            + 60 * ( tm->tm_hour                         // Hour = 60 minutes
            + 24 * ( month_day[month] + tm->tm_mday - 1  // Day = 24 hours
            + 365 * ( year - 70 )                        // Year = 365 days
            + ( year_for_leap - 69 ) / 4                 // Every 4 years is     leap...
            - ( year_for_leap - 1 ) / 100                // Except centuries...
            + ( year_for_leap + 299 ) / 400 ) ) );       // Except 400s.
      return rt < 0 ? -1 : rt;
    }
#endif



    time_t DateTime::MakeTimeFromUTCTM() const
    {
      std::tm     oldTM( m_Date );

      return _mkgmtime( &oldTM );
    }



    void DateTime::SetTime( const std::tm& Time )
    {
      m_Date = Time;
    }



    DateTime DateTime::IsoWeekOne( GR::u16 Year )
    {
      DateTime     DateTime4thJanuary;

      // get the date for the 4-Jan for this year
      DateTime4thJanuary.m_Date.tm_mday  = 4;
      DateTime4thJanuary.m_Date.tm_mon   = 0;
      DateTime4thJanuary.m_Date.tm_year  = Year - 1900;

      // get the ISO day number for this date 1==Monday, 7==Sunday
      int dayNumber = DateTime4thJanuary.DayInWeek() + 1;

      // return the date of the Monday that is less than or equal
      // to this date
      DateTime4thJanuary.AddDays( 1 - dayNumber );

      return DateTime4thJanuary;
    }



    GR::String DateTime::WeekDayAsName() const
    {
      switch ( DayInWeek() )
      {
        case 0:
          return "Montag";
        case 1:
          return "Dienstag";
        case 2:
          return "Mittwoch";
        case 3:
          return "Donnerstag";
        case 4:
          return "Freitag";
        case 5:
          return "Samstag";
        case 6:
          return "Sonntag";
      }
      return "unbekannt";
    }



    GR::String DateTime::MonthAsName() const
    {
      switch ( Month() )
      {
        case 1:
          return "Januar";
        case 2:
          return "Februar";
        case 3:
          return "März";
        case 4:
          return "April";
        case 5:
          return "Mai";
        case 6:
          return "Juni";
        case 7:
          return "Juli";
        case 8:
          return "August";
        case 9:
          return "September";
        case 10:
          return "Oktober";
        case 11:
          return "November";
        case 12:
          return "Dezember";
      }
      return "unbekannt";
    }
    


    GR::String DateTime::ToString( const GR::String& Format ) const
    {
      // "<DD>.<MM>.<YYYY> <HH>:<mm>:<ss>"
      GR::String result = GR::Strings::Replace( Format, "<DD>", Misc::Format( "%01:2%" ) << Day() );
      result = GR::Strings::Replace( result, "<MM>", Misc::Format( "%01:2%" ) << Month() );
      result = GR::Strings::Replace( result, "<YYYY>", Misc::Format( "%01:4%" ) << Year() );
      result = GR::Strings::Replace( result, "<YY>", Misc::Format( "%01:2%" ) << ( Year() % 100 ) );
      result = GR::Strings::Replace( result, "<HH>", Misc::Format( "%01:2%" ) << Hour() );
      result = GR::Strings::Replace( result, "<mm>", Misc::Format( "%01:2%" ) << Minute() );
      result = GR::Strings::Replace( result, "<ss>", Misc::Format( "%01:2%" ) << Second() );
      result = GR::Strings::Replace( result, "<ms>", Misc::Format( "%01:6%" ) << MicroSecond() );
      
      return result;
    }



    // parses format YYYY-MM-DD HH:mm:ss.msmsms or YYYY-MM-DD HH:mm:ss
    bool DateTime::FromString( const GR::String& DateString, bool IsUTC )
    {
      if ( ( DateString.length() != 26 )
      &&   ( DateString.length() != 19 ) )
      {
        return false;
      }
      if ( DateString.length() == 19 )
      {
        if ( ( DateString[4] != '-' )
        ||   ( DateString[7] != '-' )
        ||   ( DateString[10] != ' ' )
        ||   ( DateString[13] != ':' )
        ||   ( DateString[16] != ':' ) )
        {
          return false;
        }
        if ( ( !GR::Strings::IsNumeric( DateString,  0, 4 ) )
        ||   ( !GR::Strings::IsNumeric( DateString,  5, 2 ) )
        ||   ( !GR::Strings::IsNumeric( DateString,  8, 2 ) )
        ||   ( !GR::Strings::IsNumeric( DateString, 11, 2 ) )
        ||   ( !GR::Strings::IsNumeric( DateString, 14, 2 ) )
        ||   ( !GR::Strings::IsNumeric( DateString, 17, 2 ) ) )
        {
          return false;
        }

        memset( &m_Date, 0, sizeof( m_Date ) );
        m_Date.tm_mday = GR::Convert::ToI32( DateString.substr( 8, 4 ) );
        m_Date.tm_mon  = GR::Convert::ToI32( DateString.substr( 5, 2 ) ) - 1;
        m_Date.tm_year = GR::Convert::ToI32( DateString.substr( 0, 4 ) ) - 1900;
        m_Date.tm_hour = GR::Convert::ToI32( DateString.substr( 11, 2 ) );
        m_Date.tm_min  = GR::Convert::ToI32( DateString.substr( 14, 2 ) );
        m_Date.tm_sec  = GR::Convert::ToI32( DateString.substr( 17, 2 ) );
        m_MicroSeconds = 0;
        m_IsUTC = IsUTC;
        if ( IsUTC )
        {
          m_Date.tm_isdst = 0;
        }
        else
        {
          mktime( &m_Date );
        }
        return true;
      }

      if ( ( DateString[4] != '-' )
      ||   ( DateString[7] != '-' )
      ||   ( DateString[10] != ' ' )
      ||   ( DateString[13] != ':' )
      ||   ( DateString[16] != ':' )
      ||   ( DateString[19] != '.' ) )
      {
        return false;
      }
      if ( ( !GR::Strings::IsNumeric( DateString,  0, 4 ) )
      ||   ( !GR::Strings::IsNumeric( DateString,  5, 2 ) )
      ||   ( !GR::Strings::IsNumeric( DateString,  8, 2 ) )
      ||   ( !GR::Strings::IsNumeric( DateString, 11, 2 ) )
      ||   ( !GR::Strings::IsNumeric( DateString, 14, 2 ) )
      ||   ( !GR::Strings::IsNumeric( DateString, 17, 2 ) )
      ||   ( !GR::Strings::IsNumeric( DateString, 20 ) ) )
      {
        return false;
      }

      memset( &m_Date, 0, sizeof( m_Date ) );
      m_Date.tm_mday = GR::Convert::ToI32( DateString.substr( 8, 4 ) );
      m_Date.tm_mon  = GR::Convert::ToI32( DateString.substr( 5, 2 ) ) - 1;
      m_Date.tm_year = GR::Convert::ToI32( DateString.substr( 0, 4 ) ) - 1900;
      m_Date.tm_hour = GR::Convert::ToI32( DateString.substr( 11, 2 ) );
      m_Date.tm_min  = GR::Convert::ToI32( DateString.substr( 14, 2 ) );
      m_Date.tm_sec  = GR::Convert::ToI32( DateString.substr( 17, 2 ) );
      m_MicroSeconds = GR::Convert::ToI32( DateString.substr( 20, 6 ) );
      m_IsUTC = IsUTC;

      if ( IsUTC )
      {
        m_Date.tm_isdst = 0;
      }
      else
      {
        mktime( &m_Date );
      }
      return true;
    }



    bool DateTime::FromTime( time_t UTCTime )
    {
#if ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_ANDROID )
      std::tm* pTM = gmtime( &UTCTime );
      if ( !pTM )
      {
        return false;
      }
      m_Date = *pTM;
#else      
      if ( gmtime_s( &m_Date, &UTCTime ) )
      {
        return false;
      }
#endif      
      m_MicroSeconds = 0;
      m_IsUTC = true;
      return true;
    }



    bool DateTime::ConvertToUTC()
    {
      if ( m_IsUTC )
      {
        return true;
      }
      time_t    utcTime = ToTime();

#if ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_ANDROID )
      std::tm*  pUTCStamp = gmtime( &utcTime );
      if ( pUTCStamp == NULL )
      {
        return false;
      }
      m_Date = *pUTCStamp;
#else      
      if ( gmtime_s( &m_Date, &utcTime ) )
      {
        return false;
      }
#endif      
      m_IsUTC = true;
      return true;
    }



    bool DateTime::ConvertToLocalTime()
    {
      if ( !m_IsUTC )
      {
        return true;
      }
      time_t  utcTime = ToTime();
      
#if ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_ANDROID )
      std::tm*  pLocalStamp = localtime( &utcTime );
      if ( pLocalStamp == NULL )
      {
        return false;
      }
      m_Date = *pLocalStamp;
#else      
      if ( localtime_s( &m_Date, &utcTime ) )
      {
        return false;
      }
#endif      
      mktime( &m_Date );
      m_IsUTC = false;
      return true;
    }



    bool DateTime::IsDaylightSavingActive() const
    {
      if ( !m_IsUTC )
      {
        return !!m_Date.tm_isdst;
      }
      GR::DateTime::DateTime    tempLocal = *this;

      if ( !tempLocal.ConvertToLocalTime() )
      {
        return false;
      }
      return tempLocal.IsDaylightSavingActive();
    }


    bool DateTime::operator==( const DateTime& RHS ) const
    {
      if ( m_IsUTC != RHS.m_IsUTC )
      {
        return false;
      }
      return ( ( m_Date.tm_mday == RHS.m_Date.tm_mday )
      &&       ( m_Date.tm_mon == RHS.m_Date.tm_mon )
      &&       ( m_Date.tm_year == RHS.m_Date.tm_year )
      &&       ( m_Date.tm_hour == RHS.m_Date.tm_hour )
      &&       ( m_Date.tm_min == RHS.m_Date.tm_min )
      &&       ( m_Date.tm_sec == RHS.m_Date.tm_sec )
      &&       ( m_MicroSeconds == RHS.m_MicroSeconds ) );
    }



    bool DateTime::operator!=( const DateTime& RHS ) const
    {
      return !operator==( RHS );
    }



    bool DateTime::operator< ( const DateTime& RHS ) const
    {
      DateTime    otherDate( RHS );

      if ( m_IsUTC != otherDate.m_IsUTC )
      {
        if ( !m_IsUTC )
        {
          if ( !otherDate.ConvertToLocalTime() )
          {
            return false;
          }
        }
        else
        {
          if ( !otherDate.ConvertToUTC() )
          {
            return false;
          }
        }
      }
      if ( m_Date.tm_year < otherDate.m_Date.tm_year )
      {
        return true;
      }
      else if ( m_Date.tm_year > otherDate.m_Date.tm_year )
      {
        return false;
      }
      if ( m_Date.tm_mon < otherDate.m_Date.tm_mon )
      {
        return true;
      }
      else if ( m_Date.tm_mon > otherDate.m_Date.tm_mon )
      {
        return false;
      }
      if ( m_Date.tm_mday < otherDate.m_Date.tm_mday )
      {
        return true;
      }
      else if ( m_Date.tm_mday > otherDate.m_Date.tm_mday )
      {
        return false;
      }
      if ( m_Date.tm_hour < otherDate.m_Date.tm_hour )
      {
        return true;
      }
      else if ( m_Date.tm_hour > otherDate.m_Date.tm_hour )
      {
        return false;
      }
      if ( m_Date.tm_min < otherDate.m_Date.tm_min )
      {
        return true;
      }
      else if ( m_Date.tm_min > otherDate.m_Date.tm_min )
      {
        return false;
      }
      if ( m_Date.tm_sec < otherDate.m_Date.tm_sec )
      {
        return true;
      }
      else if ( m_Date.tm_sec > otherDate.m_Date.tm_sec )
      {
        return false;
      }
      return ( m_MicroSeconds < otherDate.m_MicroSeconds );
    }



    bool DateTime::IsLeapYear( GR::u16 Year )
    {
      if ( ( Year % 400 ) == 0 )
      {
        return true;
      }
      if ( ( Year % 100 ) == 0 )
      {
        return false;
      }
      if ( ( Year % 4 ) == 0 )
      {
        return true;
      }
      return false;
    }



    bool DateTime::IsValidDate( GR::u16 Day, GR::u16 Month, GR::u16 Year )
    {
      if ( ( Day < 0 )
      ||   ( Day > 31 )
      ||   ( Month < 0 )
      ||   ( Month > 12 ) )
      {
        return false;
      }
      switch ( Month )
      {
        case 2:
          if ( IsLeapYear( Year ) )
          {
            if ( Day > 29 )
            {
              return false;
            }
          }
          else if ( Day > 28 )
          {
            return false;
          }
          break;
        case 4:
        case 6:
        case 9:
        case 11:
          if ( Day > 30 )
          {
            return false;
          }
          break;
      }
      return true;
    }



    DateTime DateTime::GetStartOf( StartOf::Value Start )
    {
      switch ( Start )
      {
        case StartOf::THIS_MINUTE:
          return GR::DateTime::DateTime( Day(), Month(), Year(), Hour(), Minute(), 0, 0, IsUTC() );
        case StartOf::NEXT_MINUTE:
          return GR::DateTime::DateTime( Day(), Month(), Year(), Hour(), Minute() + 1, 0, 0, IsUTC() );
        case StartOf::THIS_HOUR:
          return GR::DateTime::DateTime( Day(), Month(), Year(), Hour(), 0, 0, 0, IsUTC() );
        case StartOf::NEXT_HOUR:
          return GR::DateTime::DateTime( Day(), Month(), Year(), Hour() + 1, 0, 0, 0, IsUTC() );
        case StartOf::THIS_DAY:
          return GR::DateTime::DateTime( Day(), Month(), Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::NEXT_DAY:
          return GR::DateTime::DateTime( Day() + 1, Month(), Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::THIS_WEEK:
          return GR::DateTime::DateTime( Day() - DayInWeek(), Month(), Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::NEXT_WEEK:
          return GR::DateTime::DateTime( Day() - DayInWeek() + 7, Month(), Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::THIS_MONTH:
          return GR::DateTime::DateTime( 1, Month(), Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::NEXT_MONTH:
          return GR::DateTime::DateTime( 1, Month() + 1, Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::THIS_YEAR:
          return GR::DateTime::DateTime( 1, 1, Year(), 0, 0, 0, 0, IsUTC() );
        case StartOf::NEXT_YEAR:
          return GR::DateTime::DateTime( 1, 1, Year() + 1, 0, 0, 0, 0, IsUTC() );
      }
      return *this;
    }



  }

}

