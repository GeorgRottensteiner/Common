#include <windows.h>
#include <stdio.h>
#include <time.h>

#include <Input/Input.h>

#include <Misc/Misc.h>
#include <Grafik\gfxpage.h>
#include <Grafik\font.h>
#include <Grafik\image.h>

#include <Debug\debugclient.h>

#include <ScreenSaver/ScreenSaver.h>



typedef VOID (WINAPI *PWDCHANGEPASSWORD) ( LPCSTR lpcRegkeyname, HWND hwnd, UINT uiReserved1, UINT uiReserved2 );

// VORSICHT WARNUNG GEFÄHRLICH
// dieses Define ermöglicht Ctrl-Alt-Delete während des Screensaver-Betriebs
// ist es nicht gesetzt und der Screensaver hängt z.Bsp. in einer Endlosschleife, dann
// hilft nur noch ein RESET!!
#ifdef DEBUG
#define ALLOW_CTRL_ALT_DELETE
#endif



ScreenSaver::ScreenSaver() :
  m_gfxPage()
{
  m_hMutex                = NULL;

  m_hwndMain              = NULL;
  m_hInstance             = NULL;
  m_hwndFakePreview       = NULL;

  m_iWidth                = 640;
  m_iHeight               = 480;
  m_dwFlags               = 0;
  m_Mode                = SM_INVALID;

  m_ucDepth               = 16;
  m_dwFrameTime           = 30;

  m_dwMouseLimit          = 10;
  m_dwNoClose             = 1000;

  m_iVirtualDesktopWidth  = GetSystemMetrics( SM_CXSCREEN );
  m_iVirtualDesktopHeight = GetSystemMetrics( SM_CYSCREEN );


  /*
  // neu - virtuelle Desktops!
  OSVERSIONINFO           osviDummy;


  osviDummy.dwOSVersionInfoSize = sizeof( osviDummy );
  GetVersionEx( &osviDummy );
  if ( osviDummy.dwPlatformId == VER_PLATFORM_WIN32_NT )
  {
    if ( osviDummy.dwMajorVersion >= 5 )
    {
      m_iVirtualDesktopWidth  = GetSystemMetrics( 78 );//SM_CXVIRTUALSCREEN );
      m_iVirtualDesktopHeight = GetSystemMetrics( 79 );//SM_CYVIRTUALSCREEN );
    }
  }
  else if ( osviDummy.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
  {
    if ( ( osviDummy.dwMajorVersion == 4 )
    &&   ( osviDummy.dwMinorVersion > 0 ) )
    {
      m_iVirtualDesktopWidth  = GetSystemMetrics( 78 );//SM_CXVIRTUALSCREEN );
      m_iVirtualDesktopHeight = GetSystemMetrics( 79 );//SM_CYVIRTUALSCREEN );
    }
    else if ( osviDummy.dwMajorVersion > 4 )
    {
      m_iVirtualDesktopWidth  = GetSystemMetrics( 78 );//SM_CXVIRTUALSCREEN );
      m_iVirtualDesktopHeight = GetSystemMetrics( 79 );//SM_CYVIRTUALSCREEN );
    }
  }
  */

  m_iFullWidth            = m_iVirtualDesktopWidth;
  m_iFullHeight           = m_iVirtualDesktopHeight;

  m_pImageDesktop         = NULL;

  m_iMouseX               = 0;
  m_iMouseY               = 0;
  m_iOldMouseX            = 60000;
  m_iOldMouseY            = 60000;

  m_bMButton1Released     = TRUE;
  m_bMButton2Released     = TRUE;

  m_bInitialized          = FALSE;
  m_bDialogActive         = FALSE;
  m_bTrueClose            = FALSE;
  m_bShutDownNow          = FALSE;
  m_bClicked              = FALSE;
}



ScreenSaver::~ScreenSaver()
{
  Release();
}



bool ScreenSaver::Configure( const tScreenSaverInfo& tssInfo )
{
  m_hMutex = CreateMutex( NULL, TRUE, "GRScreenSaverMutex" );
  if ( GetLastError() == ERROR_ALREADY_EXISTS )
  {
    // der Saver läuft bereits (oder es gab einen fiesen Fehler)
    return false;
  }

  if ( m_bInitialized )
  {
    // der Screensaver ist schon initialisiert
    return true;
  }

  m_iWidth          = tssInfo.dwWidth;
  m_iHeight         = tssInfo.dwHeight;
  m_ucDepth         = tssInfo.ucColorDepth;

  m_dwFlags         = tssInfo.dwFlags;
  m_strAppName      = tssInfo.strAppName;
  m_strIniSection   = tssInfo.strIniSection;

  if ( m_dwFlags & SF_SHOW_CURSOR )
  {
    m_hCursor = LoadCursor( NULL, IDC_ARROW );
  }
  else
  {
    m_hCursor = NULL;
  }

  // Installations-Pfad aus CONTROL.INI auslesen
  if ( !m_strIniSection.empty() )
  {
  #ifndef _KUNDEN_PREVIEW_
    {
      char              szTemp[MAX_PATH];

      DWORD             result;

      GetWindowsDirectory( szTemp, MAX_PATH );
      if ( szTemp[strlen( szTemp ) - 1] == 92 )
      {
        szTemp[strlen( szTemp ) - 1] = 0;
      }
      wsprintf( szTemp, "%s\\control.ini", szTemp );

      char              szDummy[MAX_PATH];

      result = GetPrivateProfileString( m_strIniSection.c_str(), "Directory", "default", szDummy, MAX_PATH, szTemp );
      if ( ( result == 0 )
      ||   ( result >= MAX_PATH - 1 )
      ||   ( strcmp( "default", szDummy ) == 0 ) )
      {
        // Wir konnten den Verzeichniseintrag nicht finden!!!!!!
        dh::Log( "Configure: failed to locate path entry (%s) %s", szTemp, m_strIniSection.c_str() );
        return false;
      }

      m_strAppPath = szDummy;
    }
  #else
    {
      #pragma message( "=========================\n" )
      #pragma message( "kompiliert Kunden-Preview!!!\n" )
      #pragma message( "=========================\n" )
      m_Mode = SM_SAVER;
    }
  #endif
  }

  // Grafik initialisieren
  if ( m_Mode == SM_PREVIEW )
  {
    // im Preview-Mode auf optimale Fenstergröße setzen (die dann gestaucht wird)
    m_iFullWidth = m_iWidth;
    m_iFullHeight = m_iHeight;
  }
  m_gfxPage.Create( m_hwndMain, m_iFullWidth, m_iFullHeight, m_ucDepth );
  m_iXOffset = ( ( (int)m_iFullWidth - (int)m_iWidth ) / 2 );
  m_iYOffset = ( ( (int)m_iFullHeight - (int)m_iHeight ) / 2 );
  /*
  if ( m_dwFullWidth < m_dwWidth )
  {
    m_iXOffset = 0;
    m_iYOffset = 0;
  }
  */


  // Desktop grabben?
  if ( m_dwFlags & SF_GRAB_DESKTOP )
  {
    HWND  hwndDesktop;
    HDC   hdcDesktop;

    hwndDesktop = GetDesktopWindow();
    hdcDesktop = GetDC( hwndDesktop );
    StretchBlt( m_gfxPage.GetDC(), 0, 0, m_iFullWidth, m_iFullHeight, hdcDesktop, 0, 0, m_iFullWidth, m_iFullHeight, SRCCOPY );

    ReleaseDC( hwndDesktop, hdcDesktop );

    m_pImageDesktop = new GR::Graphic::Image( (WORD)m_iFullWidth, (WORD)m_iFullHeight, m_gfxPage.GetDepth(), IMAGE_METHOD_PLAIN, 0 );

    m_pImageDesktop->GetImage( &m_gfxPage, 0, 0 );
  }

  m_bInitialized = TRUE;

  return true;
}



void ScreenSaver::Release()
{
  if ( !m_bInitialized )
  {
    return;
  }
  if ( m_dwFlags & SF_GRAB_DESKTOP )
  {
    if ( m_pImageDesktop != NULL )
    {
      delete m_pImageDesktop;
      m_pImageDesktop = NULL;
    }
  }
  m_bInitialized = FALSE;
}



void ScreenSaver::AllowStretching( BOOL bAllow )
{
  if ( !m_bInitialized )
  {
    return;
  }
  if ( bAllow )
  {
    m_dwFlags |= SF_ALLOW_STRETCHING;
  }
  else
  {
    m_dwFlags &= ~SF_ALLOW_STRETCHING;
  }
}



void ScreenSaver::AllowMouseMovement( BOOL bAllow )
{
  if ( !m_bInitialized )
  {
    return;
  }
  if ( bAllow )
  {
    m_dwFlags |= SF_ALLOW_CURSOR_MOVE;
  }
  else
  {
    m_dwFlags &= ~SF_ALLOW_CURSOR_MOVE;
  }
}



void ScreenSaver::ShutDown()
{
  m_bTrueClose = TRUE;
  PostMessage( m_hwndMain, WM_CLOSE, 0, 0 );
}



void ScreenSaver::Invalidate( int iX, int iY, int iWidth, int iHeight )
{
  if ( iWidth == 0 )
  {
    InvalidateRect( m_hwndMain, NULL, FALSE );
    return;
  }

  RECT    rc;


  rc.left = iX;
  rc.top = iY;
  rc.right = rc.left + iWidth;
  rc.bottom = rc.top + iHeight;

  InvalidateRect( m_hwndMain, &rc, FALSE );

}



void ScreenSaver::WinPrint( int iX, int iY, const char* szText, DWORD dwColor )
{
  SetTextColor( m_gfxPage.GetDC(), RGB( ( dwColor & 0xff0000 ) >> 16, ( dwColor & 0xff00 ) >> 8, dwColor & 0xff ) );
  SetBkMode( m_gfxPage.GetDC(), TRANSPARENT );
  TextOut( m_gfxPage.GetDC(), iX, iY, szText, (int)strlen( szText ) );
}




void ScreenSaver::SSUpdateFrame()
{
  CInputSystem::Instance().Poll();
  if ( GetTickCount() - m_iStartZeit > m_dwFrameTime )
  {
    BYTE      ucDummy = 0;


    while ( GetTickCount() - m_iStartZeit > m_dwFrameTime )
    {
      if ( m_bShutDownNow )
      {
        return;
      }
      UpdateFrame();
      m_iStartZeit += m_dwFrameTime;
      if ( ucDummy++ >= 20 )
      {
        // Endlosschleife verhindern
        return;
      }
    }
    if ( !m_bShutDownNow )
    {
      DisplayFrame( &m_gfxPage );
    }
  }
  else
  {
    DWORD   dwValue = m_dwFrameTime - ( GetTickCount() - m_iStartZeit );
    if ( dwValue >= m_dwFrameTime )
    {
      // JA!!! Das passiert!!! (negative Werte)
      dwValue = m_dwFrameTime;
    }
    if ( dwValue == 0 )
    {
      dwValue = 1;
    }
    //Sleep( dwValue );
    //Sleep( 20 );
  }

}



bool ScreenSaver::VerifyPassword()
{
  typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);

  VERIFYSCREENSAVEPWD     VerifyScreenSavePwd;

  DWORD                   dwUsePassword,
                          dwUsePasswordSize = sizeof( dwUsePassword );

  HINSTANCE               hCPL;

  HKEY                    hKey;

  LONG                    err;

  OSVERSIONINFO           osviDummy;


  osviDummy.dwOSVersionInfoSize = sizeof( osviDummy );
  GetVersionEx( &osviDummy );
  if ( osviDummy.dwPlatformId == VER_PLATFORM_WIN32_NT )
  {
    // unter NT wird das Paßwort vom System geregelt!
    return TRUE;
  }

  // First we test if we are going to use a password or not
  err = RegOpenKey( HKEY_CURRENT_USER, "Control Panel\\Desktop", &hKey );
  if ( err != ERROR_SUCCESS )
  {
    // Konnte Key nicht auslesen, Paßwort ok
    RegCloseKey( hKey );
    return TRUE;
  }
  err = RegQueryValueEx( hKey, "ScreenSaveUsePassword", NULL, NULL, (LPBYTE) &dwUsePassword, &dwUsePasswordSize );
  if ( err != ERROR_SUCCESS )
  {
    // Konnte Key-Wert nicht auslesen, Paßwort ok
    RegCloseKey( hKey );
    return TRUE;
  }
  RegCloseKey( hKey );

  // If there is a password, we might as well set it's stuff up now
  if ( !dwUsePassword )
  {
    // Kein Paßwort benutzt!
    return TRUE;
  }

  // See section VerifyScreenSavePwd
  // for notes on the following call.
  hCPL = LoadLibrary( "PASSWORD.CPL" );
  if ( hCPL == NULL )
  {
    // Konnte Password.CPL nicht finden
    return TRUE;
  }
  VerifyScreenSavePwd = (VERIFYSCREENSAVEPWD)GetProcAddress( hCPL, "VerifyScreenSavePwd" );
  if ( VerifyScreenSavePwd == NULL )
  {
    // Konnte PaßwortVerifydingsda nicht finden
    return TRUE;
  }
  BOOL bResult = VerifyScreenSavePwd( m_hwndMain );
  FreeLibrary( hCPL );

  if ( bResult )
  {
    return true;
  }
  return false;
}



void ScreenSaver::CallExit()
{
  if ( m_bInitialized )
  {
    m_bInitialized = FALSE;
    ExitInstance();

    CInputSystem::Instance().Release();
  }
}



LRESULT CALLBACK ScreenSaverProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  static ScreenSaver*   pSaver = NULL;

  if ( ( msg == WM_CREATE )
  ||   ( msg == WM_NCCREATE ) )
  {
    LPCREATESTRUCT  lpcs = (LPCREATESTRUCT)lParam;

    pSaver = (ScreenSaver*)lpcs->lpCreateParams;
  }

  if ( pSaver )
  {
    return pSaver->WindowProc( hwnd, msg, wParam, lParam );
  }
  return 0;
}



LRESULT CALLBACK ScreenSaver::WindowProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  switch ( msg )
  {
    case WM_CREATE:
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_SYSCOMMAND:
      if ( ( ( 0xFFF0 & wParam ) == SC_CLOSE )
      ||   ( ( 0xFFF0 & wParam ) == SC_SCREENSAVE ) )
      {
        return FALSE;
      }
      break;
    case WM_SETCURSOR:
      if ( m_Mode == SM_SAVER )
      {
        // Im ScreenSaver-Modus Cursor aus
        SetCursor( m_hCursor );
        return TRUE;
        /*
        if ( !( theScreenSaver.m_dwFlags & SF_SHOW_CURSOR ) )
        {
          SetCursor( m_hCursor );
          return TRUE;
        }
        */
      }
      break;
    case WM_KEYUP:
      if ( m_Mode == SM_SAVER )
      {
        if ( !( m_dwFlags & SF_ALLOW_KEYBOARD ) )
        {
          if ( GetTickCount() - m_iSaverStartZeit > m_dwNoClose )
          {
            m_bTrueClose = TRUE;
            PostMessage( m_hwndMain, WM_CLOSE, 0, 0 );
          }
        }
      }
      return 0;
    case WM_KILLFOCUS:
      if ( m_Mode == SM_SAVER )
      {
        if ( !m_bDialogActive )
        {
          SetFocus( m_hwndMain );
        }
      }
      break;
      /*
    case WM_ACTIVATEAPP:
      if ( ( !(BOOL)wParam )
      &&   ( !m_bDialogActive ) )
      {
        if ( m_Mode == SM_SAVER )
        {
          SetFocus( m_hwndMain );
        }
      }
      break;
      */
    case WM_ACTIVATEAPP:
      if ( ( !(BOOL)wParam )
      &&   ( !m_bDialogActive ) )
      {
        if ( m_Mode == SM_SAVER )
        {
          m_bTrueClose = TRUE;
          PostMessage( m_hwndMain, WM_CLOSE, 0, 0 );
        }
      }
      break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MOUSEMOVE:

      m_iOldMouseX = m_iMouseX;
      m_iOldMouseY = m_iMouseY;
      m_iMouseX = LOWORD( lParam );
      m_iMouseY = HIWORD( lParam );

      if ( msg == WM_LBUTTONDOWN )
      {
        m_bClicked = TRUE;
        m_iClickedAtX = m_iMouseX;
        m_iClickedAtY = m_iMouseY;
      }
      if ( m_dwFlags & SF_ALLOW_STRETCHING )
      {
        m_iMouseX = ( m_iMouseX * m_iWidth ) / m_iFullWidth;
        m_iMouseY = ( m_iMouseY * m_iHeight ) / m_iFullHeight;
      }
      if ( ( !( m_dwFlags & SF_ALLOW_CURSOR_MOVE ) )
      &&   ( m_Mode == SM_SAVER )
      &&   ( m_iOldMouseX != 60000 ) )
      {
        // Im Saver-Modus und die Maus über das Limit bewegt
        if ( ( abs( m_iMouseX - m_iOldMouseX ) >= (int)m_dwMouseLimit )
        ||   ( abs( m_iMouseY - m_iOldMouseY ) >= (int)m_dwMouseLimit ) )
        {
          // Im Saver-Modus und die Maus über das Limit bewegt
          if ( GetTickCount() - m_iSaverStartZeit > m_dwNoClose )
          {
            m_bTrueClose = TRUE;
            PostMessage( m_hwndMain, WM_CLOSE, 0, 0 );
          }
        }
      }
      m_iMButton = 0;
      if ( wParam & MK_LBUTTON )
      {
        m_iMButton |= 1;
      }
      else
      {
        m_bMButton1Released = TRUE;
      }
      if ( wParam & MK_RBUTTON )
      {
        m_iMButton |= 2;
      }
      else
      {
        m_bMButton2Released = TRUE;
      }
      return 0;
    case WM_COMMAND:
      break;
    case WM_CLOSE:
      // vorm Schließen prüfen, ob ein Paßwort eingetragen ist (ggf. abfragen)
      if ( !m_bDialogActive )
      {
        if ( m_Mode == SM_PREVIEW )
        {
          m_bDialogActive = FALSE;
          Release();
          m_bShutDownNow = TRUE;

          CallExit();

          DestroyWindow( hwnd );
          PostQuitMessage( 0 );
          return 0;
        }
        if ( m_bTrueClose )
        {
          m_bDialogActive = TRUE;
          if ( VerifyPassword() )
          {
            m_bDialogActive = FALSE;
            m_bShutDownNow = TRUE;

            CallExit();

            PostQuitMessage( 0 );
          }
          m_bDialogActive = FALSE;
        }
      }
      return 0;
    case WM_PAINT:
      if ( m_bInitialized )
      {
        if ( m_gfxPage.GetDepth() != 0 )
        {
          #ifdef EXTERN_GFX_UPDATE
          ScreenSaverGFXUpdate();
          #else
          //ScreenSaverPreUpdate();
          m_gfxPage.GFXUpdate( (BOOL)!!( m_dwFlags & SF_ALLOW_STRETCHING ),
                                              FALSE, NULL, NULL );
          //ScreenSaverPostUpdate();
          #endif
        }
      }
      return 0L;
  }
  return DefWindowProc( hwnd, msg, wParam, lParam );
}



void ScreenSaver::Run()
{
  MSG               msg;

  WNDCLASS          wc;

  BOOL              bWindowFound = FALSE;

  int               iDummy;

  HINSTANCE         hMPR;

  PWDCHANGEPASSWORD PwdChangePassword;

  RECT              rc;

  DWORD             dwActParam,
                    dwActPos,
                    dwActStart;

  char              szParameter[10][50],
                    szCmd[500];


  m_hInstance = GetModuleHandle( NULL );

  wsprintf( szCmd, "%s ", GetCommandLine() );

  dwActPos = 0;
  dwActStart = 0;
  dwActParam = 0;
  if ( strlen( szCmd ) > 1 )
  {
    while ( dwActPos < (int)strlen( szCmd ) )
    {
      if ( szCmd[dwActPos] == '"' )
      {
        dwActPos++;
        dwActStart++;
        while ( dwActPos < (int)strlen( szCmd ) )
        {
          if ( szCmd[dwActPos] == '"' )
          {
            if ( dwActParam < 10 )
            {
              memcpy( szParameter[dwActParam], &szCmd[dwActStart], dwActPos - dwActStart );
              szParameter[dwActParam][dwActPos - dwActStart] = 0;
            }
            dwActParam++;
            dwActPos++;
            dwActStart = dwActPos + 1;
            break;
          }
          ++dwActPos;
        }
      }
      else if ( szCmd[dwActPos] == ' ' )
      {
        if ( dwActStart == dwActPos - 1 )
        {
          dwActStart++;
        }
        else
        {
          // Ein Parameter!
          if ( dwActParam < 10 )
          {
            memcpy( szParameter[dwActParam], &szCmd[dwActStart], dwActPos - dwActStart );
            szParameter[dwActParam][dwActPos - dwActStart] = 0;
          }
          dwActParam++;
          dwActStart = dwActPos + 1;
        }
      }
      dwActPos++;
    }
  }

  m_Mode = SM_INVALID;
  if ( dwActParam == 1 )
  {
    m_Mode = SM_CONFIGURE;
  }
  else
  {
    if ( ( strncmp( szParameter[1], "/c", 2 ) == 0 )
    ||   ( strncmp( szParameter[1], "/C", 2 ) == 0 )
    ||   ( strncmp( szParameter[1], "-c", 2 ) == 0 )
    ||   ( strncmp( szParameter[1], "-C", 2 ) == 0 ) )
    {
      // ScreenSaver konfigurieren
      m_Mode = SM_CONFIGURE;
    }
    else if ( ( strncmp( szParameter[1], "/s", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "/S", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-s", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-S", 2 ) == 0 ) )
    {
      // Saver-Modus
      m_Mode = SM_SAVER;
    }
    else if ( ( strncmp( szParameter[1], "/f", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "/F", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-f", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-F", 2 ) == 0 ) )
    {
      // bPreview
      m_Mode = SM_PREVIEW;
    }
    else if ( ( strncmp( szParameter[1], "/p", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "/P", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-p", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-P", 2 ) == 0 ) )
    {
      // bPreview
      m_Mode = SM_PREVIEW;
    }
    else if ( ( strncmp( szParameter[1], "/l", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "/L", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-l", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-L", 2 ) == 0 ) )
    {
      // bPreview (eigentlich irgendwas mit Timing)
      m_Mode = SM_PREVIEW;
    }
    else if ( ( strncmp( szParameter[1], "/a", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "/A", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-a", 2 ) == 0 )
    ||        ( strncmp( szParameter[1], "-A", 2 ) == 0 ) )
    {
      // Paßwort ändern
      HWND        hwndDummy;
      if ( dwActParam >= 3 )
      {
        iDummy = sscanf( szParameter[3], "%i", &hwndDummy );
        if ( iDummy == 0 )
        {
          // kein "lesbarer" Parameter
          return;
        }
        bWindowFound = IsWindow( hwndDummy );
      }
      if ( !bWindowFound )
      {
        hwndDummy = GetForegroundWindow();
      }
      hMPR = LoadLibrary( "MPR.DLL" );
      if ( hMPR == NULL )
      {
        // Konnte MPR.DLL nicht laden
        return;
      }
      PwdChangePassword = (PWDCHANGEPASSWORD)GetProcAddress( hMPR, "PwdChangePasswordA" );
      if ( PwdChangePassword == NULL )
      {
        // Konnte PwdChangePassword nicht anfordern
        return;
      }
      PwdChangePassword( "SCRSAVE", hwndDummy, 0, 0 );
      FreeLibrary( hMPR );
      return;
    }
    else
    {
      //m_Mode = SM_SAVER;
      return;
    }
  }

#ifdef _KUNDEN_PREVIEW_
  m_Mode = SM_SAVER;
#endif _KUNDEN_PREVIEW_

  if ( ( m_Mode == SM_SAVER )
  ||   ( m_Mode == SM_PREVIEW ) )
  {
    wc.style          = 0;
    wc.lpfnWndProc    = (WNDPROC)ScreenSaverProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = m_hInstance;
    wc.hIcon          = NULL;//LoadIcon( NULL, MAKEINTRESOURCE( IDI_ICON1 ) );
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground  = (HBRUSH)GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = "GRSaverClass";
    if ( !RegisterClass( &wc ) )
    {
      dh::Log( "ScreenSaver::Run RegisterClass failed.\n" );
      return;
    }
  }

  if ( ( m_iFullWidth < 640 )
  ||   ( m_iFullHeight < 480 ) )
  {
    // Auflösung kleiner als 640x480???
    m_dwFlags |= SF_ALLOW_STRETCHING;
  }


  if ( m_Mode == SM_SAVER )
  {
    // VollbildFenster
#ifdef _DEBUG
    m_hwndMain = CreateWindowEx( 0,
#else
    m_hwndMain = CreateWindowEx( WS_EX_TOPMOST,
#endif
                              "GRSaverClass",
                              m_strAppName.c_str(),
                              WS_POPUP | WS_VISIBLE,
                              0,
                              0,
                              m_iFullWidth,
                              m_iFullHeight,
                              NULL,
                              NULL,
                              m_hInstance,
                              this );
    if ( m_hwndMain == NULL )
    {
      dh::Log( "ScreenSaver::Run CreateWindowEx failed.\n" );
      return;
    }
  }
  else if ( m_Mode == SM_PREVIEW )
  {
    // bPreview-Fenster
    if ( dwActParam < 3 )
    {
      // kein zweiter Parameter!
      dh::Log( "ScreenSaver::Run Preview missing parameter.\n" );
      return;
    }
    iDummy = sscanf( szParameter[2], "%i", &m_hwndFakePreview );
    if ( iDummy == 0 )
    {
      // kein "lesbarer" Parameter
      dh::Log( "ScreenSaver::Run Preview Parameter invalid.\n" );
      return;
    }
    if ( !IsWindow( m_hwndFakePreview ) )
    {
      // kein gültiges Window-Handle
      dh::Log( "ScreenSaver::Run Preview Parameter invalid HWND.\n" );
      return;
    }
    GetWindowRect( m_hwndFakePreview, &rc );

    m_dwFlags |= SF_ALLOW_STRETCHING;

    m_hwndMain = CreateWindow( "GRSaverClass",
                                 m_strAppName.c_str(),
                                 WS_CHILD | WS_VISIBLE,
                                 0,
                                 0,
                                 rc.right - rc.left,
                                 rc.bottom - rc.top,
                                 m_hwndFakePreview,
                                 NULL,
                                 m_hInstance,
                                 this );
    // wir machen ein kleines Fenster auf, aber lassen die Page in der gewünschten Full-Screen-Größe
    //theScreenSaver.m_dwFullWidth = rc.right - rc.left;
    //theScreenSaver.m_dwFullHeight = rc.bottom - rc.top;
    if ( m_hwndMain == NULL )
    {
      dh::Log( "ScreenSaver::Run Preview CreateWindow failed.\n" );
      return;
    }
  }

  if ( !CInputSystem::Instance().Create( m_hInstance, m_hwndMain, FALSE ) )
  {
    dh::Log( "ScreenSaver::Run unitInput.Create failed.\n" );
    return;
  }
  if ( !InitInstance() )
  {
    ShowWindow( m_hwndMain, SW_HIDE );
    return;
  }
  if ( m_Mode == SM_CONFIGURE )
  {
    // Ohne Parameter aufgerufen -> auch Config
    ConfigureSaver();

    CallExit();
    return;
  }

  // die Zeit zählt ab jetzt
  m_iStartZeit = GetTickCount();
  m_iSaverStartZeit = m_iStartZeit;

  ShowWindow( m_hwndMain, SW_SHOW );
  UpdateWindow( m_hwndMain );
  if ( m_Mode & SM_SAVER )
  {
#ifndef ALLOW_CTRL_ALT_DELETE
    SystemParametersInfo( SPI_SCREENSAVERRUNNING, 1, &iDummy, 0 );

    //SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, 1, 0, 0 );
#endif
  }

  while ( 1 )
  {
    if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
    {
      if ( !GetMessage( &msg, NULL, 0, 0 ) )
      {
        break;
      }
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
    else
    {
      if ( !m_bShutDownNow )
      {
        SSUpdateFrame();
      }
    }
    if ( !( m_Mode & SM_SAVER ) )
    {
      // Preview-Modus muß nicht die volle Threadleistung ziehen
      if ( GetTickCount() - m_iSaverStartZeit > m_dwNoClose )
      {
        Sleep( 20 );
      }
    }
  }
  if ( m_Mode & SM_SAVER )
  {
#ifndef ALLOW_CTRL_ALT_DELETE
    SystemParametersInfo( SPI_SCREENSAVERRUNNING, 0, &iDummy, 0 );
    //SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, 0, 0, 0 );
#endif
  }

  CallExit();

  if ( m_hMutex )
  {
    ReleaseMutex( m_hMutex );
    CloseHandle( m_hMutex );
    m_hMutex = NULL;
  }

  if ( IsWindow( m_hwndFakePreview ) )
  {
    DestroyWindow( m_hwndFakePreview );
    m_hwndFakePreview = NULL;
  }
}



const char* ScreenSaver::AppPath( const char* formatstr, ... )
{
  static char         szBuffer[3000];

  wvsprintf( szBuffer, formatstr, (char *)( &formatstr + 1 ) );

  return GR::String( m_strAppPath + szBuffer ).c_str();
}



