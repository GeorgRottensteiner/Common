#ifndef THREADBASE_H
#define THREADBASE_H



#if ( OPERATING_SYSTEM == OS_WINDOWS )
#include <windows.h>
#elif ( OPERATING_SYSTEM == OS_ANDROID )
#include <thread>
#endif

#include <debug/debugclient.h>



namespace System
{

  class ThreadBase
  {

    public:


#if ( OPERATING_SYSTEM == OS_WINDOWS )
      ThreadBase() :
        m_hThread( NULL ),
        m_hCloseEvent( NULL ),
        m_bPaused( false )
      {
  #if ( ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) ) \
||      ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE ) ) )
        InitializeCriticalSectionEx( &m_csHandleAccess, 0, 0 );
        m_hCloseEvent = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
  #else
        InitializeCriticalSection( &m_csHandleAccess );
        m_hCloseEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
  #endif
      }
#elif ( OPERATING_SYSTEM == OS_ANDROID )

      std::thread*       m_pThread;


      ThreadBase() :
        m_bPaused( false ),
        m_pThread( NULL )
      {
        //std::thread thread( ThreadProc );
      }
#endif


      virtual ~ThreadBase()
      {
        ShutDown();
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        if ( m_hCloseEvent != NULL )
        {
          CloseHandle( m_hCloseEvent );
          m_hCloseEvent = NULL;
        }
        DeleteCriticalSection( &m_csHandleAccess );
#endif
      }



      bool StartThread()
      {
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        if ( m_hThread )
        {
          dh::Log( "ThreadBase:StartThread - Thread is still alive" );
          return false;
        }
        DWORD   dwThreadID = 0;

        if ( m_hCloseEvent == NULL )
        {
          dh::Log( "ThreadBase:StartThread - CreateEvent failed" );
          return false;
        }

        ResetEvent( m_hCloseEvent );

        m_hThread = ::CreateThread( NULL, 0, ThreadFunc, this, 0, &dwThreadID );

        if ( m_hThread == NULL )
        {
          dh::Log( "ThreadBase:StartThread - CreateThread failed" );
          return false;
        }
        return true;
#elif ( OPERATING_SYSTEM == OS_ANDROID )
        if ( m_pThread != NULL )
        {
          dh::Log( "ThreadBase:StartThread - Thread is still alive" );
          return false;
        }

        // TODO - check if still running!
        m_pThread = new std::thread( &ThreadBase::Run, this );
        m_pThread->detach();
        return true;
#else
        return false;
#endif
      }



      void ShutDown( int iSleepDelay = 20 )
      {
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        //dh::Log( "ShutDown entered" );
        if ( m_hThread == NULL )
        {
          /*
          if ( m_hCloseEvent )
          {
            CloseHandle( m_hCloseEvent );
            m_hCloseEvent = NULL;
          }*/
          return;
        }
#elif ( OPERATING_SYSTEM == OS_ANDROID )
        // TODO - Race condition!
        if ( m_pThread != NULL )
        {
          m_pThread->join();
        }
#else
#endif
        if ( m_bPaused )
        {
          Resume();
        }

#if ( OPERATING_SYSTEM == OS_WINDOWS )
        SetEvent( m_hCloseEvent );

  #if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
        MSG   msg;
  #endif
        while ( true )
        {
          Sleep( iSleepDelay );

          EnterCriticalSection( &m_csHandleAccess );
          if ( m_hThread == NULL )
          {
            LeaveCriticalSection( &m_csHandleAccess );
            break;
          }
          LeaveCriticalSection( &m_csHandleAccess );

  #if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
          Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent );
  #else

          if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
          {
            GetMessage( &msg, 0, 0, 0 );
            TranslateMessage( &msg );
            DispatchMessage( &msg );
          }
  #endif
        }
#endif
      }



      bool                        Pause()
      {
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        if ( ( m_hThread == NULL )
        ||   ( m_bPaused ) )
        {
          return false;
        }
        DWORD dwPrevSuspendCount = SuspendThread( m_hThread );

        if ( dwPrevSuspendCount != -1 )
        {
          m_bPaused = true;
        }

        return ( dwPrevSuspendCount != -1 );
#else
        return false;
#endif
      }



      bool                        Resume()
      {
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        if ( ( m_hThread == NULL )
        ||   ( !m_bPaused ) )
        {
          return false;
        }
        DWORD dwPrevSuspendCount = ResumeThread( m_hThread );

        if ( dwPrevSuspendCount != -1 )
        {
          m_bPaused = false;
        }
        return ( dwPrevSuspendCount == 1 );
#else
        return false;
#endif
      }



      bool IsThreadAlive() const
      {
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        return ( m_hThread != NULL );
#else
        return false;
#endif
      }



      bool IsThreadPaused() const
      {
        return m_bPaused;
      }



    protected:

#if ( OPERATING_SYSTEM == OS_WINDOWS )
      volatile HANDLE             m_hThread;

      HANDLE                      m_hCloseEvent;

      CRITICAL_SECTION            m_csHandleAccess;
#endif

      bool                        m_bPaused;



      virtual int Run()
      {
        return 0;
      }



      bool                        HaveToShutDown()
      {
#if ( OPERATING_SYSTEM == OS_WINDOWS )
        //dh::Log( "HTS" );
        DWORD result = WaitForSingleObjectEx( m_hCloseEvent, 0, FALSE );
        // was DWORD   result = WaitForSingleObject( m_hCloseEvent, 0 );
        if ( result == WAIT_OBJECT_0 )
        {
          //dh::Log( "-yes" );
          // wieder setzen, wird evtl. verändert
          SetEvent( m_hCloseEvent );
          return true;
        }
        //dh::Log( "-no" );
#endif
        return false;
      }


    private:

#if ( OPERATING_SYSTEM == OS_WINDOWS )
      static DWORD WINAPI ThreadFunc( LPVOID lpParam )
      {
        ThreadBase*    pBase = (ThreadBase*)lpParam;

        DWORD dwResult = pBase->Run();

        EnterCriticalSection( &pBase->m_csHandleAccess );
        CloseHandle( pBase->m_hThread );
        pBase->m_hThread = NULL;
        LeaveCriticalSection( &pBase->m_csHandleAccess );

        return dwResult;
      }
#else
      static void ThreadFunc( void* lpParam )
      {
        ThreadBase*    pBase = (ThreadBase*)lpParam;

        int result = pBase->Run();

        // TODO - mutex!
        delete pBase->m_pThread;
        pBase->m_pThread = NULL;
      }

#endif

  };

}
#endif // THREADBASE_H



