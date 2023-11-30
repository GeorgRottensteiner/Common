#include <MasterFrame/MasterFrame2d.h>
#include <Misc/Misc.h>
#include <DateTime\Timer.h>

#include <Grafik/GDIPage.h>
#include <Grafik/DDrawPage.h>

#include <String/StringUtil.h>

#include <Misc/CloneFactory.h>

#include <Xtreme/Input/DXInput.h>

#include <Controls/InputEvent.h>

#include <Interface/ICloneAble.h>

#include <debug/debugclient.h>



MasterFrame2d   *pGlobalDXMasterFrame = NULL;



MasterFrame2d::MasterFrame2d() : 
  MasterFrame(),
  m_pConsoleFont( NULL ),
  m_pDirectXPage( NULL ),
  m_pWinGPage( NULL ),
  m_SaveSnapShot( false ),
  m_UseCustomMouseCursor( false ),
  m_CurrentDepth( 0 ),
  m_OrigDepth( 0 )
{
  pGlobalDXMasterFrame = this;

  m_imagesT = RegisterSymbol( "images" );

  m_ETChangeState   = EventQueue::Instance().RegisterEvent( "App.ChangeState" );
  m_ETPushState     = EventQueue::Instance().RegisterEvent( "App.PushState" );
  m_ETPopState      = EventQueue::Instance().RegisterEvent( "App.PopState" );
  m_ETShutDown      = EventQueue::Instance().RegisterEvent( "App.ShutDown" );
  m_ETToggleWindow  = EventQueue::Instance().RegisterEvent( "App.ToggleWindow" );

  GR::Service::Environment::Instance().SetService( "Application", this );

  EventQueue::Instance().AddListener( this );
}



MasterFrame2d::~MasterFrame2d()
{
  EventQueue::Instance().UnregisterEvent( m_ETChangeState );
  EventQueue::Instance().UnregisterEvent( m_ETPushState );
  EventQueue::Instance().UnregisterEvent( m_ETPopState );
  EventQueue::Instance().UnregisterEvent( m_ETShutDown );
  EventQueue::Instance().UnregisterEvent( m_ETToggleWindow );
}



void MasterFrame2d::Reset()
{
  m_pWinGPage           = NULL;
  m_pDirectXPage        = NULL;

  MasterFrame::Reset();
}



void MasterFrame2d::ResetFont()
{
  GR::Graphic::GDIPage*          pPage;

  GR::Graphic::Image* pLetter;

  HFONT               hOldFont,
                      hTempFont;

  int                 i;

  char                szDummy[5];

  SIZE                stSize;


  m_pConsoleFont = new GR::Font();
  pPage = new GR::Graphic::GDIPage();
  pPage->Create( m_Window.Hwnd, 640, 480, GetPage()->GetDepth() );

  hTempFont = CreateFont( 12, 0, 0, 0, 700, (BYTE)FALSE, (BYTE)FALSE, (BYTE)FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_DONTCARE, "Arial" );


  hOldFont = (HFONT)SelectObject( pPage->GetDC(), hTempFont );


  szDummy[1] = 0;
  for ( i = 1; i < 256; i++ )
  {
    szDummy[0] = i;
    SetBkMode( pPage->GetDC(), OPAQUE );
    SetTextColor( pPage->GetDC(), RGB( 20, 20, 20 ) );
    SetBkColor( pPage->GetDC(), RGB( 0, 0, 0 ) );

    TextOut( pPage->GetDC(), 0, 0, szDummy, 1 );
    TextOut( pPage->GetDC(), 1, 1, szDummy, 1 );

    SetBkMode( pPage->GetDC(), TRANSPARENT );
    SetTextColor( pPage->GetDC(), RGB( 255, 255, 255 ) );

    TextOut( pPage->GetDC(), 0, 0, szDummy, 1 );

    GetTextExtentPoint( pPage->GetDC(), szDummy, 1, &stSize );
    pLetter = new GR::Graphic::Image( (WORD)stSize.cx, (WORD)stSize.cy, pPage->GetDepth(), 0, 0 );
    pLetter->GetImage( pPage, 0, 0 );
    m_pConsoleFont->SetLetter( i, pLetter );
  }
  SelectObject( pPage->GetDC(), hOldFont );

  DeleteObject( hTempFont );

  delete pPage;
}



bool MasterFrame2d::CreatePage()
{
  bool      result = false;

  m_CurrentDepth = m_OrigDepth;
  if ( m_FullScreen )
  {
    int       iWidth = (int)m_MasterRegistry.GetVarI( "fs_resx" ),
              iHeight = (int)m_MasterRegistry.GetVarI( "fs_resy" );

    if ( iWidth == -1 )
    {
      // BAUSTELLE - Option für Fullscreen-Auflösung
      m_MasterRegistry.SetVarI( "fs_resx", 640 );
      m_MasterRegistry.SetVarI( "fs_resy", 480 );
    }
    SetWindowLong( m_Window.Hwnd, GWL_STYLE, GetWindowLong( m_Window.Hwnd, GWL_STYLE ) & ~( WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU ) );
    m_pDirectXPage = new GR::Graphic::DirectXPage();
    result = m_pDirectXPage->Create( m_Window.Hwnd, (int)m_MasterRegistry.GetVarI( "fs_resx" ), (int)m_MasterRegistry.GetVarI( "fs_resy" ), m_CurrentDepth );
    if ( !result )
    {
      if ( ( m_CurrentDepth == 24 )
      ||   ( m_CurrentDepth == 32 ) )
      {
        // es gibt Karten, die können nur 24, oder nur 32
        m_CurrentDepth = 56 - m_CurrentDepth;
        result = m_pDirectXPage->Create( m_Window.Hwnd, (int)m_MasterRegistry.GetVarI( "fs_resx" ), (int)m_MasterRegistry.GetVarI( "fs_resy" ), m_CurrentDepth );
        if ( !result )
        {
          m_CurrentDepth = 56 - m_CurrentDepth;
        }
      }
    }
    if ( !result )
    {
      m_FullScreen = false;
      // versuchen, wieder in Fenstermodus zu gehen
    }
    else
    {
      SetVarI( "windowed_resx", m_MasterRegistry.GetVarI( "resx" ) );
      SetVarI( "windowed_resy", m_MasterRegistry.GetVarI( "resy" ) );
      SetVarI( "resx", m_MasterRegistry.GetVarI( "fs_resx" ) );
      SetVarI( "resy", m_MasterRegistry.GetVarI( "fs_resy" ) );
    }
  }
  if ( !m_FullScreen )
  {
    m_MasterRegistry.SetVarI( "resx", m_MasterRegistry.GetVarI( "windowed_resx" ) );
    m_MasterRegistry.SetVarI( "resy", m_MasterRegistry.GetVarI( "windowed_resy" ) );

    m_pWinGPage = new GR::Graphic::GDIPage();

    result = m_pWinGPage->Create( m_Window.Hwnd, (int)m_MasterRegistry.GetVarI( "resx" ), (int)m_MasterRegistry.GetVarI( "resy" ), m_CurrentDepth );
    if ( m_Flags & MFF_CAPTION )
    {
      SetWindowLong( m_Window.Hwnd, GWL_STYLE, GetWindowLong( m_Window.Hwnd, GWL_STYLE ) | WS_POPUP | WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX );
    }
    else
    {
      SetWindowLong( m_Window.Hwnd, GWL_STYLE, GetWindowLong( m_Window.Hwnd, GWL_STYLE ) | WS_POPUP | WS_MINIMIZEBOX );
    }
    SetWindowPos( m_Window.Hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_DRAWFRAME );
  }

  if ( m_pConsoleFont == NULL )
  {
    ResetFont();
  }
  return result;
}



void MasterFrame2d::DestroyPage()
{
  if ( m_pWinGPage != NULL )
  {
    delete m_pWinGPage;
    m_pWinGPage = NULL;
  }
  if ( m_pDirectXPage != NULL )
  {
    delete m_pDirectXPage;
    m_pDirectXPage = NULL;
  }
}



bool MasterFrame2d::CleanUp()
{
  SafeDelete( m_pConsoleFont );

  DestroyPage();

  return MasterFrame::CleanUp();
}



void MasterFrame2d::SetPalette( GR::Graphic::Palette *pPalette )
{
  m_Palette = *pPalette;
  if ( m_pWinGPage != NULL )
  {
    m_pWinGPage->SetPalette( &m_Palette );
  }
  if ( m_pDirectXPage != NULL )
  {
    m_pDirectXPage->SetPalette( &m_Palette );
  }
}



bool MasterFrame2d::Create( int NewWidth, int NewHeight, unsigned char Depth, GR::u32 Flags )
{
  DestroyPage();

  m_OrigDepth = Depth;
  if ( !MasterFrame::Create( NewWidth, NewHeight, Depth, Flags ) )
  {
    return false;
  }

  m_Window.SubclassManager.AddHandler( "MasterFrame2d", fastdelegate::MakeDelegate( this, &MasterFrame2d::WindowProc2d ) );

  if ( !CreatePage() )
  {
    CloseWindow( m_Window.Hwnd );
    DestroyWindow( m_Window.Hwnd );
    m_Window.Hwnd = NULL;
    return false;
  }
  return true;
}



bool MasterFrame2d::ToggleFullScreen()
{
  bool      result = false;


  DestroyPage();

  m_FullScreen = !m_FullScreen;
  if ( m_FullScreen )
  {
    ZeroMemory( &m_wplMain, sizeof( m_wplMain ) );
    m_wplMain.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( m_Window.Hwnd, &m_wplMain );
  }
  result = CreatePage();
  if ( m_CurrentDepth <= 8 )
  {
    if ( m_pWinGPage != NULL )
    {
      m_pWinGPage->SetPalette( &m_Palette );
    }
    if ( m_pDirectXPage != NULL )
    {
      m_pDirectXPage->SetPalette( &m_Palette );
    }
  }

  if ( !m_FullScreen )
  {
    SetWindowPlacement( m_Window.Hwnd, &m_wplMain );
  }

  SendEventNow( "masterframe", MF_EVENT_SCREENMODE_CHANGED, GetPage()->GetDepth(), 0, 0, 0 );
  return result;
}



void MasterFrame2d::DisplayConsole()
{
  if ( m_pConsoleFont == NULL )
  {
    return;
  }


  int             iPos = 4;

  std::list<GR::String>::iterator    it( m_listConsoleLines.begin() );
  while ( it != m_listConsoleLines.end() )
  {
    GR::String&    strLine = *it;

    m_pConsoleFont->PrintFont( GetPage(), 4, iPos, strLine.c_str() );
    iPos += 15;

    if ( iPos >= m_MasterRegistry.GetVarI( "resy" ) - 30 )
    {
      break;
    }

    ++it;
  }

  if ( ( GetTickCount() % 1024 ) > 512 )
  {
    m_pConsoleFont->PrintFont( GetPage(), 4, (int)m_MasterRegistry.GetVarI( "resy" ) - 15, ( ">" + m_ConsoleEntry + "_" ).c_str(), IMAGE_METHOD_TRANSPARENT, 0 );
  }
  else
  {
    m_pConsoleFont->PrintFont( GetPage(), 4, (int)m_MasterRegistry.GetVarI( "resy" ) - 15, ( ">" + m_ConsoleEntry ).c_str(), IMAGE_METHOD_TRANSPARENT, 0 );
  }
}



void MasterFrame2d::Print( int X, int Y, const char* Text )
{
  m_pConsoleFont->PrintFont( GetPage(), X, Y, Text, IMAGE_METHOD_TRANSPARENT );
}



void MasterFrame2d::Print( int X, int Y, const GR::String& Text )
{
  m_pConsoleFont->PrintFont( GetPage(), X, Y, Text, IMAGE_METHOD_TRANSPARENT );
}



void MasterFrame2d::ProcessCommandTokens( GR::Tokenizer::TokenSequence& m_TokenSequence, GR::String& strCommand )
{
  GR::Tokenizer::TokenSequence::iterator       m_itTokenPos;

  m_itTokenPos = m_TokenSequence.begin();

  if ( m_itTokenPos == m_TokenSequence.end() )
  {
    return;
  }
  GR::Tokenizer::Token currentToken = *m_itTokenPos;

  if ( currentToken.Type() == m_infoT )
  {
    m_itTokenPos++;
    if ( m_itTokenPos->Type() == m_imagesT )
    {
      ConsolePrint( "Page Depth %d", GetPage()->GetDepth() );
      return;
    }
  }
  else if ( currentToken.Type() == m_imagesT )
  {
    // alle geladenen Bilder dumpen
    DWORD       dwCount;
    dwCount = 0;
    PJ::generic_map_it it( m_ImageManager._GetMap().begin() );
    for( ; it !=  m_ImageManager._GetMap().end(); ++it )
    {
      dwCount++;
      if ( it->second != NULL )
      {
        ConsolePrint( "%s = %d (%dx%dx%d)",
                 it->first.c_str(),
                 it->second,
                 ( (GR::Graphic::Image*)it->second )->GetWidth(),
                 ( (GR::Graphic::Image*)it->second )->GetHeight(),
                 ( (GR::Graphic::Image*)it->second )->GetDepth() );
      }
      else
      {
        ConsolePrint( "%s = NULL", it->first.c_str() );
      }
    }
    ConsolePrint( "%d images loaded", dwCount );
    return;
  }
  MasterFrame::ProcessCommandTokens( m_TokenSequence, strCommand );
}



GR::Graphic::GFXPage* MasterFrame2d::GetPage()
{
  if ( m_FullScreen )
  {
    return m_pDirectXPage;
  }
  return m_pWinGPage;
}



GR::Graphic::Image *MasterFrame2d::LoadImage( const char *szName )
{
  GR::Graphic::Image*    pNewImage;


  pNewImage = m_ImageManager.Recall( szName );
  if ( pNewImage == NULL )
  {
    // das Bild gibt es noch nicht
    pNewImage = new GR::Graphic::Image( CWADFileSystem::Instance().OpenFile( szName ) );
    if ( pNewImage->GetDepth() == 0 )
    {
      dh::Log( "MasterFrame2d::LoadImage  Failed to load Image %s\n", szName );
      return NULL;
    }
    m_ImageManager.Store( szName, pNewImage );
  }
  return pNewImage;
}



void MasterFrame2d::AddImage( const char *szName, GR::Graphic::Image *pImage )
{
  m_ImageManager.Store( szName, pImage );
}



void MasterFrame2d::DeleteImage( const char *szName )
{
  m_ImageManager.Erase( szName );
}



BOOL MasterFrame2d::WindowProc2d( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message )
  {
    case WM_PAINT:
      if ( !m_FullScreen )
      {
        if ( m_pWinGPage != NULL )
        {
          //m_pWinGPage->GFXUpdate( FALSE, FALSE, NULL, (HDC)wParam, FALSE );
          m_pWinGPage->GFXUpdate( TRUE, FALSE, NULL, (HDC)wParam, FALSE );
        }
        return 0;
      }
      break;
    case WM_CLOSE:
      DestroyPage();
      break;
    case WM_SIZE:
      if ( !m_FullScreen )
      {
        if ( m_Flags & MFF_RESIZEABLE )
        {
          DestroyPage();
          SetVarI( "resx", LOWORD( lParam ) );
          SetVarI( "resy", HIWORD( lParam ) );
          SetVarI( "windowed_resx", LOWORD( lParam ) );
          SetVarI( "windowed_resy", HIWORD( lParam ) );
          CreatePage();
          if ( m_CurrentDepth <= 8 )
          {
            if ( m_pWinGPage != NULL )
            {
              m_pWinGPage->SetPalette( &m_Palette );
            }
            if ( m_pDirectXPage != NULL )
            {
              m_pDirectXPage->SetPalette( &m_Palette );
            }
          }
          SendEventNow( "masterframe", MF_EVENT_SCREENMODE_CHANGED, 0, 0, 0, 0 );
        }
      }
      break;
  }

  return m_Window.SubclassManager.CallNext( hwnd, message, wParam, lParam );
}



void MasterFrame2d::SaveSnapShot( const char *szFile )
{
  m_SaveSnapShot   = true;
  m_SnapShotFile = szFile;
}



void MasterFrame2d::DoSaveSnapShot( const char* szFile )
{
  GR::Graphic::GFXPage*   pPage = GetPage();

  GR::Graphic::Image      imgSnapShot( pPage->GetWidth(), pPage->GetHeight(), pPage->GetDepth() );

  imgSnapShot.GetImage( pPage, 0, 0 );

  GR::Graphic::Palette    myPal;

  pPage->GetPalette( &myPal );
  imgSnapShot.Save( szFile, FALSE, &myPal );
}



int MasterFrame2d::Run()
{
  CleanUp();
  SetInstance( (HINSTANCE)GetModuleHandle( NULL ) );

  // BAUSTELLE CommandLine
  //SetCommandLine( lpCmdLine );

  AddListener( this );

  if ( !InitInstance() )
  {
    CleanUp();
    return 1;
  }

  MSG           msg;

  while ( !m_ShutDownRequested )
  {
    if ( !m_ApplicationActive )
    {
      if ( !GetMessage( &msg, NULL, 0, 0 ) )
      {
        break;
      }
      if ( msg.hwnd )
      {
        if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
        {
          continue;
        }
      }
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
    else
    {
      while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
      {
        if ( ( !GetMessage( &msg, NULL, 0, 0 ) )
        ||   ( m_ShutDownRequested ) )
        {
          break;
        }
        if ( msg.hwnd )
        {
          if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
          {
            continue;
          }
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
      }
      {
        // Konsole an/aus
        if ( m_pInput )
        {
          if ( m_pInput->ReleasedKeyPressed( Xtreme::KEY_F11 ) )
          {
            ToggleConsole();
          }
          if ( m_pInput->ReleasedKeyPressed( Xtreme::KEY_F3 ) )
          {
            SaveSnapShot( CMisc::AppPath( "shot.igf" ).c_str() );
          }
          // Full-Screen-Toggle
          if ( m_pInput->AltPressed() )
          {
            // ALT
            if ( ( m_pInput->ReleasedKeyPressed( Xtreme::KEY_ENTER ) )
            ||   ( m_pInput->ReleasedKeyPressed( Xtreme::KEY_NUMPAD_ENTER ) ) )
            {
              // Alt-Enter
              ToggleFullScreen();
            }
          }
        }

        GR::f64   ElapsedTime = Time::Timer::Time( Time::Timer::TF_GETELAPSEDTIME );
        if ( ElapsedTime > 10.0f )
        {
          ElapsedTime = 10.0f;
        }

        if ( ( !m_ShutDownRequested )
        &&   ( !IsPaused() ) )
        {
          if ( IsTimed() )
          {
            DWORD     dwGnu = 0;

            if ( GetTickCount() - GetLastFrameTime() > GetFrameTime() )
            {
              while ( GetTickCount() - GetLastFrameTime() > GetFrameTime() )
              {
                dwGnu++;
                if ( dwGnu > (DWORD)m_MasterRegistry.GetVarI( "maxupdates" ) )
                {
                  // Safety-Break um eventuelle Endlosloops zu verhindern
                  SetLastFrameTime( (float)GetTickCount() );
                  break;
                }
                ProcessEventQueue();
                EventQueue::Instance().ProcessQueue();

                m_pInput->Update( (GR::f32)GetFrameTime() );
                UpdateFrame();

                SetLastFrameTime( GetLastFrameTime() + GetFrameTime() );
                if ( m_ShutDownRequested )
                {
                  break;
                }
              }
            }

          }
          else
          {
            ProcessEventQueue();
            EventQueue::Instance().ProcessQueue();

            m_pInput->Update( (GR::f32)ElapsedTime );
            UpdateFrame();
          }
        }
        else
        {
          ProcessEventQueue();
          m_pInput->Update( (GR::f32)ElapsedTime );
        }
        if ( m_ShutDownRequested )
        {
          continue;
        }

        /*
        if ( m_LockFPS )
        {
          if ( ElapsedTime < 1.0f / GetLockedFPS() )
          {
            Sleep( (DWORD)( 1000 / GetLockedFPS() - ElapsedTime * 1000 ) );
            continue;
          }
        }*/

        bool    canDraw = true;

        if ( IsFullScreen() )
        {
          if ( m_pDirectXPage != NULL )
          {
            canDraw = m_pDirectXPage->GFXBegin();
          }
        }
        if ( ( canDraw )
        &&   ( GetPage() != NULL ) )
        {
          if ( m_ApplicationActive )
          {
            // darstellen
            m_LastFrameTicks = Time::Timer::Time();
            UpdatePerDisplayFrame( (GR::f32)ElapsedTime );
            DisplayFrame( GetPage() );

            if ( m_UseCustomMouseCursor )
            {
              m_CustomMouseCursor.PutImage( GetPage(),
                                            m_pInput->MouseX() - m_MouseCursorOffset.x,
                                            m_pInput->MouseY() - m_MouseCursorOffset.y );
            }

            if ( m_SaveSnapShot )
            {
              m_SaveSnapShot = false;
              DoSaveSnapShot( m_SnapShotFile.c_str() );
            }


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
          }
        }
        if ( IsFullScreen() )
        {
          if ( m_pDirectXPage != NULL )
          {
            m_pDirectXPage->GFXDone();
            if ( !m_DoNotFlipPages )
            {
              m_pDirectXPage->GFXUpdate();
            }
          }
        }
        else
        {
          if ( IsWindow( GetHWND() ) )
          {
            InvalidateRect( GetHWND(), NULL, FALSE );
          }
        }
      }
    }
  }

  ExitInstance();

  CleanUp();

  GR::Service::Environment::Instance().RemoveService( "Application" );
  GR::Service::Environment::Instance().RemoveService( "GUI" );

  return msg.wParam;
}



void MasterFrame2d::ProcessEvent( const tGlobalEvent& Event )
{
  if ( Event.m_Type == m_ETChangeState )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      // übergebene Parameter
      GR::Strings::ParameterList    paramList;

      GR::String                   strParam = Event.m_Param;

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
      MFGameState<GR::Graphic::GFXPage>*    pGameState = static_cast<MFGameState<GR::Graphic::GFXPage>*>( pClone );
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
      ChangeState( (MFGameState<GR::Graphic::GFXPage>*)Event.m_Param1 );
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
      MFGameState<GR::Graphic::GFXPage>*    pGameState = static_cast<MFGameState<GR::Graphic::GFXPage>*>( pClone );
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
      PushState( (MFGameState<GR::Graphic::GFXPage>*)Event.m_Param1 );
    }
  }
  else if ( Event.m_Type == m_ETShutDown )
  {
    ShutDown();
  }
  else if ( Event.m_Type == m_ETToggleWindow )
  {
    ToggleFullScreen();
  }
}



void MasterFrame2d::SetCustomMouseCursor( GR::Graphic::Image* pImage, const GR::tPoint& ptMouseCursorOffset )
{
  if ( pImage == NULL )
  {
    m_UseCustomMouseCursor = false;
  }
  else
  {
    m_CustomMouseCursor = *pImage;
    m_UseCustomMouseCursor = true;
    m_MouseCursorOffset = ptMouseCursorOffset;
  }
}