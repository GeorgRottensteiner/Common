#include <MasterFrame/MasterFrame.h>
#include <Misc/misc.h>
#include <DateTime\Timer.h>

#include <String/Path.h>

#include <Xtreme/Input/WinInput.h>

#include <Xtreme/Audio/DXSound.h>

#include <IO/FileUtil.h>

#include <WinSys/WinUtils.h>
#include <Setup/Groups.h>

#include <debug\debugclient.h>



MasterFrame*     g_pGlobalMasterFrame = NULL;



MasterFrame::MasterFrame() :
  m_Caption( "MasterFrame" ),
  m_ShutDownRequested( false ),
  m_ExclusiveInput( false ),
  m_DoNotFlipPages( false ),
  m_DoNotAllowConsole( false ),
  m_IconID( 0 ),
  m_BitmapResourceID( 0 ),
  m_AppPath( "" ),
  m_hCursor( LoadCursor( NULL, IDC_ARROW ) ),
  m_pInput( NULL )
{

  if ( g_pGlobalMasterFrame == NULL )
  {
    g_pGlobalMasterFrame = this;
  }

  GR::Service::Environment::Instance().SetService( "GlobalQueue", &GLOBAL_QUEUE );
  GR::Service::Environment::Instance().SetService( "Logger", &m_DebugService );

  Reset();

  m_Frames            = 0;
  m_FPS                = 0.0f;
  m_ElapsedTicks       = 0.0f;

  m_AppPath = CMisc::AppPath( "" );

  ConsolePrint( "MasterFrame V%d.%02d", MFF_VERSION / 100, MFF_VERSION % 100 );
}



MasterFrame::~MasterFrame()
{
  GR::Service::Environment::Instance().RemoveService( "GlobalQueue" );

  CleanUp();

  GR::Service::Environment::Instance().RemoveService( "Logger" );
  GR::Service::Environment::Instance().RemoveService( "SubclassManager" );
}



void MasterFrame::Reset()
{
  m_ApplicationActive   = false;
  m_FullScreen          = false;
  m_InputInitialized    = false;
  m_SoundInitialized    = false;
  m_hInstance           = NULL;
  m_Flags               = 0;
  m_UseFrameTime        = true;
  m_LockFPS             = false;

  m_MasterRegistry.SetVarI( "frametime", 50 );
  m_MasterRegistry.SetVarI( "lockedfps", 60 );
  m_MasterRegistry.SetVarI( "maxupdates", 20 );
  m_StartTime          = 0.0f;
}



bool MasterFrame::CleanUp()
{
  if ( m_pInput != NULL )
  {
    GR::Service::Environment::Instance().RemoveService( "Input" );
    m_pInput->Release();
    delete m_pInput;
    m_pInput = NULL;
  }

  m_InputInitialized = FALSE;
  m_SoundInitialized = FALSE;

  UnregisterClass( "MasterFrameClass", GetModuleHandle( NULL ) );
  return TRUE;
}



void MasterFrame::ShutDown()
{
  PostMessage( m_Window.Hwnd, WM_CLOSE, 0, 0 );
  //m_ShutDownRequested = true;
}



void MasterFrame::EnableCursor( bool Enable )
{
  if ( Enable )
  {
    if ( m_Flags & MFF_HIDE_MOUSE )
    {
      m_Flags &= ~MFF_HIDE_MOUSE;
      SendMessage( m_Window.Hwnd, WM_SETCURSOR, HTERROR, 0 );
    }
  }
  else
  {
    if ( !( m_Flags & MFF_HIDE_MOUSE ) )
    {
      m_Flags |= MFF_HIDE_MOUSE;
      SetCursor( NULL );
    }
  }
}



void MasterFrame::Pause()
{
  m_Flags |= MFF_PAUSED;
}



void MasterFrame::Resume()
{
  m_Flags &= ~MFF_PAUSED;
}



void MasterFrame::ToggleConsole()
{
  if ( m_DoNotAllowConsole )
  {
    return;
  }
  m_Flags ^= MFF_SHOW_CONSOLE;

  SendEventNow( MF_EVENT_CONSOLE_TOGGLED, ( m_Flags & MFF_SHOW_CONSOLE ) ? 1 : 0 );

  if ( m_Flags & MFF_SHOW_CONSOLE )
  {
    Pause();
  }
  else
  {
    Resume();
  }
}



void MasterFrame::ProcessCommandTokens( GR::Tokenizer::TokenSequence& m_TokenSequence, GR::String& strCommand )
{
  GR::Tokenizer::TokenSequence::iterator       m_itTokenPos;

  m_itTokenPos = m_TokenSequence.begin();
  GR::Tokenizer::Token currentToken = *m_itTokenPos;

  if ( currentToken.Type() == m_setT )
  {
    m_itTokenPos++;
    if ( m_itTokenPos == m_TokenSequence.end() )
    {
      // alle Variablen ausgeben
      tMapVars::iterator    vai;

      for ( vai = m_Vars.begin(); vai != m_Vars.end(); vai++ )
      {
        ConsolePrint( "%s = %s", vai->first.c_str(), vai->second.c_str() );
      }
    }
    else if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::LABEL )
    {
      GR::String   strVariable,
                    strValue;

      strVariable = m_itTokenPos->String();
      m_itTokenPos++;
      if ( m_itTokenPos == m_TokenSequence.end() )
      {
        // Variablenwert anzeigen
        ConsolePrint( "%s=%s", strVariable.c_str(), GetVar( strVariable.c_str() ).c_str() );
      }
      else
      {
        // Variable setzen/erzeugen
        strValue = m_itTokenPos->String();
        SetVar( strVariable.c_str(), strValue.c_str() );
      }
    }
  }
  else if ( currentToken.Type() == m_incT )
  {
    m_itTokenPos++;
    if ( m_itTokenPos == m_TokenSequence.end() )
    {
      return;
    }
    if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::LABEL )
    {
      GR::String   strVariable,
                    strValue;

      strVariable = m_itTokenPos->String();
      // Variable setzen/erzeugen
      int iValue = GetVarI( strVariable.c_str() );
      iValue++;
      SetVarI( strVariable.c_str(), iValue );
    }
  }
  else if ( currentToken.Type() == m_decT )
  {
    m_itTokenPos++;
    if ( m_itTokenPos == m_TokenSequence.end() )
    {
      return;
    }
    if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::LABEL )
    {
      GR::String   strVariable,
                    strValue;

      strVariable = m_itTokenPos->String();

      int iValue = GetVarI( strVariable.c_str() );
      iValue--;
      SetVarI( strVariable.c_str(), iValue );
    }
  }
  else if ( currentToken.Type() == m_eraseT )
  {
    m_itTokenPos++;
    if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::LABEL )
    {
      // eine Variable löschen
      GR::String   strVariable;


      strVariable = m_itTokenPos->String();
      DeleteVar( strVariable );
      ConsolePrint( "%s removed.", strVariable.c_str() );
    }
  }
  else if ( currentToken.Type() == m_helpT )
  {
    ConsolePrint( "set" );
    ConsolePrint( " -gesetzte Variablen anzeigen" );
    ConsolePrint( "set [Variable] [Wert]" );
    ConsolePrint( " -Variable setzen" );
    ConsolePrint( "erase [Variable]" );
    ConsolePrint( " -Variable löschen" );
    ConsolePrint( "images" );
    ConsolePrint( " -dumpt alle (über den Frame) geladenen Bilder" );
    ConsolePrint( "sounds" );
    ConsolePrint( " -dumpt alle (über den Frame) geladenen Sounds" );
    ConsolePrint( "play [Nummer]" );
    ConsolePrint( " -spielt Sound Nr. Nummer" );
    ConsolePrint( "loop [Nummer]" );
    ConsolePrint( " -loop Sound Nr. Nummer" );
    ConsolePrint( "stop [Nummer]" );
    ConsolePrint( " -spielt Sound Nr. Nummer, ohne Nummer werden alle Sounds gestoppt" );
    ConsolePrint( "toggle" );
    ConsolePrint( " -schaltet zwischen Windowed/Fullscreen um" );
    ConsolePrint( "quit" );
    ConsolePrint( " -beendet das Programm" );
  }
  else if ( currentToken.Type() == m_infoT )
  {
    m_itTokenPos++;
    ConsolePrint( "Info available on IMAGES, SOUND." );
  }
  else if ( currentToken.Type() == m_soundT )
  {
    ConsolePrint( "%d sounds loaded", DXSound::Instance().m_Sounds.size() );
  }
  else if ( currentToken.Type() == m_toggleT )
  {
    // Toggle Fullscreen/Windowed
    ToggleFullScreen();
  }
  else if ( currentToken.Type() == m_playT )
  {
    // Sound spielen
    m_itTokenPos++;
    if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::INT )
    {
      DXSound::Instance().Play( m_itTokenPos->Int() );
    }
  }
  else if ( currentToken.Type() == m_stopT )
  {
    // Sound stoppen
    m_itTokenPos++;
    if ( m_itTokenPos == m_TokenSequence.end() )
    {
      DXSound::Instance().StopAll();
    }
    else if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::INT )
    {
      DXSound::Instance().Stop( m_itTokenPos->Int() );
    }
  }
  else if ( currentToken.Type() == m_loopT )
  {
    // Sound loopen
    m_itTokenPos++;
    if ( m_itTokenPos->Type() == GR::Tokenizer::TokenType::INT )
    {
      DXSound::Instance().Loop( m_itTokenPos->Int() );
    }
  }
  else if ( currentToken.Type() == m_quitT )
  {
    // Quit
    PostMessage( m_Window.Hwnd, WM_CLOSE, 0, 0 );
  }
  else if ( currentToken.Type() == GR::Tokenizer::TokenType::UNKNOWN )
  {
    ConsolePrint( "Syntax error." );
  }
  else
  {
    AddEvent( strCommand.c_str(), MF_EVENT_CONSOLE_COMMAND, 0, 0, 0, 0 );
  }
}



void MasterFrame::ParseCommandLine()
{
  char    szCmdLine[MAX_PATH];

  wsprintf( szCmdLine, GetCommandLine() );

  m_CommandLine = "";
  if ( szCmdLine )
  {
    m_CommandLine = szCmdLine;
  }

  m_StartParameter.clear();

  unsigned int             iPos = 0;

  GR::String     strParam = "";

  while ( iPos < m_CommandLine.length() )
  {
    if ( m_CommandLine[iPos] == '"' )
    {
      iPos++;
      while ( iPos < m_CommandLine.length() )
      {
        if ( m_CommandLine[iPos] == '"' )
        {
          m_StartParameter.push_back( strParam );
          strParam = "";
          break;
        }
        else
        {
          strParam += m_CommandLine[iPos];
        }
        ++iPos;
      }
    }
    else if ( m_CommandLine[iPos] == ' ' )
    {
      if ( strParam.length() )
      {
        m_StartParameter.push_back( strParam );
      }
      strParam = "";
    }
    else
    {
      strParam += m_CommandLine[iPos];
    }

    ++iPos;
  }
  if ( strParam.length() )
  {
    m_StartParameter.push_back( strParam );
  }

}



bool MasterFrame::ParameterSwitch( const char* szSwitch )
{

  std::list<GR::String>::iterator    it( m_StartParameter.begin() );
  while ( it != m_StartParameter.end() )
  {
    GR::String&    strParam = *it;

    if ( strParam.empty() )
    {
      ++it;
      continue;
    }

    if ( ( strParam[0] == '-' )
    ||   ( strParam[0] == '/' ) )
    {
      if ( strParam.substr( 1 ) == szSwitch )
      {
        return true;
      }
    }

    ++it;
  }

  return false;

}



bool MasterFrame::Create( int iNewWidth, int iNewHeight, unsigned char ucDepth, DWORD dwFlags )
{
  m_SubclassManager.AddHandler( "MasterFrame", fastdelegate::MakeDelegate( this, &MasterFrame::WindowProc ) );

  WNDCLASS      WndClass;


  m_Flags                 = dwFlags;
  m_ApplicationActive     = FALSE;
  m_FullScreen            = FALSE;

  m_MasterRegistry.SetVarI( "resx", iNewWidth );
  m_MasterRegistry.SetVarI( "resy", iNewHeight );
  m_MasterRegistry.SetVarI( "windowed_resx", iNewWidth );
  m_MasterRegistry.SetVarI( "windowed_resy", iNewHeight );

  m_MasterRegistry.SetVarI( "frametime", 50 );

  ParseCommandLine();

  if ( ParameterSwitch( "f" ) )
  {
    m_FullScreen = TRUE;
  }

  if ( m_Flags & MFF_FULLSCREEN )
  {
    m_FullScreen = TRUE;
  }

  if ( ParameterSwitch( "w" ) )
  {
    m_FullScreen = FALSE;
  }

  m_MasterRegistry.SetVarI( "fs_resx", iNewWidth );
  m_MasterRegistry.SetVarI( "fs_resy", iNewHeight );

  if ( m_Window.Hwnd == NULL )
  {
    ZeroMemory( &WndClass, sizeof( WndClass ) );
    WndClass.style          = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc    = MasterFrameWndProc;
    WndClass.cbClsExtra     = 0;
    WndClass.cbWndExtra     = 0;
    WndClass.hInstance      = m_hInstance;
    WndClass.hIcon          = LoadIcon( m_hInstance, MAKEINTRESOURCE( m_IconID ) );
    WndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    WndClass.hbrBackground  = (HBRUSH)GetStockObject( WHITE_BRUSH );
    WndClass.lpszMenuName   = NULL;
    WndClass.lpszClassName  = "MasterFrameClass";
    if ( !RegisterClass( &WndClass ) )
    {
      return false;
    }

    if ( m_FullScreen )
    {
      m_Window.Hwnd = CreateWindow( "MasterFrameClass",
                                 m_Caption.c_str(),
                                 WS_OVERLAPPED,
                                 0,
                                 0,
                                 m_MasterRegistry.GetVarI( "resx" ),
                                 m_MasterRegistry.GetVarI( "resy" ),
                                 NULL,
                                 NULL,
                                 m_hInstance,
                                 NULL );

      int     iWidth,
              iHeight;
      iWidth  = m_MasterRegistry.GetVarI( "resx" );
      iHeight = m_MasterRegistry.GetVarI( "resy" );

      if ( m_Flags & MFF_CAPTION )
      {
        if ( m_Flags & MFF_RESIZEABLE )
        {
          iWidth += 2 * GetSystemMetrics( SM_CXSIZEFRAME );
          iHeight += 2 * GetSystemMetrics( SM_CYSIZEFRAME ) + GetSystemMetrics( SM_CYCAPTION );
        }
        else
        {
          iWidth += 2 * GetSystemMetrics( SM_CXFIXEDFRAME );
          iHeight += 2 * GetSystemMetrics( SM_CYFIXEDFRAME ) + GetSystemMetrics( SM_CYCAPTION );
        }
      }
      else
      {
        if ( m_Flags & MFF_RESIZEABLE )
        {
          iWidth += 2 * GetSystemMetrics( SM_CXSIZEFRAME );
          iHeight += 2 * GetSystemMetrics( SM_CYSIZEFRAME );
        }
      }
      m_MasterRegistry.SetVarI( "windowed_resx", iWidth );
      m_MasterRegistry.SetVarI( "windowed_resy", iHeight );

      m_wplMain.length = sizeof( WINDOWPLACEMENT );
      m_wplMain.rcNormalPosition.left = ( GetSystemMetrics( SM_CXSCREEN ) - iWidth ) / 2;
      m_wplMain.rcNormalPosition.top = ( GetSystemMetrics( SM_CYSCREEN ) - iHeight ) / 2;
      m_wplMain.rcNormalPosition.right = m_wplMain.rcNormalPosition.left + iWidth;
      m_wplMain.rcNormalPosition.bottom = m_wplMain.rcNormalPosition.top + iHeight;
      m_wplMain.showCmd = SW_SHOW;
    }
    else
    {
      DWORD   dwDummyFlags;
      int     iWidth,
              iHeight;
      dwDummyFlags = WS_POPUP & ~( WS_CAPTION | WS_SYSMENU | WS_BORDER );
      iWidth  = m_MasterRegistry.GetVarI( "resx" );
      iHeight = m_MasterRegistry.GetVarI( "resy" );

      if ( m_Flags & MFF_CAPTION )
      {
        dwDummyFlags = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU;
        if ( m_Flags & MFF_RESIZEABLE )
        {
          dwDummyFlags |= WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
          iWidth += 2 * GetSystemMetrics( SM_CXSIZEFRAME );
          iHeight += 2 * GetSystemMetrics( SM_CYSIZEFRAME ) + GetSystemMetrics( SM_CYCAPTION );
        }
        else
        {
          dwDummyFlags |= WS_MINIMIZEBOX;
          iWidth += 2 * GetSystemMetrics( SM_CXFIXEDFRAME );
          iHeight += 2 * GetSystemMetrics( SM_CYFIXEDFRAME ) + GetSystemMetrics( SM_CYCAPTION );
        }
      }
      else
      {
        if ( m_Flags & MFF_RESIZEABLE )
        {
          dwDummyFlags |= WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
          iWidth += 2 * GetSystemMetrics( SM_CXSIZEFRAME );
          iHeight += 2 * GetSystemMetrics( SM_CYSIZEFRAME );
        }
      }

      RECT    rc;
      ::SetRect( &rc, 0, 0, iNewWidth, iNewHeight );

      AdjustWindowRectEx( &rc, dwDummyFlags, FALSE, WS_EX_CLIENTEDGE );
      int     windowWidth = rc.right - rc.left;
      int     windowHeight = rc.bottom - rc.top;

      m_Window.Hwnd = CreateWindowEx( WS_EX_CLIENTEDGE, "MasterFrameClass",
                                 m_Caption.c_str(),
                                 dwDummyFlags,
                                 0,
                                 0,
                                 windowWidth,
                                 windowHeight,
                                 NULL,
                                 NULL,
                                 m_hInstance,
                                 NULL );
    }
    if ( m_Window.Hwnd == NULL )
    {
      return false;
    }

    // Sound-Unit initialisieren
    if ( !m_SoundInitialized )
    {
      if ( !DXSound::Instance().Initialize( GR::Service::Environment::Instance() ) )
      {
        ConsolePrint( "Sound initialization failed." );
        return false;
      }
      if ( !DXSound::Instance().IsInitialized() )
      {
        ConsolePrint( "No sound available." );
      }
      else
      {
        ConsolePrint( "Sound initialized." );
      }
      m_SoundInitialized = TRUE;
    }
  }

  ShowWindow( m_Window.Hwnd, SW_SHOW );

  // Input-Unit initialisieren
  if ( !m_InputInitialized )
  {
    if ( m_Flags & MFF_WININPUT )
    {
      m_pInput = new CWinInput();
    }
    else if ( m_Flags & MFF_COMPATIBILITY )
    {
      // Kompatibilität heißt, es läuft auf auch DX3 (NT!)
      m_pInput = new CDXInput();
    }
    else
    {
      // DirectX volle Wäsche
      m_pInput = new CDXInput();
    }
    if ( !m_pInput->Initialize( GR::Service::Environment::Instance() ) )
    {
      return FALSE;
    }
    GR::Service::Environment::Instance().SetService( "Input", m_pInput );

    m_pInput->SetTaskPriority( 1000 );

    m_pInput->AddListener( this );
    m_InputInitialized = TRUE;
  }

  // Splash-Screen
  if ( m_BitmapResourceID )
  {
    HBITMAP   hbm = LoadBitmap( m_hInstance, MAKEINTRESOURCE( m_BitmapResourceID ) );
    if ( hbm )
    {
      HDC   hdcMain = GetDC( m_Window.Hwnd );
      HDC   hdcBitmap = CreateCompatibleDC( hdcMain );

      SetStretchBltMode( hdcMain, COLORONCOLOR );

      BITMAP    bm;
      GetObject( hbm, sizeof( BITMAP ), &bm );

      SelectObject( hdcBitmap, hbm );

      RECT    rc;
      GetClientRect( m_Window.Hwnd, &rc );
      StretchBlt( hdcMain, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                  hdcBitmap, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

      DeleteDC( hdcBitmap );
      ReleaseDC( m_Window.Hwnd, hdcMain );

      DeleteObject( hbm );
    }
  }
  else
  {
    // loading...
    HDC   hdcMain = GetDC( m_Window.Hwnd );

    RECT  rc;
    GetClientRect( m_Window.Hwnd, &rc );
    FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

    SelectObject( hdcMain, GetStockObject( DEFAULT_GUI_FONT ) );

    SetTextColor( hdcMain, 0xffffff );
    SetBkMode( hdcMain, TRANSPARENT );
    DrawText( hdcMain, "loading...", 10, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

    ReleaseDC( m_Window.Hwnd, hdcMain );
  }

  UpdateWindow( m_Window.Hwnd );

  m_LastFrameTicks = Time::Timer::Time();

  return true;
}



bool MasterFrame::ToggleFullScreen()
{
  return true;
}



void MasterFrame::AddEvent( const char *szEvent, MasterFrameEventType mfEvent, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwParam3, DWORD_PTR dwParam4 )
{
  MasterFrameEvent     Event;

  Event.m_strEvent = szEvent;
  Event.m_mfEvent  = mfEvent;
  Event.m_dwParam1 = dwParam1;
  Event.m_dwParam2 = dwParam2;
  Event.m_dwParam3 = dwParam3;
  Event.m_dwParam4 = dwParam4;

  PostEvent( Event );
}



void MasterFrame::AddEvent( MasterFrameEventType mfEvent, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwParam3, DWORD_PTR dwParam4 )
{

  MasterFrameEvent     Event;

  Event.m_strEvent = "";
  Event.m_mfEvent  = mfEvent;
  Event.m_dwParam1 = dwParam1;
  Event.m_dwParam2 = dwParam2;
  Event.m_dwParam3 = dwParam3;
  Event.m_dwParam4 = dwParam4;

  PostEvent( Event );

}



void MasterFrame::SendEventNow( const char *szEvent, MasterFrameEventType mfEvent, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwParam3, DWORD_PTR dwParam4 )
{

  MasterFrameEvent     Event;

  Event.m_strEvent = szEvent;
  Event.m_mfEvent  = mfEvent;
  Event.m_dwParam1 = dwParam1;
  Event.m_dwParam2 = dwParam2;
  Event.m_dwParam3 = dwParam3;
  Event.m_dwParam4 = dwParam4;

  EventProducer<MasterFrameEvent>::SendEvent( Event );

}

void MasterFrame::SendEventNow( MasterFrameEventType mfEvent, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwParam3, DWORD_PTR dwParam4 )
{

  MasterFrameEvent     Event;

  Event.m_strEvent = "";
  Event.m_mfEvent  = mfEvent;
  Event.m_dwParam1 = dwParam1;
  Event.m_dwParam2 = dwParam2;
  Event.m_dwParam3 = dwParam3;
  Event.m_dwParam4 = dwParam4;

  EventProducer<MasterFrameEvent>::SendEvent( Event );

}



LRESULT CALLBACK MasterFrame::MasterFrameWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  if ( g_pGlobalMasterFrame )
  {
    if ( !g_pGlobalMasterFrame->m_ShutDownRequested )
    {
      g_pGlobalMasterFrame->AddEvent( "masterframe", MF_EVENT_WINDOWS_MESSAGE, message, wParam, lParam, 0 );
    }
    /*
    if ( g_pGlobalMasterFrame->m_SubclassManager.Empty() )
    {
      return (LRESULT)g_pGlobalMasterFrame->WindowProc( hWnd, message, wParam, lParam );
    }
    */
    return g_pGlobalMasterFrame->m_SubclassManager.CallChain( hWnd, message, wParam, lParam );
  }

  return ::DefWindowProc( hWnd, message, wParam, lParam );

}



BOOL MasterFrame::WindowProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{

  switch ( message )
  {
    case WM_CREATE:
      {
        RECT                rc;

        GetClientRect( hwnd, &rc );
        SetWindowPos( hwnd, HWND_TOP,
                      ( GetSystemMetrics( SM_CXSCREEN ) - ( rc.right - rc.left ) ) / 2,
                      ( GetSystemMetrics( SM_CYSCREEN ) - ( rc.bottom - rc.top ) ) / 2,
                      0, 0, SWP_NOSIZE | SWP_DRAWFRAME | SWP_FRAMECHANGED );
      }
      break;
    case WM_SETCURSOR:
      if ( ( m_Flags & MFF_HIDE_MOUSE )
      &&   ( LOWORD( lParam ) == HTCLIENT )
      &&   ( m_ApplicationActive ) )
      {
        SetCursor( NULL );
        return TRUE;
      }
      if ( LOWORD( lParam ) == HTCLIENT )
      {
        SetCursor( m_hCursor );
        return TRUE;
      }
      break;
    case WM_CHAR:
      if ( ConsoleVisible() )
      {
        if ( wParam >= 32 )
        {
          if ( m_ConsoleEntry.length() < 200 )
          {
            m_ConsoleEntry += (TCHAR)wParam;
          }
        }
        if ( wParam == VK_BACK )
        {
          if ( m_ConsoleEntry.length() > 0 )
          {
            m_ConsoleEntry.resize( m_ConsoleEntry.length() - 1 );
          }
        }
        else if ( wParam == VK_RETURN )
        {
          if ( m_ConsoleEntry.length() > 0 )
          {
            ConsolePrint( ">%s", m_ConsoleEntry.c_str() );
            ProcessCommand( m_ConsoleEntry.c_str() );
            m_LastConsoleEntry = m_ConsoleEntry;
            m_ConsoleEntry = "";
          }
        }
      }
      break;
    case WM_KEYDOWN:
      if ( ConsoleVisible() )
      {
        if ( wParam == VK_F3 )
        {
          m_ConsoleEntry = m_LastConsoleEntry;
        }
      }
      break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONUP:
      m_MouseX = LOWORD( lParam );
      m_MouseY = HIWORD( lParam );
      m_MouseButtons = (DWORD)wParam;
      break;
    case WM_ACTIVATE:
      m_ApplicationActive = (BOOL)wParam;
      break;
      //return 0;
    case WM_ERASEBKGND:
      return 1;
      /*
    case WM_CLOSE:
      CleanUp();
      break;
      */
    case WM_NCDESTROY:
      m_ShutDownRequested = true;
      //CleanUp();
      break;
    case WM_DESTROY:
      ExitInstance();
      PostQuitMessage( 0 );
      break;
  }

  return m_SubclassManager.CallNext( hwnd, message, wParam, lParam );

}



DWORD MasterFrame::LoadSound( const char* szFileName, bool bLoad3d )
{

  return DXSound::Instance().LoadWave( CWADFileSystem::Instance().OpenFile( szFileName ), bLoad3d );

}



GR::String MasterFrame::AppPath( const char *szFileName ) const
{

  GR::String   strResult = m_AppPath;

  strResult += szFileName;

  return CMisc::AppPath( szFileName );

}



void MasterFrame::LockFPS( GR::u32 FPS )
{
  if ( FPS == 0 )
  {
    m_LockFPS = false;
    return;
  }
  m_MasterRegistry.SetVarI( "lockedfps", FPS );
  m_LockFPS = true;
}



void MasterFrame::AppCursor( HCURSOR hCursor )
{
  m_hCursor = hCursor;

  POINT   pt;
  GetCursorPos( &pt );

  if ( ::SendMessage( m_Window.Hwnd, WM_NCHITTEST, 0, MAKELPARAM( pt.x, pt.y ) ) == HTCLIENT )
  {
    SetCursor( m_hCursor );
  }
}



bool MasterFrame::ProcessEvent( const Xtreme::tInputEvent& Event )
{
  AddEvent( "masterframe", MF_EVENT_INPUT_MESSAGE, Event.Type, Event.Param1, Event.Param2 );

  return false;
}


MasterFrame::DataStorage& MasterFrame::Storage()
{
  static MasterFrame::DataStorage    s_Storage;

  return s_Storage;
}



bool MasterFrame::ConfigureApp( const GR::String& strAssetFile, const GR::String& strAppPath )
{
  Storage().m_strAppPath = GR::Convert::ToStringW( strAppPath );
  if ( ( !strAppPath.empty() )
  &&   ( !GR::IO::FileUtil::CreateSubDir( UserAppDataPath() ) ) )
  {
    return false;
  }
  return true;
}



GR::String MasterFrame::UserAppDataPath( const GR::String& strAppPath )
{
  return Path::Append( Path::Append( CProgramGroups::GetShellFolder( "AppData", true ), GR::Convert::ToStringA( Storage().m_strAppPath ) ), strAppPath );
}
