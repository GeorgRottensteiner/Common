#include "Timer.h"

#include <OS/OS.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <windows.h>
#include <mmsystem.h>

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
#pragma comment( lib, "winmm.lib" )
#endif
#endif

#if OPERATING_SYSTEM == OS_TANDEM
#include <cextdecs.h>
#include <sys/timeb.h>
#endif

#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
#include <SDL.h>

#include <debug/debugclient.h>
#endif


namespace Time
{

  /*-Timer----------------------------------------------------------------------+
  |        TF_RESET           - to reset the timer                             |
  |        TF_START           - to start the timer                             |
  |        TF_STOP            - to stop (or pause) the timer                   |
  |        TF_ADVANCE         - to advance the timer by 0.1 seconds            |
  |        TF_GETABSOLUTETIME - to get the absolute system time                |
  |        TF_GETAPPTIME      - to get the current time                        |
  |        TF_GETELAPSEDTIME  - to get the time that elapsed between           |
  |                                TF_GETELAPSEDTIME calls                     |
  +----------------------------------------------------------------------------*/

  double Timer::Time( eTimeFlags tfCommand )
  {
    #if OPERATING_SYSTEM == OS_WINDOWS
    static BOOL       m_TimerInitialized = FALSE;
    static BOOL       m_UsingQPF         = FALSE;
    static LONGLONG   m_QPFTicksPerSec  = 0;

    if ( tfCommand == TF_REINITIALIZE )
    {
      m_TimerInitialized = FALSE;
    }

    // Initialize the timer
    if ( !m_TimerInitialized )
    {
      m_TimerInitialized = TRUE;

#if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
      // sicherstellen, dass wir nur auf einem Prozessor laufen
      ULONG_PTR     ProcessAffinityMask,
                    SysAffinityMask, 
                    FirstProcessorMask = 1;

		  GetProcessAffinityMask( GetCurrentProcess(), &ProcessAffinityMask, &SysAffinityMask );
		  while( !( FirstProcessorMask & SysAffinityMask ) )
      {
			  FirstProcessorMask <<= 1;
      }
		  SetThreadAffinityMask( GetCurrentThread(), FirstProcessorMask );
		  Sleep( 1 );
#endif

      // Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
      // not supported, we will timeGetTime() which returns milliseconds.
      LARGE_INTEGER TicksPerSec;
      m_UsingQPF = QueryPerformanceFrequency( &TicksPerSec );
      if ( m_UsingQPF )
      {
        m_QPFTicksPerSec = TicksPerSec.QuadPart;
      }
    }

    if ( m_UsingQPF )
    {
      static LONGLONG m_StopTime        = 0;
      static LONGLONG m_LastElapsedTime = 0;
      static LONGLONG m_BaseTime        = 0;
      static LONGLONG m_OffsetTime      = 0;
      double          Time;
      double          ElapsedTime;
      LARGE_INTEGER   quadTime;
      
      // Get either the current time or the stop time, depending
      // on whether we're stopped and what command was sent
      if ( ( m_StopTime != 0 )
      &&   ( tfCommand != TF_START )
      &&   ( tfCommand != TF_GETABSOLUTETIME ) )
      {
        quadTime.QuadPart = m_StopTime;
      }
      else
      {
        QueryPerformanceCounter( &quadTime );
        if ( m_LastElapsedTime == 0 )
        {
          m_LastElapsedTime = quadTime.QuadPart;
        }
      }
      if ( tfCommand == TF_SET_OFFSET_TIME )
      {
        m_OffsetTime = quadTime.QuadPart;
        return 0.0f;
      }

      // Return the elapsed time
      if ( tfCommand == TF_GETELAPSEDTIME )
      {
        ElapsedTime = (double)( quadTime.QuadPart - m_LastElapsedTime - m_OffsetTime ) / (double)m_QPFTicksPerSec;
        m_LastElapsedTime = quadTime.QuadPart;
        return ElapsedTime;
      }

      // Return the current time
      if ( tfCommand == TF_GETAPPTIME )
      {
        return (double)( quadTime.QuadPart - m_BaseTime - m_OffsetTime ) / (double)m_QPFTicksPerSec;
      }

      // Reset the timer
      if ( tfCommand == TF_RESET )
      {
        m_BaseTime        = quadTime.QuadPart;
        m_LastElapsedTime = quadTime.QuadPart;
        return 0.0;
      }

      // Start the timer
      if ( tfCommand == TF_START )
      {
        m_BaseTime += quadTime.QuadPart - m_StopTime;
        m_StopTime = 0;
        m_LastElapsedTime = quadTime.QuadPart;
        return 0.0;
      }

      // Stop the timer
      if ( tfCommand == TF_STOP )
      {
        m_StopTime        = quadTime.QuadPart;
        m_LastElapsedTime = quadTime.QuadPart;
        return 0.0;
      }

      // Advance the timer by 1/10th second
      if ( tfCommand == TF_ADVANCE )
      {
        m_StopTime += m_QPFTicksPerSec / 10;
        return 0.0;
      }

      if ( tfCommand == TF_GETABSOLUTETIME )
      {
        Time = (double)( quadTime.QuadPart - m_OffsetTime ) / (double)m_QPFTicksPerSec;
        return Time;
      }
      // Invalid command specified
      return -1.0;   
    }
    else
    {
#if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
      // Get the time using timeGetTime()
      static double m_LastElapsedTime  = 0.0;
      static double m_BaseTime         = 0.0;
      static double m_StopTime         = 0.0;
      double Time;
      double ElapsedTime;
      
      // Get either the current time or the stop time, depending
      // on whether we're stopped and what command was sent
      if ( ( m_StopTime != 0.0 )
      &&   ( tfCommand != TF_START )
      &&   ( tfCommand != TF_GETABSOLUTETIME ) )
      {
        Time = m_StopTime;
      }
      else
      {
        #if OPERATING_SUB_SYSTEM == OS_SUB_SDL
        unsigned long     ticks = SDL_GetTicks();
        Time = ticks * 0.001;
        #else
        Time = timeGetTime() * 0.001;
        #endif
      }

      // Return the elapsed time
      if ( tfCommand == TF_GETELAPSEDTIME )
      {   
        ElapsedTime       = (double)( Time - m_LastElapsedTime );
        m_LastElapsedTime = Time;
        return ElapsedTime;
      }

      // Return the current time
      if ( tfCommand == TF_GETAPPTIME )
      {
        return ( Time - m_BaseTime );
      }

      // Reset the timer
      if ( tfCommand == TF_RESET )
      {
        m_BaseTime         = Time;
        m_LastElapsedTime  = Time;
        return 0.0;
      }

      // Start the timer
      if ( tfCommand == TF_START )
      {
        m_BaseTime += Time - m_StopTime;
        m_StopTime = 0.0f;
        m_LastElapsedTime  = Time;
        return 0.0;
      }

      // Stop the timer
      if ( tfCommand == TF_STOP )
      {
        m_StopTime = Time;
        return 0.0;
      }

      // Advance the timer by 1/10th second
      if ( tfCommand == TF_ADVANCE )
      {
        m_StopTime += 0.1f;
        return 0.0;
      }

      if ( tfCommand == TF_GETABSOLUTETIME )
      {
        return Time;
      }
#endif
      // Invalid command specified
      return -1.0;
    }
    #elif OPERATING_SYSTEM == OS_TANDEM
    // Get the time using timeGetTime()
    static double m_LastElapsedTime  = 0.0;
    static double m_BaseTime         = 0.0;
    static double m_StopTime         = 0.0;
    double Time;
    double ElapsedTime;
    
    // Get either the current time or the stop time, depending
    // on whether we're stopped and what command was sent
    if ( ( m_StopTime != 0.0 )
    &&   ( tfCommand != TF_START )
    &&   ( tfCommand != TF_GETABSOLUTETIME ) )
    {
      Time = m_StopTime;
    }
    else
    {
      Time = (double)JULIANTIMESTAMP() * 0.000001;

      /*
      struct timeb  mark;
      ftime( &mark );
        
      Time = (double)mark.time + (double)mark.millitm * 0.001;
      */
    }

    // Return the elapsed time
    if ( tfCommand == TF_GETELAPSEDTIME )
    {   
      ElapsedTime = Time - m_LastElapsedTime;
      m_LastElapsedTime = Time;
      return ElapsedTime;
    }

    // Return the current time
    if ( tfCommand == TF_GETAPPTIME )
    {
      return( Time - m_BaseTime );
    }

    // Reset the timer
    if ( tfCommand == TF_RESET )
    {
      m_BaseTime         = Time;
      m_LastElapsedTime  = Time;
      return 0.0;
    }

    // Start the timer
    if ( tfCommand == TF_START )
    {
      m_BaseTime += Time - m_StopTime;
      m_StopTime = 0.0f;
      m_LastElapsedTime  = Time;
      return 0.0;
    }

    // Stop the timer
    if ( tfCommand == TF_STOP )
    {
      m_StopTime = Time;
      return 0.0;
    }

    // Advance the timer by 1/10th second
    if ( tfCommand == TF_ADVANCE )
    {
      m_StopTime += 0.1;
      return 0.0;
    }

    if ( tfCommand == TF_GETABSOLUTETIME )
    {
      return Time;
    }

    // Invalid command specified
    return -1.0;
    #elif OPERATING_SYSTEM == OS_WEB
    // Get the time using timeGetTime()
    static double m_LastElapsedTime  = 0.0;
    static double m_BaseTime         = 0.0;
    static double m_StopTime         = 0.0;
    double Time;
    double ElapsedTime;
    
    // Get either the current time or the stop time, depending
    // on whether we're stopped and what command was sent
    if ( ( m_StopTime != 0.0 )
    &&   ( tfCommand != TF_START )
    &&   ( tfCommand != TF_GETABSOLUTETIME ) )
    {
      Time = m_StopTime;
    }
    else
    {
      unsigned long     ticks = SDL_GetTicks();
      Time = ticks * 0.001;
    }

    // Return the elapsed time
    if ( tfCommand == TF_GETELAPSEDTIME )
    {   
      ElapsedTime = Time - m_LastElapsedTime;
      m_LastElapsedTime = Time;
      return ElapsedTime;
    }

    // Return the current time
    if ( tfCommand == TF_GETAPPTIME )
    {
      return( Time - m_BaseTime );
    }

    // Reset the timer
    if ( tfCommand == TF_RESET )
    {
      m_BaseTime         = Time;
      m_LastElapsedTime  = Time;
      return 0.0;
    }

    // Start the timer
    if ( tfCommand == TF_START )
    {
      m_BaseTime += Time - m_StopTime;
      m_StopTime = 0.0f;
      m_LastElapsedTime  = Time;
      return 0.0;
    }

    // Stop the timer
    if ( tfCommand == TF_STOP )
    {
      m_StopTime = Time;
      return 0.0;
    }

    // Advance the timer by 1/10th second
    if ( tfCommand == TF_ADVANCE )
    {
      m_StopTime += 0.1;
      return 0.0;
    }

    if ( tfCommand == TF_GETABSOLUTETIME )
    {
      return Time;
    }

    // Invalid command specified
    return -1.0;
    #else
    return 0.0;
    #endif
  }

}
