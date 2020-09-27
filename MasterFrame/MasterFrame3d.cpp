#include <MasterFrame/MasterFrame3d.h>

#include <Misc/Misc.h>
#include <DateTime\Timer.h>

#include <Input/Input.h>
#include <Sound/Sound.h>



CMasterFrame3d*       pGlobal3DMasterFrame = NULL;



CMasterFrame3d::CMasterFrame3d()
:  CMasterFrame()
{
  pGlobal3DMasterFrame = this;
  m_bUseDepthBuffer = true;
}



CMasterFrame3d::~CMasterFrame3d()
{
}



void CMasterFrame3d::Reset()
{
  CMasterFrame::Reset();
}



BOOL CMasterFrame3d::CleanUp()
{
  CD3DViewer::Cleanup3DEnvironment();

  return CMasterFrame::CleanUp();
}



BOOL CMasterFrame3d::Create( int iNewWidth, int iNewHeight, unsigned char ucDepth, DWORD dwFlags )
{

  CD3DViewer::Cleanup3DEnvironment();

  m_ucOrigDepth = ucDepth;
  m_dwCreationWidth   = iNewWidth;
  m_dwCreationHeight  = iNewHeight;
  m_dwCreationDepth   = ucDepth;
  if ( !CMasterFrame::Create( iNewWidth, iNewHeight, ucDepth, dwFlags ) )
  {
    return FALSE;
  }

  if ( ParameterSwitch( "f" ) )
  {
    m_bWindowed = FALSE;
  }

  if ( !CD3DViewer::Create( m_hInstance, m_hwndMain ) )
  {
    CloseWindow( m_hwndMain );
    DestroyWindow( m_hwndMain );
    m_hwndMain = NULL;
    return FALSE;
  }

  return TRUE;

}



/*-ToggleFullScreen-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CMasterFrame3d::ToggleFullScreen( void )
{

  BOOL bResult = SUCCEEDED( CD3DViewer::ToggleFullscreen() );

  SendEventNow( "masterframe", MF_EVENT_SCREENMODE_CHANGED, 0, 0, 0, 0 );
  return bResult;

}



/*-RestoreAppDeviceObjects----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMasterFrame3d::RestoreAppDeviceObjects()
{

  CD3DViewer::RestoreAppDeviceObjects();

  SendEventNow( "masterframe", MF_EVENT_SCREENMODE_CHANGED, 0, 0, 0, 0 );

}



void CMasterFrame3d::DisplayConsole()
{
  int             iPos = 4;

  std::list<GR::String>::iterator    it( m_listConsoleLines.begin() );
  while ( it != m_listConsoleLines.end() )
  {
    GR::String&    strLine = *it;

    Print( 4, iPos, strLine.c_str() );
    iPos += 15;

    if ( iPos >= (int)m_dwRenderHeight - 30 )
  {
      break;
    }

    ++it;
  }

  if ( ( GetTickCount() % 1024 ) > 512 )
  {
    Print( 4, m_dwRenderHeight - 15, ( ">" + m_ConsoleEntry + "_" ).c_str() );
  }
  else
  {
    Print( 4, m_dwRenderHeight - 15, ( ">" + m_ConsoleEntry ).c_str() );
  }
}



void CMasterFrame3d::ProcessCommandTokens( GR::TokenSequence& m_TokenSequence, GR::String& strCommand )
{

  GR::TokenSequence::iterator       m_itTokenPos;

  m_itTokenPos = m_TokenSequence.begin();

  if ( m_itTokenPos == m_TokenSequence.end() )
  {
    return;
  }

  GR::CToken currentToken = *m_itTokenPos;

  if ( currentToken.Type() == m_infoT )
  {
    m_itTokenPos++;
  }
  CMasterFrame::ProcessCommandTokens( m_TokenSequence, strCommand );

}



LRESULT FAR PASCAL CMasterFrame3d::WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  return CMasterFrame::MasterFrameWndProc( hwnd, message, wParam, lParam );
}



BOOL CMasterFrame3d::Run()
{

  SetInstance( (HINSTANCE)GetModuleHandle( NULL ) );

  // BAUSTELLE CommandLine
  //SetCommandLine( lpCmdLine );
  AddListener( this );

  if ( !InitInstance() )
  {
    CleanUp();
    return FALSE;
  }

  MSG           msg;

  while ( 1 )
  {
    if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
    {
      if ( !GetMessage( &msg, NULL, 0, 0 ) )
      {
        break;
      }
      if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
      {
        continue;
      }
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
    else
    {
      if ( !m_ApplicationActive )
      {
        Sleep( 20 );
      }
      // Konsole an/aus
      if ( CInputSystem::Instance().ReleasedKeyPressed( CInputSystem::KEY_F11 ) )
      {
        ToggleConsole();
      }
      if ( CInputSystem::Instance().ReleasedKeyPressed( CInputSystem::KEY_F3 ) )
      {
        SaveSnapShot( CMisc::AppPath( "shot.tga" ) );
      }
      // Full-Screen-Toggle
      if ( CInputSystem::Instance().AltPressed() )
      {
        // ALT
        if ( ( CInputSystem::Instance().ReleasedKeyPressed( CInputSystem::KEY_ENTER ) )
        ||   ( CInputSystem::Instance().ReleasedKeyPressed( CInputSystem::KEY_NUMPAD_ENTER ) ) )
        {
          // Alt-Enter
          ToggleFullScreen();
        }
      }

      if ( !IsPaused() )
      {
        if ( IsTimed() )
        {
          DWORD     dwGnu = 0;

          while ( Time::Timer::Time() - GetLastFrameTime() > GetFrameTime() )
          {
            dwGnu++;
            if ( dwGnu > (DWORD)m_MasterRegistry.GetVarI( "maxupdates" ) )
            {
              // Safety-Break um eventuelle Endlosloops zu verhindern
              SetLastFrameTime( Time::Timer::Time() );
              break;
            }
            ProcessEventQueue();

            ExecuteTasks( (GR::f32)GetFrameTime() );

            UpdateFrame();

            SetLastFrameTime( GetLastFrameTime() + GetFrameTime() );

            if ( m_ShutDownRequested )
            {
              CleanUp();
              PostMessage( m_hwndMain, WM_CLOSE, 0, 0 );
              break;
            }
          }
        }
        else
        {
          ProcessEventQueue();

          ExecuteTasks( (GR::f32)( Time::Timer::Time() - GetLastFrameTime() ) );
          UpdateFrame();

          if ( m_ShutDownRequested )
          {
            CleanUp();
            PostMessage( m_hwndMain, WM_CLOSE, 0, 0 );
          }
        }
      }
      else
      {
        ProcessEventQueue();

        ExecuteTasks( 0.0f );
      }
      if ( m_pd3dDevice )
      {
        float   ElapsedTime = (float)Time::Timer::Time( Time::Timer::TF_GETELAPSEDTIME );
        if ( ElapsedTime > 10.0f )
        {
          ElapsedTime = 10.0f;
        }


        if ( m_LockFPS )
        {
          if ( ElapsedTime < 1.0f / GetLockedFPS() )
          {
            Sleep( DWORD( 1000 / GetLockedFPS() - ElapsedTime * 1000 ) );
            continue;
          }
        }
        if ( IsPaused() )
        {
          ElapsedTime = 0.0f;
        }
        if ( RenderBegin() )
        {
          // Clear the viewport
          m_LastFrameTicks = Time::Timer::Time();

          UpdatePerDisplayFrame( ElapsedTime );
          DisplayFrame();

          m_Frames++;
          if ( floor( m_ElapsedTicks ) < floor( m_ElapsedTicks + ElapsedTime ) )
          {
            m_FPS = (float)m_Frames;
            m_Frames = 0;
          }
          m_ElapsedTicks += ElapsedTime;

          // Console
          if ( ConsoleVisible() )
          {
            DisplayConsole();
          }

          RenderDone();
        }
      }
    }
  }

  CleanUp();

  //ExitInstance();

  return TRUE;

}



