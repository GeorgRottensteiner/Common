#include <GR\DeskGame\DeskGame.h>

#include <ControlHelper/CWnd.h>

#include <WinSys/RegistryHelper.h>
#include <WinSys/WinUtils.h>

#include <String/StringUtil.h>

#include <Misc/Misc.h>
#include <DateTime/Timer.h>
#include <Misc/CloneFactory.h>



CDeskGame*      g_pDeskGame = NULL;



BOOL CALLBACK DeskGameWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  if ( g_pDeskGame )
  {
    if ( !g_pDeskGame->m_Window.SubclassManager.Empty() )
    {
      return g_pDeskGame->m_Window.SubclassManager.CallChain( hwndDlg, uMsg, wParam, lParam );
    }
  }
  return (BOOL)g_pDeskGame->DeskGameWndProc( hwndDlg, uMsg, wParam, lParam );
}



BOOL CALLBACK DeskGamePageWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  return (BOOL)g_pDeskGame->DeskGamePageWndProc( hwndDlg, uMsg, wParam, lParam );
}



CDeskGame::CDeskGame() :
  m_HwndMain( NULL ),
  m_HwndPage( NULL ),
  m_MenuId( 0 ),
  m_IconId( 0 ),
  m_ExtendedClientStyles( WS_EX_CLIENTEDGE ),
  m_pPage( NULL ),
  m_Depth( 16 ),
  m_Width( 640 ),
  m_Height( 480 ),
  m_FixedSize( false ),
  m_AppPath( "" ),
  m_Caption( "DeskGame" ),
  m_BaseKey( "Endurion\\DeskGame" ),
  m_GameActive( true ),
  m_GameMinimized( false ),
  m_GameShutDown( false ),
  m_FixedLogicTimeStep( 0.05f )
{
  g_pDeskGame = this;

  m_ETChangeState = GLOBAL_QUEUE.RegisterEvent( "App.ChangeState" );
  m_ETPushState   = GLOBAL_QUEUE.RegisterEvent( "App.PushState" );
  m_ETPopState    = GLOBAL_QUEUE.RegisterEvent( "App.PopState" );

  m_ETShutDown    = GLOBAL_QUEUE.RegisterEvent( "App.ShutDown" );

  GLOBAL_QUEUE.AddListener( this );
}



CDeskGame::~CDeskGame()
{
  g_pDeskGame = NULL;
  CleanUp();

  GLOBAL_QUEUE.UnregisterEvent( m_ETChangeState );
  GLOBAL_QUEUE.UnregisterEvent( m_ETPushState );
  GLOBAL_QUEUE.UnregisterEvent( m_ETPopState );
  GLOBAL_QUEUE.UnregisterEvent( m_ETShutDown );

  GLOBAL_QUEUE.RemoveListener( this );
}



void CDeskGame::ShutDown()
{
  m_GameShutDown = true;
}



GR::String CDeskGame::AppPath( const char* szPath )
{
  if ( m_AppPath.length() == 0 )
  {
    char    szTempPath[MAX_PATH];
    GetModuleFileName( NULL, szTempPath, MAX_PATH );
    while ( szTempPath[strlen( szTempPath ) - 1] != 92 )
    {
      szTempPath[strlen( szTempPath ) - 1] = 0;
    }
    m_AppPath = szTempPath;
    if ( m_AppPath.length() >= 10)
    {
      if ( ( toupper( m_AppPath[m_AppPath.length() - 8] ) == 'R' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 7] ) == 'E' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 6] ) == 'L' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 5] ) == 'E' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 4] ) == 'A' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 3] ) == 'S' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 2] ) == 'E' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 9] ) == '\\' ) )
      {
        m_AppPath = m_AppPath.substr( 0, m_AppPath.length() - 8 );
      }
    }
    if ( m_AppPath.length() >= 8 )
    {
      if ( ( toupper( m_AppPath[m_AppPath.length() - 6] ) == 'D' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 5] ) == 'E' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 4] ) == 'B' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 3] ) == 'U' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 2] ) == 'G' )
      &&   ( toupper( m_AppPath[m_AppPath.length() - 7] ) == '\\' ) )
      {
        m_AppPath = m_AppPath.substr( 0, m_AppPath.length() - 6 );
      }
    }
  }

  GR::String   result = m_AppPath + szPath;

  return result;
}



bool CDeskGame::InitInstance()
{
  return true;
}



void CDeskGame::ExitInstance()
{
}



void CDeskGame::UpdateFrame()
{
  UpdateCurrentState();
}



void CDeskGame::DisplayFrame()
{
  if ( m_pPage )
  {
    DisplayStates( *m_pPage );
  }
}



void CDeskGame::UpdatePerDisplayFrame( const float fElapsedTime )
{
  UpdateStatesPerDisplayFrame( fElapsedTime );
  Invalidate();
}



void CDeskGame::CleanUp()
{
  std::map<DWORD,GR::Graphic::Image*>::iterator  it( m_Images.begin() );
  while ( it != m_Images.end() )
  {
    SafeDelete( it->second );

    it++;
  }
  m_Images.clear();
}



GR::Graphic::Image* CDeskGame::LoadImage( DWORD Id )
{
  tMapImages::iterator    it( m_Images.find( Id ) );
  if ( it != m_Images.end() )
  {
    return it->second;
  }

  GR::Graphic::Image*    pImageDummy = new GR::Graphic::Image( Win::Util::MemoryStreamFromResource( NULL, MAKEINTRESOURCE( Id ), "GFX" ) );
  if ( !pImageDummy->GetDepth() )
  {
    delete pImageDummy;
    return NULL;
  }

  m_Images[Id] = pImageDummy;

  return pImageDummy;
}



void CDeskGame::GetMousePos( GR::tPoint& Pos )
{
  POINT     pt;
  GetCursorPos( &pt );

  ScreenToClient( m_HwndPage, &pt );

  if ( IsPageScaled() )
  {
    pt.x = (int)( pt.x * m_FixedWidth / m_Width );
    pt.y = (int)( pt.y * m_FixedHeight / m_Height );
  }

  Pos.set( pt.x, pt.y );
}



BOOL CDeskGame::DeskGameWndProc( HWND HwndDlg, UINT Msg, WPARAM WParam, LPARAM LParam )
{
  switch ( Msg )
  {
    case WM_NCCREATE:
      break;
    case WM_CREATE:
      {

        RECT    rc;

        GetClientRect( HwndDlg, &rc );

        m_HwndMain = HwndDlg;

        SafeDelete( m_pPage );

        m_HwndPage = CreateWindowExW( m_ExtendedClientStyles,
                                      L"STATIC",
                                      L"DeskGamePage",
                                      WS_CHILD | WS_VISIBLE,
                                      rc.left, rc.top,
                                      rc.right - rc.top, rc.bottom - rc.left,
                                      m_HwndMain,
                                      NULL,
                                      NULL,
                                      0 );

        SetWindowLongPtr( m_HwndPage, GWL_STYLE, GetWindowLongPtr( m_HwndPage, GWL_STYLE ) | SS_OWNERDRAW );
        SetWindowLongPtr( m_HwndPage, GWL_WNDPROC, (LONG_PTR)::DeskGamePageWndProc );

        GetClientRect( m_HwndPage, &rc );

        m_pPage = new GR::Graphic::GDIPage();
        m_pPage->Create( m_HwndPage, rc.right - rc.left, rc.bottom - rc.top, (BYTE)m_Depth );

        SelectObject( m_pPage->GetDC(), (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

        GR::Service::Environment::Instance().SetService( "Logger", &m_Debugger );

        Xtreme::Win32Window* pWindowService = (Xtreme::Win32Window*)GR::Service::Environment::Instance().Service( "Window" );
        pWindowService->Hwnd = m_HwndMain;

        m_Window.SubclassManager.AddHandler( "DeskGame", fastdelegate::MakeDelegate( this, &CDeskGame::DeskGameWndProc ) );

        m_Input.Initialize( GR::Service::Environment::Instance() );

        m_Input.SetAlternateMousePosWindow( m_HwndPage );

        InitInstance();
      }
      break;
    case WM_SIZE:
      if ( WParam == SIZE_MINIMIZED )
      {
        m_GameMinimized = true;
      }
      else
      {
        m_GameMinimized = false;
      }
      break;
    case WM_ACTIVATE:
      if ( ( LOWORD( WParam ) == WA_ACTIVE )
      ||   ( LOWORD( WParam ) == WA_CLICKACTIVE ) )
      {
        m_GameActive = true;
      }
      else
      {
        m_GameActive = false;
      }
      break;
    case WM_ERASEBKGND:
      if ( m_pPage )
      {
        return TRUE;
      }
      break;
      /*
    case WM_PAINT:
      if ( m_pPage )
      {
        static int iLastTicks = GetTickCount();
        int iElapsedTicks = GetTickCount() - iLastTicks;
        iLastTicks = GetTickCount();

        DisplayFrame();

        RECT    rc;

        GetClientRect( hwndDlg, &rc );

        PAINTSTRUCT    ps;

        //HDC  hdc = BeginPaint( m_HwndDlg, &ps );
        HDC  hdc = BeginPaint( m_HwndPage, &ps );
        BitBlt( hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                m_pPage->GetDC(), rc.left, rc.top, SRCCOPY );
        EndPaint( m_HwndPage, &ps );
        //EndPaint( hwndDlg, &ps );
        return TRUE;
      }
      break;
      */
    case WM_COMMAND:
      if ( HIWORD( WParam ) == 0 )
      {
        // das kam vom Menü
      }
      break;
    case WM_NCDESTROY:
      ShutDown();
      break;
    case WM_DESTROY:
      ShutDown();
      break;
  }
  if ( IsWindow( HwndDlg ) )
  {
    return DefWindowProc( HwndDlg, Msg, WParam, LParam );
  }
  return 0;
}



bool CDeskGame::IsPageScaled()
{
  RECT    rc;

  GetClientRect( m_HwndPage, &rc );

  if ( ( m_pPage->GetWidth() == ( rc.right - rc.left ) )
  &&   ( m_pPage->GetHeight() == ( rc.bottom - rc.top ) ) )
  {
    return false;
  }
  return true;
}



LRESULT CALLBACK CDeskGame::DeskGamePageWndProc( HWND HwndDlg, UINT Msg, WPARAM WParam, LPARAM LParam )
{
  switch ( Msg )
  {
    case WM_ERASEBKGND:
      if ( m_pPage )
      {
        return TRUE;
      }
      break;
    case WM_PAINT:
      {
        if ( m_pPage )
        {
          DisplayFrame();

          RECT    rc;

          GetClientRect( HwndDlg, &rc );

          PAINTSTRUCT    ps;

          HDC  hdc = BeginPaint( m_HwndPage, &ps );
          if ( m_pPage )
          {
            if ( !IsPageScaled() )
            {
              BitBlt( hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                      m_pPage->GetDC(), rc.left, rc.top, SRCCOPY );
            }
            else
            {
              SetStretchBltMode( hdc, HALFTONE );
              StretchBlt( hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                          m_pPage->GetDC(), rc.left, rc.top, m_pPage->GetWidth(), m_pPage->GetHeight(), SRCCOPY );
            }
          }
          EndPaint( m_HwndPage, &ps );
          return TRUE;
        }
      }
      break;
  }
  if ( IsWindow( HwndDlg ) )
  {
    return DefWindowProc( HwndDlg, Msg, WParam, LParam );
  }
  return 0;
}



void CDeskGame::SetWindowValues( const char* Caption, DWORD Icon, DWORD MenuId, DWORD ExtendedClientStyles )
{
  m_MenuId    = MenuId;
  m_IconId    = Icon;
  m_Caption   = Caption;
  if ( m_HwndMain )
  {
    SendMessageA( m_HwndMain, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon( NULL, MAKEINTRESOURCE( Icon ) ) );
    SendMessageA( m_HwndMain, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon( NULL, MAKEINTRESOURCE( Icon ) ) );

    SetMenu( m_HwndMain, LoadMenu( NULL, MAKEINTRESOURCE( m_MenuId ) ) );
    SetWindowText( m_HwndMain, Caption );
  }
  m_ExtendedClientStyles = ExtendedClientStyles;
}



void CDeskGame::SetBaseKey( const char* BaseKey )
{
  m_BaseKey = BaseKey;
}



void CDeskGame::SetSetting( const char* Key, const char* Value )
{
  Registry::SetKey( HKEY_CURRENT_USER, CMisc::printf( "Software\\%s", m_BaseKey.c_str() ), Key, Value );
}



GR::String CDeskGame::GetSetting( const char* Key )
{
  GR::String      value;

  Registry::GetKey( HKEY_CURRENT_USER, CMisc::printf( "Software\\%s", m_BaseKey.c_str() ), Key, value );

  return value;
}



void CDeskGame::SetSettingI( const char* Key, int Value )
{
  char    value[MAX_PATH];

  wsprintf( value, "%d", Value );

  Registry::SetKey( HKEY_CURRENT_USER, CMisc::printf( "Software\\%s", m_BaseKey.c_str() ), Key, value );
}



int CDeskGame::GetSettingI( const char* Key, int Default )
{
  GR::String      value;

  if ( Registry::GetKey( HKEY_CURRENT_USER, CMisc::printf( "Software\\%s", m_BaseKey.c_str() ), Key, value ) )
  {
    return GR::Convert::ToI32( value );
  }
  return Default;
}



void CDeskGame::SetSize( int Width, int Height, DWORD Depth, bool FixedSize )
{
  m_Width     = Width;
  m_Height    = Height;
  m_Depth     = Depth;
  m_FixedSize = FixedSize;

  m_FixedWidth  = Width;
  m_FixedHeight = Height;
}



void CDeskGame::Invalidate( RECT* pRect )
{
  InvalidateRect( m_HwndPage, pRect, FALSE );
}



void CDeskGame::EnableMenuItem( DWORD Id )
{
  ::EnableMenuItem( GetMenu( m_HwndMain ), Id, MF_ENABLED );
}



void CDeskGame::DisableMenuItem( DWORD Id )
{
  ::EnableMenuItem( GetMenu( m_HwndMain ), Id, MF_GRAYED );
}



void CDeskGame::AddEvent( DWORD EventId, DWORD EventValue, GR::String Value )
{
  PostEvent( tEvent( EventId, EventValue, Value ) );
}



bool CDeskGame::ProcessEvent( const tEvent& Event )
{
  return false;
}



void CDeskGame::WinPrint( int X, int Y, const char* Text, DWORD Color )
{
  if ( m_pPage == NULL )
  {
    return;
  }
  SetTextColor( m_pPage->GetDC(), RGB( ( Color & 0xff0000 ) >> 16, ( Color & 0xff00 ) >> 8, Color & 0xff ) );
  SetBkMode( m_pPage->GetDC(), TRANSPARENT );
  TextOut( m_pPage->GetDC(), X, Y, Text, (int)strlen( Text ) );
}



void CDeskGame::SetClientSize( int Width, int Height )
{
  if ( !m_FixedSize )
  {
    SafeDelete( m_pPage );
  }

  m_Width   = Width;
  m_Height  = Height;

  RECT    rcClient;

  rcClient.left   = 0;
  rcClient.top    = 0;
  rcClient.right  = Width;
  rcClient.bottom = Height;
  AdjustWindowRectEx( &rcClient, GetWindowLong( m_HwndPage, GWL_STYLE ), FALSE, GetWindowLong( m_HwndPage, GWL_EXSTYLE ) );

  RECT    rcMainClient;

  rcMainClient.left = 0;
  rcMainClient.top = 0;
  rcMainClient.right = rcClient.right - rcClient.left;
  rcMainClient.bottom = rcClient.bottom - rcClient.top;
  AdjustWindowRectEx( &rcMainClient, GetWindowLong( m_HwndMain, GWL_STYLE ), !!GetMenu( m_HwndMain ), GetWindowLong( m_HwndMain, GWL_EXSTYLE ) );

  SetWindowPos( m_HwndMain, 0,
                ( GetSystemMetrics( SM_CXSCREEN ) - ( rcMainClient.right - rcMainClient.left ) ) / 2,
                ( GetSystemMetrics( SM_CYSCREEN ) - ( rcMainClient.bottom - rcMainClient.top ) ) / 2,
                rcMainClient.right - rcMainClient.left, rcMainClient.bottom - rcMainClient.top, SWP_NOZORDER );

  SetWindowPos( m_HwndPage, 0,
                0, 0,
                rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_NOMOVE | SWP_NOZORDER );


  if ( !m_FixedSize )
  {
    m_pPage = new GR::Graphic::GDIPage();
    m_pPage->Create( m_HwndPage, Width, Height, (BYTE)m_Depth );
  }

  SelectObject( m_pPage->GetDC(), (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );
}



int CDeskGame::Run()
{
  MSG           msg;

  WNDCLASS      WndClass;


  WndClass.style          = CS_HREDRAW | CS_VREDRAW;
  WndClass.lpfnWndProc    = (WNDPROC)::DeskGameWndProc;
  WndClass.cbClsExtra     = 0;
  WndClass.cbWndExtra     = 0;
  WndClass.hInstance      = (HINSTANCE)GetModuleHandle( NULL );
  WndClass.hIcon          = LoadIcon( (HINSTANCE)GetModuleHandle( NULL ), MAKEINTRESOURCE( m_IconId ) );
  WndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );
  WndClass.hbrBackground  = (HBRUSH)GetStockObject( WHITE_BRUSH );
  WndClass.lpszMenuName   = MAKEINTRESOURCE( m_MenuId );
  WndClass.lpszClassName  = "DeskGame";
  if ( !RegisterClass( &WndClass ) )
  {
    return -1;
  }

  RECT    rc;
  SetRect( &rc, 0, 0, m_Width, m_Height );
  AdjustWindowRectEx( &rc, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, NULL, m_ExtendedClientStyles );

  WORD    wTrueWidth  = (WORD)( rc.right - rc.left );
  WORD    wTrueHeight = (WORD)( rc.bottom - rc.top );

  if ( m_MenuId )
  {
    wTrueHeight += GetSystemMetrics( SM_CYMENU );
  }

  m_HwndMain = CreateWindowW( GR::Convert::ToUTF16( WndClass.lpszClassName ).c_str(),
                              GR::Convert::ToUTF16( m_Caption ).c_str(),
                              WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              ( GetSystemMetrics( SM_CXSCREEN ) - wTrueWidth ) / 2,
                              ( GetSystemMetrics( SM_CYSCREEN ) - wTrueHeight ) / 2,
                              wTrueWidth,
                              wTrueHeight,
                              NULL,
                              NULL,
                              (HINSTANCE)GetModuleHandle( NULL ),
                              NULL );
  if ( !m_HwndMain )
  {
    return -1;
  }
  ShowWindow( m_HwndMain, SW_SHOW );
  UpdateWindow( m_HwndMain );

  m_LastFrameTime      = Time::Timer::Time();
  m_LastFixedFrameTime = m_LastFrameTime;

  while ( m_HwndMain )
  {
    if ( !m_GameActive )
    {
      static DWORD    lastUpdate = GetTickCount();

      if ( !GetMessage( &msg, NULL, 0, 0 ) )
      {
        SendEvent( m_ETShutDown );
        ExitInstance();
        m_Input.Release();
        CleanUp();
        if ( m_pPage )
        {
          m_pPage->Destroy();
          SafeDelete( m_pPage );
        }
        GR::Service::Environment::Instance().RemoveService( "Logger" );
        GR::Service::Environment::Instance().RemoveService( "SubclassManager" );
        m_HwndMain = NULL;
        break;
      }
      if ( !WindowsWrapper::CWnd::WalkPreTranslateTree( NULL, &msg ) )
      {
        if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
        {
          continue;
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );

        if ( GetTickCount() > lastUpdate + 1000 )
        {
          InvalidateRect( m_HwndMain, NULL, FALSE );
          lastUpdate = GetTickCount();
        }
      }
    }
    else
    {
      while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
      {
        BOOL    bRet = GetMessage( &msg, NULL, 0, 0 );

        if ( bRet == 0 )
        {
          SendEvent( m_ETShutDown );
          m_HwndMain = NULL;
          break;
        }
        else if ( bRet == -1 )
        {
          // Fehler!
          m_HwndMain = NULL;
          break;
        }
        else
        {
          if ( !WindowsWrapper::CWnd::WalkPreTranslateTree( NULL, &msg ) )
          {
  	        ::TranslateMessage( &msg );
	          ::DispatchMessage( &msg );
          }
        }
      }
      GR::f64   newFrameTime      = Time::Timer::Time();
      GR::f64   elapsedTime       = newFrameTime - m_LastFrameTime;
      GR::f64   elapsedFixedTime  = newFrameTime - m_LastFixedFrameTime;

      int     safetyLoopCounter = 0;
      while ( elapsedFixedTime >= m_FixedLogicTimeStep )
      {
        ++safetyLoopCounter;
        if ( safetyLoopCounter > 20 )
        {
          // Safety-Break um eventuelle Endlosloops zu verhindern
          m_LastFixedFrameTime = newFrameTime;
          break;
        }
        UpdateFrame();
        ProcessEventQueue();
        GLOBAL_QUEUE.ProcessQueue();
        elapsedFixedTime -= m_FixedLogicTimeStep;
        m_LastFixedFrameTime += m_FixedLogicTimeStep;
      }
      m_LastFrameTime = newFrameTime;

      m_Input.Update( (float)elapsedTime );

      UpdateStatesPerDisplayFrame( (float)elapsedTime );
      UpdatePerDisplayFrame( (float)elapsedTime );
      ProcessEventQueue();
      GLOBAL_QUEUE.ProcessQueue();

      if ( m_pPage )
      {
        DisplayFrame();
      }

      if ( m_GameShutDown )
      {
        SendEvent( m_ETShutDown );
        ExitInstance();
        m_Input.Release();
        CleanUp();
        if ( m_pPage )
        {
          m_pPage->Destroy();
          SafeDelete( m_pPage );
        }
        GR::Service::Environment::Instance().RemoveService( "Logger" );
        GR::Service::Environment::Instance().RemoveService( "SubclassManager" );

        if ( m_HwndMain )
        {
          DestroyWindow( m_HwndMain );
          m_HwndMain = NULL;
        }
        PostQuitMessage( 0 );
      }
    }
  }

  return (int)msg.wParam;

  while ( 1 )
  {
    if ( ( !m_GameMinimized )
    &&   ( m_GameActive ) )
    {
      if ( !WindowsWrapper::CWnd::PumpMessage() )
      {
        SendEvent( m_ETShutDown );
        break;
      }
      Sleep( 10 );

      m_Input.Poll();

      UpdateFrame();
      ProcessEventQueue();
      if ( m_GameShutDown )
      {
        if ( m_HwndMain )
        {
          DestroyWindow( m_HwndMain );
        }
      }
    }
    else
    {
      while ( GetMessage( &msg, NULL, 0, 0 ) )
      {
        if ( msg.message == WM_QUIT )
        {
          break;
        }
        if ( !WindowsWrapper::CWnd::WalkPreTranslateTree( NULL, &msg ) )
        {
          if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
          {
            continue;
          }

          TranslateMessage( &msg );
          DispatchMessage( &msg );
        }
        if ( m_GameActive )
        {
          break;
        }
        if ( m_GameShutDown )
        {
          if ( m_HwndMain )
          {
            DestroyWindow( m_HwndMain );
          }
        }
      }
      if ( !m_GameActive )
      {
        SendEvent( m_ETShutDown );
        break;
      }
    }
  }

  return (int)msg.wParam;
}



void CDeskGame::ProcessEvent( const tGlobalEvent& Event )
{
  if ( Event.m_Type == m_ETChangeState )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      // übergebene Parameter
      GR::Strings::ParameterList        paramList;

      GR::String           strParam = Event.m_Param;

      size_t    iParamPos = Event.m_Param.find( '(' );
      if ( iParamPos != GR::String::npos )
      {
        size_t    iParamEndPos = Event.m_Param.find( ')', iParamPos );

        if ( ( iParamEndPos != GR::String::npos )
        &&   ( iParamEndPos > iParamPos ) )
        {
          std::list<GR::String>    listParams;

          GR::Strings::Split( Event.m_Param.substr( iParamPos + 1, iParamEndPos - iParamPos - 1 ), ',', listParams );

          std::list<GR::String>::iterator    it( listParams.begin() );
          while ( it != listParams.end() )
          {
            GR::String    strParam( GR::Strings::Trim( *it ) );

            size_t    iEqualPos = strParam.find( '=' );
            if ( iEqualPos != GR::String::npos )
            {
              paramList.SetParam( strParam.substr( 0, iEqualPos ), strParam.substr( iEqualPos + 1 ) );
            }

            ++it;
          }
        }
        strParam = Event.m_Param.substr( 0, iParamPos );
      }

      ICloneAble*               pClone = CloneFactory::Instance().CreateObject( strParam );
      IGameState<GR::Graphic::GFXPage>*    pGameState = static_cast<IGameState<GR::Graphic::GFXPage>*>( pClone );
      if ( pGameState )
      {
        pGameState->m_ParamInit = paramList;
        ChangeState( pGameState );
      }
      else
      {
        dh::Warning( "XFrameApp: Trying to change to unregistered state %s", strParam.c_str() );
      }
    }
    else
    {
      ChangeState( (IGameState<GR::Graphic::GFXPage>*)Event.m_Param1 );
    }
  }
  else if ( Event.m_Type == m_ETPopState )
  {
    PopState();
  }
  else if ( Event.m_Type == m_ETPushState )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      ICloneAble*   pClone = CloneFactory::Instance().CreateObject( Event.m_Param );
      IGameState<GR::Graphic::GFXPage>*    pGameState = static_cast<IGameState<GR::Graphic::GFXPage>*>( pClone );
      if ( pGameState )
      {
        PushState( pGameState );
      }
      else
      {
        dh::Warning( "MasterFrame2d: Trying to push unregistered state %s", Event.m_Param.c_str() );
      }
    }
    else
    {
      PushState( (IGameState<GR::Graphic::GFXPage>*)Event.m_Param1 );
    }
  }
  else if ( Event.m_Type == m_ETShutDown )
  {
    ShutDown();
    SendEvent( m_ETShutDown );
  }
}



Xtreme::XInput* CDeskGame::InputClass()
{
  return &m_Input;
}

