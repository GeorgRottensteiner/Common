#ifndef _PJ_STOPWATCH_H_INCLUDED
#define _PJ_STOPWATCH_H_INCLUDED

#include <string>
#include <sstream>

namespace PJ
{
  class CStopWatch
  {
  private:
    long m_iStartTime;
    long m_iCountedTime;
    bool m_bStopped;

    void _UpdateTime()
    {
      if ( m_bStopped ) return;
      m_iCountedTime = GetTickCount() - m_iStartTime;
    }

  public:

    CStopWatch()
      : m_iStartTime( GetTickCount() ),
        m_iCountedTime( 0 ),
        m_bStopped( false )
    {}

    CStopWatch( const CStopWatch& rhs )
      : m_iStartTime(   rhs.m_iStartTime ),
        m_iCountedTime( rhs.m_iCountedTime ),
        m_bStopped( rhs.m_bStopped )
    {}

    CStopWatch& operator=( const CStopWatch& rhs )
    {
      if ( this == &rhs ) return *this;
      m_iStartTime    = rhs.m_iStartTime;
      m_iCountedTime  = rhs.m_iCountedTime;
      m_bStopped      = rhs.m_bStopped;
      return *this;
    }

    long Reset()
    {
      m_iStartTime    = GetTickCount();
      m_iCountedTime  = 0;
      return m_iCountedTime;
    }

    long Ticks()
    {
      _UpdateTime();
      return m_iCountedTime;
    }

    long Seconds()
    {
      _UpdateTime();
      return m_iCountedTime / 1000;
    }

    GR::String String()
    {
      _UpdateTime();
      std::ostringstream oss;
      oss.precision( 3 );
      oss << ( m_iCountedTime / 1000.0 );
      return GR::String( oss.str() );
    }

    long Start()
    {
      m_bStopped = false;
      _UpdateTime();
      return m_iCountedTime;
    }

    long Stop()
    {
      _UpdateTime();
      m_bStopped = true;
      return m_iCountedTime;
    }
  };
};

#endif //_PJ_STOPWATCH_H_INCLUDED