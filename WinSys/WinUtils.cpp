#include <tchar.h>
#include <shlobj.h>

#include <String/Convert.h>
#include <String/StringUtil.h>

#include "WinUtils.h"

#include <WinSys/RegistryHelper.h>

#include <Grafik/ImageData.h>
#include <Grafik/ContextDescriptor.h>
#include <Grafik/Palette.h>

// used for checking GetVersionInfo availability vs. deprecation
#include <ntverp.h>
#if VER_PRODUCTBUILD >= 9600
  // should be >= Windows 8.1 SDK code goes here
#include <VersionHelpers.h>
#endif

namespace
{
  bool s_EnumVisibleWindowsOnly = false;
}




BOOL CALLBACK Win::Util::EnumWindowProc( HWND hwnd, LPARAM lParam )
{
  std::list<HWND>*    pListWindows = (std::list<HWND>*)lParam;

  if ( ( !s_EnumVisibleWindowsOnly )
  ||   ( ::IsWindowVisible( hwnd ) ) )
  {
    pListWindows->push_back( hwnd );
  }

  return TRUE;
}



std::list<HWND> Win::Util::EnumWindows( bool VisibleOnly )
{
  std::list<HWND>     listWindows;

  s_EnumVisibleWindowsOnly = VisibleOnly;

  ::EnumWindows( EnumWindowProc, (LPARAM)&listWindows );

  return listWindows;
}



std::list<HWND> Win::Util::EnumChildWindows( HWND HwndParent, bool VisibleOnly )
{
  std::list<HWND>     listWindows;

  s_EnumVisibleWindowsOnly = true;

  ::EnumChildWindows( HwndParent, EnumWindowProc, (LPARAM)&listWindows );

  return listWindows;
}



HWND Win::Util::FindWindowWhichContains( const GR::Char* Text )
{
  std::list<HWND>     listWindows = EnumWindows();

  std::list<HWND>::iterator   it( listWindows.begin() );
  while ( it != listWindows.end() )
  {
    int   length = ( int )::SendMessageW( *it, WM_GETTEXTLENGTH, 0, 0 );

    WCHAR* pDummy = new WCHAR[length + 3];

    ::GetWindowTextW( *it, pDummy, length + 1 );

    GR::String    result = GR::Convert::ToUTF8( pDummy );
    delete[] pDummy;

    if ( result.find( Text ) != GR::String::npos )
    {
      return *it;
    }
    ++it;
  }
  return NULL;
}



HICON Win::Util::CreateGrayscaleIcon( HICON hIcon, bool bFlat )
{
  HICON       hGrayIcon = NULL;
  HDC         hMainDC = NULL,
              hMemDC1 = NULL,
              hMemDC2 = NULL;
  BITMAP      bmp;
  HBITMAP     hOldBmp1 = NULL,
              hOldBmp2 = NULL;
  ICONINFO    csII, csGrayII;
  BOOL        bRetValue = FALSE;

  bRetValue = ::GetIconInfo(hIcon, &csII);
  if ( bRetValue == FALSE )
  {
    return NULL;
  }

  hMainDC = ::GetDC( ::GetDesktopWindow() );
  hMemDC1 = ::CreateCompatibleDC( hMainDC );
  hMemDC2 = ::CreateCompatibleDC( hMainDC );
  if ( ( hMainDC == NULL )
  ||   ( hMemDC1 == NULL )
  ||   ( hMemDC2 == NULL )  )
  {
    return NULL;
  }

  if ( ::GetObject( csII.hbmColor, sizeof( BITMAP ), &bmp ) )
  {
    csGrayII.hbmColor = ::CreateBitmap( csII.xHotspot * 2,
                                        csII.yHotspot * 2,
                                        bmp.bmPlanes,
                                        bmp.bmBitsPixel,
                                        NULL );
    if ( csGrayII.hbmColor )
    {
      hOldBmp1 = (HBITMAP)::SelectObject( hMemDC1, csII.hbmColor );
      hOldBmp2 = (HBITMAP)::SelectObject( hMemDC2, csGrayII.hbmColor );

      ::BitBlt( hMemDC2, 0, 0, csII.xHotspot * 2, csII.yHotspot * 2, hMemDC1, 0, 0, SRCCOPY );

      GR::u32    LoopY = 0, LoopX = 0;
      COLORREF crPixel = 0;
      BYTE     byNewPixel = 0;

      for ( LoopY = 0; LoopY < csII.yHotspot * 2; LoopY++ )
      {
        for ( LoopX = 0; LoopX < csII.xHotspot * 2; LoopX++ )
        {
          crPixel = ::GetPixel( hMemDC2, LoopX, LoopY );

          if ( bFlat )
          {
            byNewPixel = 128;
          }
          else
          {
          byNewPixel = (BYTE)( ( GetRValue( crPixel ) * 0.299 ) +
                               ( GetGValue( crPixel ) * 0.587 ) +
                               ( GetBValue( crPixel ) * 0.114 ) );
          }
          if ( crPixel )
          {
            ::SetPixel( hMemDC2,
                        LoopX,
                        LoopY,
                        RGB( byNewPixel, byNewPixel, byNewPixel ) );
          }
        }
      }

      ::SelectObject( hMemDC1, hOldBmp1 );
      ::SelectObject( hMemDC2, hOldBmp2 );

      csGrayII.hbmMask = csII.hbmMask;

      csGrayII.fIcon = TRUE;
      hGrayIcon = ::CreateIconIndirect( &csGrayII );
    }

    if ( csGrayII.hbmColor != NULL )
    {
      ::DeleteObject( csGrayII.hbmColor );
    }
  }

  ::DeleteObject( csII.hbmColor );
  ::DeleteObject( csII.hbmMask );
  ::DeleteDC( hMemDC1 );
  ::DeleteDC( hMemDC2 );
  ::ReleaseDC( ::GetDesktopWindow(), hMainDC );

  return hGrayIcon;

}



GR::u32 Win::Util::InternetExplorerVersion()
{
  GR::String      ieVersion;

  if ( Registry::GetKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Internet Explorer", "Version", ieVersion ) )
  {
    return GR::Convert::ToU32( ieVersion );
  }
  if ( Registry::GetKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Internet Explorer", "IVer", ieVersion ) )
  {
    if ( ieVersion.empty() )
    {
      return 0;
    }
    int   iVer = GR::Convert::ToI32( ieVersion );

    if ( iVer == 103 )
    {
      return 3;
    }
    else if ( iVer == 102 )
    {
      return 2;   // 2.5
    }
    else if ( iVer == 101 )
    {
      return 2;
    }
    else if ( iVer == 100 )
    {
      return 1;
    }
  }
  return 0;
}




//-----------------------------------------------------------------------------
// Name: GetDXVersion()
// Desc: This function returns the DirectX version number as follows:
//          0x0000 = No DirectX installed
//          0x0100 = DirectX version 1 installed
//          0x0200 = DirectX 2 installed
//          0x0300 = DirectX 3 installed
//          0x0301 = DirectX 3.0a oder 3.0b installed
//          0x0500 = At least DirectX 5 installed.
//          0x0600 = At least DirectX 6 installed.
//          0x0601 = At least DirectX 6.1 installed.
//          0x0602 = At least DirectX 6.1a installed.
//          0x0700 = At least DirectX 7 installed.
//          0x0800 = At least DirectX 8 installed.
//          0x0801 = DirectX 8.1
//          0x0802 = DirectX 8.1a
//          0x0803 = DirectX 8.1b
//          0x0804 = DirectX 8.2
//          0x0900 = DirectX 9
//
//       Please note that this code is intended as a general guideline. Your
//       app will probably be able to simply query for functionality (via
//       QueryInterface) for one or two components.
//
//       Please also note:
//          "if( DXVersion != 0x500 ) return FALSE;" is VERY BAD.
//          "if( DXVersion <  0x500 ) return FALSE;" is MUCH BETTER.
//       to ensure your app will run on future releases of DirectX.
//-----------------------------------------------------------------------------

/*-GetDXVersion---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#pragma comment( lib, "version.lib" )
//-----------------------------------------------------------------------------
// Name: GetFileVersion()
// Desc: Returns ULARGE_INTEGER with a file version of a file, or a failure code.
//-----------------------------------------------------------------------------
HRESULT GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion )
{
  if ( ( szPath == NULL )
  ||   ( pllFileVersion == NULL ) )
  {
    return E_INVALIDARG;
  }

  DWORD   Handle;
  UINT    cb;
  cb = GetFileVersionInfoSize( szPath, &Handle );
  if ( cb > 0 )
  {
    BYTE* pFileVersionBuffer = new BYTE[cb];
    if ( pFileVersionBuffer == NULL )
    {
      return E_OUTOFMEMORY;
    }

    if ( GetFileVersionInfo( szPath, 0, cb, pFileVersionBuffer ) )
    {
      VS_FIXEDFILEINFO* pVersion = NULL;
      if ( ( VerQueryValue( pFileVersionBuffer, TEXT( "\\" ), (VOID**)&pVersion, &cb ) )
      &&   ( pVersion != NULL ) )
      {
        pllFileVersion->HighPart = pVersion->dwFileVersionMS;
        pllFileVersion->LowPart  = pVersion->dwFileVersionLS;
        delete[] pFileVersionBuffer;
        return S_OK;
      }
    }

    delete[] pFileVersionBuffer;
  }

  return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: MakeInt64()
// Desc: Returns a ULARGE_INTEGER where a<<48|b<<32|c<<16|d<<0
//-----------------------------------------------------------------------------
ULARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d )
{
    ULARGE_INTEGER ull;
    ull.HighPart = MAKELONG(b,a);
    ull.LowPart = MAKELONG(d,c);
    return ull;
}




//-----------------------------------------------------------------------------
// Name: CompareLargeInts()
// Desc: Returns 1 if ullParam1 > ullParam2
//       Returns 0 if ullParam1 = ullParam2
//       Returns -1 if ullParam1 < ullParam2
//-----------------------------------------------------------------------------
int CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 )
{
    if( ullParam1.HighPart > ullParam2.HighPart )
        return 1;
    if( ullParam1.HighPart < ullParam2.HighPart )
        return -1;

    if( ullParam1.LowPart > ullParam2.LowPart )
        return 1;
    if( ullParam1.LowPart < ullParam2.LowPart )
        return -1;

    return 0;
}

GR::u32 Win::Util::GetDXVersion()
{

  ULARGE_INTEGER llFileVersion;
  TCHAR szPath[512];
  TCHAR szFile[512];

  GR::u32       Version = 0;

  if ( GetSystemDirectory( szPath, MAX_PATH ) != 0 )
  {
    szPath[MAX_PATH - 1] = 0;

    // Switch off the ddraw version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT( "\\ddraw.dll" ) );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 2, 0, 95 ) ) >= 0 ) // Win9x version
      {
        Version = 0x0100;
      }

      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 3, 0, 1096 ) ) >= 0 ) // Win9x version
      {
        // flle is is >= DX2.0 version, so we must DX2.0 or DX2.0a (no redist change)
        Version = 0x0200;
      }

      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 4, 0, 68 ) ) >= 0 ) // Win9x version
      {
        // flle is is >= DX3.0 version, so we must be at least DX3.0
        Version = 0x0300;
      }
    }

    // Switch off the d3drg8x.dll version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\d3drg8x.dll") );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if( CompareLargeInts( llFileVersion, MakeInt64( 4, 4, 0, 70 ) ) >= 0 ) // Win9x version
      {
        // d3drg8x.dll is the DX3.0a version, so we must be DX3.0a or DX3.0b  (no redist change)
        Version = 0x0301;
      }
    }

    // Switch off the ddraw version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\ddraw.dll") );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 5, 0, 155 ) ) >= 0 ) // Win9x version
      {
        // ddraw.dll is the DX5.0 version, so we must be DX5.0 or DX5.2 (no redist change)
        Version = 0x0500;
      }

      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 6, 0, 318 ) ) >= 0 ) // Win9x version
      {
        Version = 0x0600;
      }

      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 6, 0, 436 ) ) >= 0 ) // Win9x version
      {
        // ddraw.dll is the DX6.1 version, so we must be at least DX6.1
        Version = 0x0601;
      }
    }

    // Switch off the dplayx.dll version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\dplayx.dll") );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 6, 3, 518 ) ) >= 0 ) // Win9x version
      {
        // ddraw.dll is the DX6.1 version, so we must be at least DX6.1a
        Version = 0x0602;
      }
    }

    // Switch off the ddraw version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\ddraw.dll") );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( ( CompareLargeInts( llFileVersion, MakeInt64( 4, 7, 0, 700 ) ) >= 0 ) // Win9x version
      ||   ( HIWORD( llFileVersion.HighPart ) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 0, 2180, 1 ) ) >= 0) ) // Win2k/WinXP version
      {
        // ddraw.dll is the DX7.0 version, so we must be at least DX7.0
        Version = 0x0700;
      }
    }

    // Switch off the dinput version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\dinput.dll") );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( CompareLargeInts( llFileVersion, MakeInt64( 4, 7, 0, 716 ) ) >= 0 ) // Win9x version
      {
        // ddraw.dll is the DX7.0 version, so we must be at least DX7.0a
        Version = 0x0701;
      }
    }

    // Switch off the ddraw version
    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\ddraw.dll") );
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( ( HIWORD( llFileVersion.HighPart ) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 8, 0, 400 ) ) >= 0) || // Win9x version
           ( HIWORD( llFileVersion.HighPart ) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 1, 2258, 400 ) ) >= 0) ) // Win2k/WinXP version
      {
        // ddraw.dll is the DX8.0 version, so we must be at least DX8.0 or DX8.0a (no redist change)
        Version = 0x0800;
      }
    }

    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\d3d8.dll"));
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if( (HIWORD(llFileVersion.HighPart) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 8, 1, 881 ) ) >= 0) || // Win9x version
          (HIWORD(llFileVersion.HighPart) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 1, 2600, 881 ) ) >= 0) ) // Win2k/WinXP version
      {
        // d3d8.dll is the DX8.1 version, so we must be at least DX8.1
        Version = 0x0801;
      }

      if ( ( HIWORD( llFileVersion.HighPart ) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 8, 1, 901 ) ) >= 0) || // Win9x version
           ( HIWORD( llFileVersion.HighPart ) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 1, 2600, 901 ) ) >= 0) ) // Win2k/WinXP version
      {
        // d3d8.dll is the DX8.1a version, so we must be at least DX8.1a
        Version = 0x0802;
      }
    }

    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\mpg2splt.ax"));
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if ( CompareLargeInts( llFileVersion, MakeInt64( 6, 3, 1, 885 ) ) >= 0 ) // Win9x/Win2k/WinXP version
      {
        // quartz.dll is the DX8.1b version, so we must be at least DX8.1b
        Version = 0x0803;
      }
    }

    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\dpnet.dll"));
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      if( (HIWORD(llFileVersion.HighPart) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 9, 0, 134 ) ) >= 0) || // Win9x version
          (HIWORD(llFileVersion.HighPart) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 2, 3677, 134 ) ) >= 0) ) // Win2k/WinXP version
      {
        // dpnet.dll is the DX8.2 version, so we must be at least DX8.2
        Version = 0x0804;
      }
    }

    _tcscpy_s( szFile, MAX_PATH - 1, szPath );
    _tcscat_s( szFile, MAX_PATH - 1, TEXT("\\d3d9.dll"));
    if ( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
    {
      // File exists, but be at least DX9
      Version = 0x0900;
    }
  }

  return Version;

}



bool Win::Util::StartProgram( const GR::Char* CommandLine, int iShowWindowFlags )
{
  STARTUPINFOW            sui;

  PROCESS_INFORMATION     pi;


  memset( &sui, 0, sizeof( sui ) );
  sui.cb = sizeof( sui );
  sui.wShowWindow = iShowWindowFlags;

  GR::String    strFileName = CommandLine,
                strCmdLine = CommandLine;

  if ( ( strFileName.length() )
  &&   ( strFileName[0] == '"' ) )
  {
    // der Pfad ist in Anführungszeichen gehalten
    strCmdLine = strFileName;
    strFileName = strFileName.substr( 1 );
    if ( strFileName.find( '"' ) != GR::String::npos )
    {
      strFileName = strFileName.substr( 0, strFileName.find( '"' ) );
    }
  }
  else if ( strFileName.length() )
  {
    if ( strFileName.find( ' ' ) != GR::String::npos )
    {
      strFileName = strFileName.substr( 0, strFileName.find( ' ' ) );
    }
  }

  GR::String   strCurDir = strFileName;

  while ( ( strCurDir.length() )
  &&      ( strCurDir[strCurDir.length() - 1] != '\\' ) )
  {
    strCurDir = strCurDir.substr( 0, strCurDir.length() - 1 );
  }

  if ( strCurDir.length() )
  {
    strCurDir = strCurDir.substr( 0, strCurDir.length() - 1 );
  }

  if ( strCurDir.length() == 2 )
  {
    strCurDir += '\"';
  }

  GR::WString   cmdLine = GR::Convert::ToUTF16( strCmdLine );

  GR::WChar*   pCmdLine = new GR::WChar[cmdLine.length() + 2];

  memcpy( pCmdLine, cmdLine.c_str(), cmdLine.length() * sizeof( GR::WChar ) + 2 );



  if ( !CreateProcessW( GR::Convert::ToUTF16( strFileName ).c_str(),
                        pCmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        0,//CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                        NULL,
                       GR::Convert::ToUTF16( strCurDir ).c_str(),
                        &sui,
                        &pi ) )
  {
    delete[] pCmdLine;
    return false;
  }

  delete[] pCmdLine;

  CloseHandle( pi.hThread );
  CloseHandle( pi.hProcess );

  return true;

}



bool Win::Util::StartAndWaitForProgram( const GR::Char* CommandLine, GR::u32* pExitCode, int iShowWindowFlags )
{
  STARTUPINFOW            sui;

  PROCESS_INFORMATION     pi;


  memset( &sui, 0, sizeof( sui ) );
  sui.cb = sizeof( sui );
  sui.wShowWindow = iShowWindowFlags;

  GR::String    strFileName = CommandLine,
                    strCmdLine = CommandLine;

  if ( ( strFileName.length() )
  &&   ( strFileName[0] == '"' ) )
  {
    // der Pfad ist in Anführungszeichen gehalten
    strCmdLine = strFileName;
    strFileName = strFileName.substr( 1 );
    if ( strFileName.find( '"' ) != GR::String::npos )
    {
      strFileName = strFileName.substr( 0, strFileName.find( '"' ) );
    }
  }
  else if ( strFileName.length() )
  {
    if ( strFileName.find( ' ' ) != GR::String::npos )
    {
      strFileName = strFileName.substr( 0, strFileName.find( ' ' ) );
    }
  }

  GR::String   strCurDir = strFileName;

  while ( ( strCurDir.length() )
  &&      ( strCurDir[strCurDir.length() - 1] != '\\' ) )
  {
    strCurDir = strCurDir.substr( 0, strCurDir.length() - 1 );
  }

  if ( strCurDir.length() )
  {
    strCurDir = strCurDir.substr( 0, strCurDir.length() - 1 );
  }

  if ( strCurDir.length() == 2 )
  {
    strCurDir += '\\';
  }

  GR::WString   cmdLine = GR::Convert::ToUTF16( strCmdLine );

  GR::WChar*   pCmdLine = new GR::WChar[cmdLine.length() + 2];

  memcpy( pCmdLine, cmdLine.c_str(), cmdLine.length() * sizeof( GR::WChar ) + 2 );

  if ( !CreateProcessW( GR::Convert::ToUTF16( strFileName ).c_str(),
                        pCmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                        NULL,
                       GR::Convert::ToUTF16( strCurDir ).c_str(),
                        &sui,
                        &pi ) )
  {
    delete[] pCmdLine;
    return false;
  }
  delete[] pCmdLine;

  MSG     msg;

  DWORD   ExitCode = STILL_ACTIVE;

  do
  {
    Sleep( 20 );
    if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
    {
      GetMessage( &msg, NULL, 0, 0 );
      if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
      {
        continue;
      }
      if ( msg.message == WM_ENDSESSION )
      {
        if ( (BOOL)msg.wParam )
        {
          // das aufgerufene Programm will Windows beenden
          // -> Loop beenden
          break;
        }
      }
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
    GetExitCodeProcess( pi.hProcess, &ExitCode );
  }
  while ( ExitCode == STILL_ACTIVE );

  CloseHandle( pi.hThread );
  CloseHandle( pi.hProcess );

  if ( pExitCode != NULL )
  {
    *pExitCode = ExitCode;
  }

  return true;

}



GR::String Win::Util::LoadStringEx( GR::u32 ResourceId, GR::u16 wLanguage )
{
  GR::String   strTemp;

  int nBlock = ResourceId / 16 + 1;
  int nNum = ResourceId & 0x0f;

  HRSRC hRes = FindResourceEx( GetModuleHandle( NULL ), RT_STRING, MAKEINTRESOURCE( nBlock ), wLanguage );

  if ( hRes == NULL )
  {
    return GR::String();
  }

  HGLOBAL hGlobal = LoadResource( GetModuleHandle( NULL ), hRes );
  if ( hGlobal == NULL )
  {
    return GR::String();
  }

  WCHAR*   pData = (WCHAR*)LockResource( hGlobal );

  WORD    wStringLength = 0;
  if ( pData )
  {

    for ( int i = 0; i < nNum; ++i )
    {
      wStringLength = (WORD)*pData;

      pData += 1 + wStringLength;
    }
    wStringLength = (WORD)*pData;
    ++pData;

#ifdef UNICODE
    strTemp.append( (GR::Char*)pData, wStringLength );
#else

    char*      pTemp = new char[wStringLength + 1];

    WideCharToMultiByte( CP_ACP, 0, (WCHAR*)pData, wStringLength, pTemp, wStringLength, 0, 0 );

    pTemp[wStringLength] = 0;

    strTemp = pTemp;

    delete[] pTemp;
#endif
  }

  UnlockResource( hGlobal );
  FreeResource( hGlobal );

  return strTemp;

}



GR::String Win::Util::LoadString( GR::u32 ResourceId )
{
  LANGID curLang = GetUserDefaultLangID();

  GR::String     strString = LoadStringEx( ResourceId, curLang );
  if ( !strString.empty() )
  {
    return strString;
  }
  return LoadStringEx( ResourceId, MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ) );
}



Win::Util::eWindowsVersion Win::Util::GetWindowsVersion()
{
#if ( ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) ) \
||      ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE ) ) )
  if ( IsWindows10OrGreater() )
  {
    return WV_WIN10;
  }
  else if ( IsWindows7OrGreater() )
  {
    return WV_WIN7;
  }
  else if ( IsWindows7OrGreater() )
  {
    return WV_WIN7;
  }
  else if ( IsWindowsXPOrGreater() )
  {
    return WV_WINXP;
  }
#else

#if VER_PRODUCTBUILD >= 9600
  // should be >= Windows 8.1 SDK code goes here
  if ( IsWindows10OrGreater() )
  {
    return WV_WIN10;
  }
  else if ( IsWindows7OrGreater() )
  {
    return WV_WIN7;
  }
  else if ( IsWindows7OrGreater() )
  {
    return WV_WIN7;
  }
  else if ( IsWindowsXPOrGreater() )
  {
    return WV_WINXP;
  }
#else
  OSVERSIONINFO   osInfo;

  osInfo.dwOSVersionInfoSize = sizeof( osInfo );
  GetVersionEx( &osInfo );

  if ( osInfo.dwPlatformId == VER_PLATFORM_WIN32s )
  {
    return WV_WIN311;
  }
  else if ( osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
  {
    if ( ( osInfo.dwMajorVersion == 4 )
    &&   ( osInfo.dwMinorVersion == 0 ) )
    {
    }
    else if ( ( osInfo.dwMajorVersion == 4 )
    &&        ( osInfo.dwMinorVersion == 10 ) )
    {
      return WV_WIN98;
    }
    else if ( ( osInfo.dwMajorVersion == 4 )
    &&        ( osInfo.dwMinorVersion == 90 ) )
    {
      return WV_WINME;
    }
  }
  else if ( osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
  {
    if ( ( osInfo.dwMajorVersion == 3 )
    &&   ( osInfo.dwMinorVersion == 51 ) )
    {
      return WV_NT351;
    }
    else if ( ( osInfo.dwMajorVersion == 4 )
    &&        ( osInfo.dwMinorVersion == 0 ) )
    {
      return WV_NT4;
    }
    else if ( ( osInfo.dwMajorVersion == 5 )
    &&        ( osInfo.dwMinorVersion == 0 ) )
    {
      return WV_WIN2000;
    }
    else if ( ( osInfo.dwMajorVersion == 5 )
    &&        ( osInfo.dwMinorVersion == 1 ) )
    {
      return WV_WINXP;
    }
    else if ( ( osInfo.dwMajorVersion == 5 )
    &&        ( osInfo.dwMinorVersion == 2 ) )
    {
      return WV_WINSERVER2003;
    }
    else if ( ( osInfo.dwMajorVersion == 6 )
    &&        ( osInfo.dwMinorVersion == 0 ) )
    {
      return WV_VISTA;
    }
    else if ( ( osInfo.dwMajorVersion == 6 )
    &&        ( osInfo.dwMinorVersion == 1 ) )
    {
      return WV_WIN7;
    }
    else if ( ( ( osInfo.dwMajorVersion == 6 )
    &&          ( osInfo.dwMinorVersion >= 1 ) )
    ||        ( osInfo.dwMajorVersion > 6 ) )
    {
      return WV_HIGHER_THAN_KNOWN;
    }
  }
#endif

#endif
  return WV_UNKNOWN;
}



bool Win::Util::GetVersionInfo( HINSTANCE hInstance, GR::u32 ResourceID, GR::u32& Major, GR::u32& Minor,
                                GR::u32& BuildNumber, GR::u32& SubBuild, const GR::String& VersionType )
{
  HRSRC   hrSrc = FindResource( hInstance, MAKEINTRESOURCE( ResourceID ), RT_VERSION );
  if ( hrSrc == NULL )
  {
    return false;
  }
  HGLOBAL hGlobal = LoadResource( hInstance, hrSrc );
  if ( hGlobal == NULL )
  {
    return false;
  }

  BYTE*   pBlock = (BYTE*)LockResource( hGlobal );

  GR::u32   Size = SizeofResource( hInstance, hrSrc );

  BYTE*   pData = new BYTE[Size * 2];

  memset( pData, 0, Size * 2 );

  memcpy( pData, pBlock, Size );

  struct LANGANDCODEPAGE {
    WORD wLanguage;
    WORD wCodePage;
  } * lpTranslate;

  UINT    cbTranslate;


  // Read the list of languages and code pages.
  VerQueryValueW( pData,
                  L"\\VarFileInfo\\Translation",
                  (LPVOID*)&lpTranslate,
                  &cbTranslate );

  // Read the file description for each language and code page.
  GR::WChar    SubBlock[50];

  for( size_t i = 0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
  {
    wsprintfW( SubBlock,
              L"\\StringFileInfo\\%04x%04x\\%s",
              lpTranslate[i].wLanguage,
              lpTranslate[i].wCodePage,
              GR::Convert::ToUTF16( VersionType ).c_str() );

    // Retrieve file description for language and code page "i".
    LPVOID     pBuffer = NULL;

    GR::u32     Length = 0;

    VerQueryValueW( pData,
                  SubBlock,
                  &pBuffer,
                  (PUINT)&Length );

    GR::String strVersion = GR::Convert::ToUTF8( GR::WString( ( GR::WChar* )pBuffer, Length ) );

    strVersion = GR::Strings::Remove( strVersion, ' ' );
    strVersion = GR::Strings::Remove( strVersion, 0 );

    char          cSeparator = '.';
    if ( strVersion.find( cSeparator ) == GR::String::npos )
    {
      cSeparator = ',';
    }

    if ( strVersion.find( cSeparator ) != GR::String::npos )
    {
      Major = GR::Convert::ToU32( strVersion.substr( 0, strVersion.find( cSeparator ) ) );
      strVersion = strVersion.substr( strVersion.find( cSeparator ) + 1 );
    }
    if ( strVersion.find( cSeparator ) != GR::String::npos )
    {
      Minor = GR::Convert::ToU32( strVersion.substr( 0, strVersion.find( cSeparator ) ) );
      strVersion = strVersion.substr( strVersion.find( cSeparator ) + 1 );
    }
    if ( strVersion.find( cSeparator ) != GR::String::npos )
    {
      BuildNumber = GR::Convert::ToU32( strVersion.substr( 0, strVersion.find( cSeparator ) ) );
      strVersion = strVersion.substr( strVersion.find( cSeparator ) + 1 );
  }
    if ( !strVersion.empty() )
    {
      SubBuild = GR::Convert::ToU32( strVersion );
    }
  }

  delete[] pData;

  if ( hGlobal != NULL )
  {
    UnlockResource( hGlobal );
    FreeResource( hGlobal );
  }

  return true;
}



#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
BOOL Win::Util::BitmapToIconInfoEx( HDC hdcRef, HBITMAP hBmpSrc, ICONINFO* pii, COLORREF crTransparent )
{

  COLORREF      crBkgnd,
                crText;

  BITMAP        bm;

  HDC           hdc1,
                hdc2;


  GetObject( hBmpSrc, sizeof( bm ), &bm );

  hdc1          = CreateCompatibleDC( hdcRef );
  hdc2          = CreateCompatibleDC( hdcRef );
  pii->fIcon    = TRUE;
  pii->xHotspot = 0;
  pii->yHotspot = 0;
  HBITMAP hbmOld  = (HBITMAP)SelectObject( hdc1, CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL ) );
  HBITMAP hbmOldColor = (HBITMAP)SelectObject( hdc2, CopyImage( hBmpSrc, IMAGE_BITMAP,
                                                          bm.bmWidth, bm.bmHeight, LR_CREATEDIBSECTION ) );

  // Die Transparente Farbe liegt nun als Parameter vor
  crBkgnd       = SetBkColor( hdc2, crTransparent );

  BitBlt( hdc1, 0, 0, bm.bmWidth, bm.bmHeight, hdc2, 0, 0, SRCCOPY );

  SetBkColor( hdc2, RGB( 0x00, 0x00, 0x00 ) );
  crText = SetTextColor( hdc2, RGB( 0xFF, 0xFF, 0xFF ) );

  BitBlt( hdc2, 0, 0, bm.bmWidth, bm.bmHeight, hdc1, 0, 0, SRCAND );

  SetTextColor( hdc2, crText );
  SetBkColor( hdc2, crBkgnd );

  pii->hbmColor = (HBITMAP)SelectObject( hdc2, hbmOldColor );
  pii->hbmMask  = (HBITMAP)SelectObject( hdc1, hbmOld );

  DeleteDC( hdc2 );
  DeleteDC( hdc1 );

  return TRUE;
}
#endif



HICON Win::Util::BitmapToIconEx( HDC hdcRef, HBITMAP hBitmap, COLORREF crTransparent )
{
#if ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE ) )
  return NULL;
#else
  ICONINFO    ii;

  HICON       hIcon;


  if ( !BitmapToIconInfoEx( hdcRef, hBitmap, &ii, crTransparent ) )
  {
    return NULL;
  }

  hIcon = CreateIconIndirect( &ii );

  DeleteObject( (HGDIOBJ)ii.hbmMask );
  DeleteObject( (HGDIOBJ)ii.hbmColor );

  return hIcon;
#endif
}



HRGN Win::Util::BitmapToRegion( HBITMAP hBmp, COLORREF cTransparentColor )
{

  HRGN hRgn = NULL;

  if ( hBmp == NULL )
  {
    return NULL;
  }

  // Create a memory DC inside which we will scan the bitmap content
  HDC hMemDC = CreateCompatibleDC( NULL );

  if ( hMemDC == NULL )
  {
    return NULL;
  }

  // Get bitmap size
  BITMAP      bm;

  GetObject( hBmp, sizeof( bm ), &bm );

  // Create a 32 bits depth bitmap and select it into the memory DC
  BITMAPINFOHEADER RGB32BITSBITMAPINFO =
  {
    sizeof(BITMAPINFOHEADER), // biSize
    bm.bmWidth,         // biWidth;
    bm.bmHeight,        // biHeight;
    1,              // biPlanes;
    32,             // biBitCount
    BI_RGB,           // biCompression;
    0,              // biSizeImage;
    0,              // biXPelsPerMeter;
    0,              // biYPelsPerMeter;
    0,              // biClrUsed;
    0             // biClrImportant;
  };

  void*   pbits32;

  HBITMAP hbm32 = CreateDIBSection( hMemDC, (BITMAPINFO*)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0 );

  if ( hbm32 == NULL )
  {
    DeleteDC( hMemDC );
    return NULL;
  }

  HBITMAP holdBmp = (HBITMAP)SelectObject( hMemDC, hbm32 );

  // Create a DC just to copy the bitmap into the memory DC
  HDC hDC = CreateCompatibleDC( hMemDC );
  if ( hDC )
  {
    // Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
    BITMAP    bm32;

    GetObject( hbm32, sizeof( bm32 ), &bm32 );

    while ( bm32.bmWidthBytes % 4 )
    {
      bm32.bmWidthBytes++;
    }

    // Copy the bitmap into the memory DC
    HBITMAP holdBmp = (HBITMAP)SelectObject( hDC, hBmp );

    BitBlt( hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY );

    // For better performances, we will use the ExtCreateRegion() function to create the
    // region. This function take a RGNDATA structure on entry. We will add rectangles by
    // amount of ALLOC_UNIT number in this structure.

    GR::u32 maxRects = 100;
    HANDLE hData = GlobalAlloc( GMEM_MOVEABLE, sizeof( RGNDATAHEADER ) + ( sizeof( RECT ) * maxRects ) );
    if ( hData == NULL )
    {
      return NULL;
    }
    RGNDATA* pData = (RGNDATA*)GlobalLock( hData );
    if ( pData == NULL )
    {
      GlobalFree( hData );
      return NULL;
    }
    pData->rdh.dwSize = sizeof( RGNDATAHEADER );
    pData->rdh.iType  = RDH_RECTANGLES;
    pData->rdh.nCount = pData->rdh.nRgnSize = 0;
    SetRect( &pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0 );

    // Keep on hand highest and lowest values for the "transparent" pixels

    // Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
    BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
    for (int y = 0; y < bm.bmHeight; y++)
    {
      // Scan each bitmap pixel from left to right
      for (int x = 0; x < bm.bmWidth; x++)
      {
        // Search for a continuous range of "non transparent pixels"
        int x0 = x;
        LONG *p = (LONG *)p32 + x;
        while (x < bm.bmWidth)
        {
          if ( *p == cTransparentColor )
          {
            // This pixel is "transparent"
            break;
          }
          p++;
          x++;
        }

        if (x > x0)
        {
          // Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
          if (pData->rdh.nCount >= maxRects)
          {
            GlobalUnlock(hData);
            maxRects += 100;
            hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
            pData = (RGNDATA *)GlobalLock(hData);
          }
          RECT *pr = (RECT *)&pData->Buffer;
          SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
          if (x0 < pData->rdh.rcBound.left)
            pData->rdh.rcBound.left = x0;
          if (y < pData->rdh.rcBound.top)
            pData->rdh.rcBound.top = y;
          if (x > pData->rdh.rcBound.right)
            pData->rdh.rcBound.right = x;
          if (y+1 > pData->rdh.rcBound.bottom)
            pData->rdh.rcBound.bottom = y+1;
          pData->rdh.nCount++;

          // On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
          // large (ie: > 4000). Therefore, we have to create the region by multiple steps.
          if (pData->rdh.nCount == 2000)
          {
            HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
            if (hRgn)
            {
              CombineRgn(hRgn, hRgn, h, RGN_OR);
              DeleteObject(h);
            }
            else
              hRgn = h;
            pData->rdh.nCount = 0;
            SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
          }
        }
      }

      // Go to next row (remember, the bitmap is inverted vertically)
      p32 -= bm32.bmWidthBytes;
    }

    // Create or extend the region with the remaining rectangles
    HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
    if (hRgn)
    {
      CombineRgn(hRgn, hRgn, h, RGN_OR);
      DeleteObject(h);
    }
    else
      hRgn = h;

    // Clean up
    SelectObject(hDC, holdBmp);
    DeleteDC(hDC);

    GlobalUnlock( hData );
    GlobalFree( hData );
  }

  DeleteObject(SelectObject(hMemDC, holdBmp));
  DeleteDC( hMemDC );

  return hRgn;

}



GR::String Win::Util::BrowseForFolder( HWND hwndParent, const GR::Char* DisplayText )
{
  GR::WChar      szTemp[65536];

  szTemp[0] = 0;

  BROWSEINFOW   bi;
  LPITEMIDLIST  lpiil;

  bi.hwndOwner        = hwndParent;
  bi.pidlRoot         = NULL;
  bi.pszDisplayName   = szTemp;
  bi.lpszTitle        = GR::Convert::ToUTF16( DisplayText ).c_str();
  bi.ulFlags          = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
  bi.lpfn             = NULL;
  bi.lParam           = 0;
  bi.iImage           = 0;

  lpiil = SHBrowseForFolderW( &bi );

  if ( lpiil != NULL )
  {
    SHGetPathFromIDListW( lpiil, szTemp );

    if ( szTemp[wcslen( szTemp ) - 1] != 92 )
    {
      wsprintfW( szTemp, L"%s\\", szTemp );
    }
    LPMALLOC    lpMalloc;
    if ( SUCCEEDED( SHGetMalloc( &lpMalloc ) ) )
    {
      lpMalloc->Free( lpiil );
    }
  }
  return GR::Convert::ToUTF8( szTemp );
}



MemoryStream Win::Util::MemoryStreamFromResource( HINSTANCE hInstance, const GR::Char* Resource, const GR::Char* ResourceType )
{
  HRSRC hrHandle = NULL;
  GR::WString     utf16ResourceType = GR::Convert::ToUTF16( ResourceType );


  if ( IS_INTRESOURCE( Resource ) )
  {
    // this is a resource int identifier, not a real string
    hrHandle = FindResourceW( hInstance, (LPCWSTR)Resource, utf16ResourceType.c_str() );
  }
  else
  {
    GR::WString     utf16Resource = GR::Convert::ToUTF16( Resource );

    hrHandle = FindResourceW( hInstance, utf16Resource.c_str(), utf16ResourceType.c_str() );
  }
  if ( hrHandle == NULL )
  {
    return MemoryStream();
  }
  HGLOBAL globHandle = LoadResource( hInstance, hrHandle );
  if ( globHandle == NULL )
  {
    return MemoryStream();
  }
  return MemoryStream( LockResource( globHandle ), SizeofResource( hInstance, hrHandle ) );
}



bool Win::Util::IsUserAdmin()
{
#if VER_PRODUCTBUILD < 9600
  if ( GetVersion() >= 0x80000000 )
  {
    // nicht NT
    return true;
  }
#endif

  HANDLE hAccessToken       = NULL;
  PBYTE  pInfoBuffer        = NULL;
  DWORD  InfoBufferSize   = 1024;
  PTOKEN_GROUPS ptgGroups   = NULL;
  PSID   psidAdministrators = NULL;
  SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
  BOOL   bResult = FALSE;

  bool   bIsAdmin = false;

  // init security token
  if ( !AllocateAndInitializeSid( &siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &psidAdministrators ) )
  {
    return false;
  }

    // for Windows NT 4.0 only
  if ( !OpenProcessToken( GetCurrentProcess(),TOKEN_QUERY,&hAccessToken ) )
  {
    FreeSid( psidAdministrators );
    return false;
  }

  do
  {
    if ( pInfoBuffer )
    {
      delete[] pInfoBuffer;
    }
    pInfoBuffer = new BYTE[InfoBufferSize];
    if ( !pInfoBuffer )
    {
      FreeSid( psidAdministrators );
      return false;
    }
    SetLastError( 0 );
    if ( ( !GetTokenInformation( hAccessToken,
                               TokenGroups,
                               pInfoBuffer,
                               InfoBufferSize,
                               &InfoBufferSize ) )
    &&    ( ERROR_INSUFFICIENT_BUFFER != GetLastError() ) )
    {
      FreeSid( psidAdministrators );
      return false;
    }
    else
    {
      ptgGroups = (PTOKEN_GROUPS)pInfoBuffer;
    }
  }
  while ( GetLastError() );

  for ( UINT i = 0; i < ptgGroups->GroupCount; i++ )
  {
    if ( EqualSid( psidAdministrators, ptgGroups->Groups[i].Sid ) )
    {
      bIsAdmin = true;
      break;
    }
  }

  if ( hAccessToken )
  {
    CloseHandle( hAccessToken );
  }
  if ( pInfoBuffer )
  {
    delete[] pInfoBuffer;
  }
  if ( psidAdministrators )
  {
    FreeSid( psidAdministrators );
  }

  return bIsAdmin;

}



bool Win::Util::SleepMsg( GR::u32 Timeout )
{
  DWORD Start = GetTickCount();

  DWORD Elapsed = 0;

  while ( ( Elapsed = GetTickCount() - Start ) < Timeout )
  {
    DWORD Status = MsgWaitForMultipleObjects( 0, NULL, TRUE,
                                                Timeout - Elapsed,
                                                QS_ALLINPUT );
    if ( Status == WAIT_OBJECT_0 )
    {
      MSG msg;

      while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
      {
        if ( msg.message == WM_QUIT )
        {
          // abandoned due to WM_QUIT
          PostQuitMessage( (int)msg.wParam );
          return false;
        }
        if ( !CallMsgFilter( &msg, WH_MSGFILTER ) )
        {
          TranslateMessage( &msg );
          DispatchMessage( &msg );
        }
      }
    }
  }
  return true;
}



namespace
{

  #define IS_WIN30_DIB(lpbi)  ((*(LPORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

  WORD DIBNumColors( BYTE* lpDIB )
  {
   WORD wBitCount;  /* DIB bit count */

   /*  If this is a Windows-style DIB, the number of colors in the
    *  color table can be less than the number of bits per pixel
    *  allows for (i.e. lpbi->biClrUsed can be set to some value).
    *  If this is the case, return the appropriate value.
    */

   if ( (*(LPDWORD)( lpDIB ) ) == sizeof( BITMAPINFOHEADER ) )
   {
      DWORD ClrUsed;

      ClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
      if (ClrUsed)
     return (WORD)ClrUsed;
   }

   /*  Calculate the number of colors in the color table based on
    *  the number of bits per pixel for the DIB.
    */
   if ( (*(LPDWORD)( lpDIB ) ) == sizeof( BITMAPINFOHEADER ) )
      wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
   else
      wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

   /* return number of colors based on bits per pixel */
   switch (wBitCount)
      {
   case 1:
      return 2;

   case 4:
      return 16;

   case 8:
      return 256;

   default:
      return 0;
      }
}

WORD PaletteSize(BYTE* lpDIB)
{
   WORD size;

   /* calculate the size required by the palette */
   if ( (*(LPDWORD)( lpDIB ) ) == sizeof( BITMAPINFOHEADER ) )
   {
      size = (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
   }
   else
   {
      size = (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
   }

   return size;
}
};


GR::Graphic::ImageData Win::Util::CreateImageFromHDIB( HGLOBAL hmem )
{

  if ( hmem == NULL )
  {
    return GR::Graphic::ImageData();
  }

  BITMAPINFOHEADER *pBIH;

  pBIH = (BITMAPINFOHEADER*)GlobalLock(hmem);

  BYTE    *pData = ( (BYTE*)pBIH ) + pBIH->biSize + PaletteSize( (BYTE*)pBIH );

  DWORD   Mask[3];

  memset( Mask, 0, sizeof( Mask ) );

  if ( pBIH->biCompression == BI_BITFIELDS )
  {
    // dann sind noch 3 ORDs (die Masken) mit drin
    Mask[0] = ( (DWORD*)pData )[0];
    Mask[1] = ( (DWORD*)pData )[1];
    Mask[2] = ( (DWORD*)pData )[2];
    pData += 12;

    /*
    dh::Log( "Masks %x  %x  %x\n",
        Mask[0], Mask[1], Mask[2] );
    */
  }


  GR::Graphic::ImageData      ImageData;

  switch ( pBIH->biBitCount )
  {
    case 1:
      {
        int             i,
                        j,
                        iLO;



        iLO = pBIH->biWidth / 8;
        if ( pBIH->biWidth & 7 )
        {
          iLO++;
        }
        if ( iLO % 4 )
        {
          iLO += ( 4 - iLO % 4 );
        }

        int   iWidth = pBIH->biWidth,
              iHeight = pBIH->biHeight;

        ImageData.CreateData( iWidth, iHeight, GR::Graphic::IF_MONOCHROME );
        for ( j = 0; j < abs( iHeight ); j++ )
        {
          for ( i = 0; i < ( iWidth + 7 ) / 8; i++ )
          {
            if ( iHeight < 0 )
            {
              ( (BYTE*)ImageData.Data() )[i + j * ImageData.LineOffsetInBytes()] = pData[i + j * iLO];
            }
            else
            {
              ( (BYTE*)ImageData.Data() )[i + ( abs( iHeight ) - j - 1 ) * ImageData.LineOffsetInBytes()] = pData[i + j * iLO];
            }
          }
        }

        // Palette holen
        {
          RGBQUAD                 *bmiColor;

          bmiColor = (RGBQUAD*)( (BYTE*)pBIH + pBIH->biSize );

          for ( int i = 0; i < 2; i++ )
          {
            ImageData.Palette().SetColor( i, bmiColor[i].rgbRed, bmiColor[i].rgbGreen, bmiColor[i].rgbBlue );
          }
        }
      }
      break;
    case 4:
      {
        int             i,
                        j,
                        iLO;



        iLO = pBIH->biWidth / 2;
        if ( pBIH->biWidth & 1 )
        {
          iLO++;
        }
        if ( iLO % 4 )
        {
          iLO += ( 4 - iLO % 4 );
        }

        int   iWidth = pBIH->biWidth,
              iHeight = pBIH->biHeight;

        ImageData.CreateData( iWidth, iHeight, GR::Graphic::IF_INDEX4 );
        for ( j = 0; j < abs( iHeight ); j++ )
        {
          for ( i = 0; i < ( iWidth + 1 ) / 2; i++ )
          {
            if ( iHeight < 0 )
            {
              ( (BYTE*)ImageData.Data() )[i + j * ImageData.LineOffsetInBytes()] = pData[i + j * iLO];
            }
            else
            {
              ( (BYTE*)ImageData.Data() )[i + ( abs( iHeight ) - j - 1 ) * ImageData.LineOffsetInBytes()] = pData[i + j * iLO];
            }
          }
        }

        // Palette holen
        {
          RGBQUAD                 *bmiColor;

          bmiColor = (RGBQUAD*)( (BYTE*)pBIH + pBIH->biSize );

          for ( int i = 0; i < 16; i++ )
          {
            ImageData.Palette().SetColor( i, bmiColor[i].rgbRed, bmiColor[i].rgbGreen, bmiColor[i].rgbBlue );
          }
        }
      }
      break;
    case 8:
      {
        int             i,
                        j,
                        iDummy,
                        iLO;



        iLO = pBIH->biWidth;
        if ( iLO % 4 )
        {
          iLO += ( 4 - iLO % 4 );
        }

        iDummy = 256;

        int   iWidth = pBIH->biWidth,
              iHeight = pBIH->biHeight;

        ImageData.CreateData( iWidth, iHeight, GR::Graphic::IF_PALETTED );
        for ( j = 0; j < abs( iHeight ); j++ )
        {
          for ( i = 0; i < iWidth; i++ )
          {
            if ( iHeight < 0 )
            {
              ( (BYTE*)ImageData.Data() )[i + j * iWidth] = pData[i + j * iLO];
            }
            else
            {
              ( (BYTE*)ImageData.Data() )[i + ( abs( iHeight ) - j - 1 ) * iWidth] = pData[i + j * iLO];
            }
          }
        }

        // Palette holen
        {
          RGBQUAD                 *bmiColor;

          bmiColor = (RGBQUAD*)( (BYTE*)pBIH + pBIH->biSize );

          for ( int i = 0; i < 256; i++ )
          {
            ImageData.Palette().SetColor( i, bmiColor[i].rgbRed, bmiColor[i].rgbGreen, bmiColor[i].rgbBlue );
          }
        }
      }
      break;
    case 16:
      {
        int             i,
                        j,
                        iLO;



        iLO = pBIH->biWidth * 2;
        if ( iLO % 4 )
        {
          iLO += ( 4 - iLO % 4 );
        }

        int   iWidth = pBIH->biWidth,
              iHeight = pBIH->biHeight;

        ImageData.CreateData( iWidth, iHeight, GR::Graphic::IF_X1R5G5B5 );

        WORD    wDummy;

        for ( j = 0; j < abs( iHeight ); j++ )
        {
          for ( i = 0; i < iWidth; i++ )
          {
            wDummy = ( (WORD*)pData )[i + j * iLO / 2];

            if ( Mask[0] == 0xf800 )
            {
              // von 16 auf 15 bit runter
              wDummy = ( ( wDummy & 0xf800 ) >> 1 ) + ( ( wDummy & 0x7c0 ) >> 1 ) + ( wDummy & 0x1f );
            }

            if ( iHeight < 0 )
            {
              *( (WORD*)ImageData.Data() + i + j * iWidth ) = wDummy;
            }
            else
            {
              *( (WORD*)ImageData.Data() + i + abs( iHeight - j - 1 ) * iWidth ) = wDummy;
            }
          }
        }
      }
      break;
    case 24:
      {
        int             i,
                        j,
                        iLO;



        iLO = pBIH->biWidth * 3;
        if ( iLO % 4 )
        {
          iLO += ( 4 - iLO % 4 );
        }

        int   iWidth = pBIH->biWidth,
              iHeight = pBIH->biHeight;

        ImageData.CreateData( iWidth, iHeight, GR::Graphic::IF_R8G8B8 );

        for ( j = 0; j < abs( iHeight ); j++ )
        {
          for ( i = 0; i < iWidth; i++ )
          {
            if ( iHeight < 0 )
            {
              ( (BYTE*)ImageData.Data() )[( i + j * iWidth ) * 3 + 0] = pData[i * 3 + j * iLO + 0];
              ( (BYTE*)ImageData.Data() )[( i + j * iWidth ) * 3 + 1] = pData[i * 3 + j * iLO + 1];
              ( (BYTE*)ImageData.Data() )[( i + j * iWidth ) * 3 + 2] = pData[i * 3 + j * iLO + 2];
            }
            else
            {
              ( (BYTE*)ImageData.Data() )[( i + abs( iHeight - j - 1 ) * iWidth ) * 3 + 0] = pData[i * 3 + j * iLO + 0];
              ( (BYTE*)ImageData.Data() )[( i + abs( iHeight - j - 1 ) * iWidth ) * 3 + 1] = pData[i * 3 + j * iLO + 1];
              ( (BYTE*)ImageData.Data() )[( i + abs( iHeight - j - 1 ) * iWidth ) * 3 + 2] = pData[i * 3 + j * iLO + 2];
            }
          }
        }
      }
      break;
    case 32:
      {
        int             i,
                        j,
                        iLO;



        iLO = pBIH->biWidth * 4;

        int   iWidth = pBIH->biWidth,
              iHeight = pBIH->biHeight;

        ImageData.CreateData( iWidth, iHeight, GR::Graphic::IF_X8R8G8B8 );

        DWORD   Dummy;

        for ( j = 0; j < abs( iHeight ); j++ )
        {
          for ( i = 0; i < iWidth; i++ )
          {
            Dummy = ( (DWORD*)pData )[i + j * iLO / 4];
            if ( iHeight < 0 )
            {
              ( (DWORD*)ImageData.Data() )[i + j * iWidth] = Dummy;
            }
            else
            {
              ( (DWORD*)ImageData.Data() )[i + abs( iHeight - j - 1 ) * iWidth] = Dummy;
            }
          }
        }
      }
      break;
    default:
      // nicht unterstützte Farbtiefe
      {
        dh::Log( "HDIB unsupported %d", pBIH->biBitCount );
      }
      break;
  }
  return ImageData;

}



HANDLE Win::Util::CreateHDIBFromImage( const GR::Graphic::ImageData& Image )
{

  BITMAPINFOHEADER    bi;
  BITMAPINFOHEADER*   lpbi;
  DWORD               Len;
  HANDLE              hDIB = NULL, h = NULL;

  if ( ( Image.BitsProPixel() != 1 )
  &&   ( Image.BitsProPixel() != 2 )
  &&   ( Image.BitsProPixel() != 4 )
  &&   ( Image.BitsProPixel() != 8 )
  &&   ( Image.BitsProPixel() != 15 )
  &&   ( Image.BitsProPixel() != 16 )
  &&   ( Image.BitsProPixel() != 24 )
  &&   ( Image.BitsProPixel() != 32 ) )
  {
    // not supported depth
    return NULL;
  }

  bi.biSize = sizeof( BITMAPINFOHEADER );
  bi.biWidth    = Image.Width();
  bi.biHeight   = Image.Height();
  bi.biPlanes   = 1;
  bi.biBitCount = Image.BitsProPixel();
  if ( bi.biBitCount == 15 )
  {
    bi.biBitCount = 16;
  }
  bi.biCompression = BI_RGB;

  bi.biSizeImage = ( ( ( (DWORD)bi.biWidth * bi.biBitCount ) + 31 ) / 32 * 4 ) * bi.biHeight;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  /* calculate size of memory block required to store BITMAPINFO */
  Len = bi.biSize + PaletteSize( (BYTE*)&bi );

  hDIB = GlobalAlloc(GHND, Len);
  if ( hDIB == NULL )
  {
    // uh oh
    return NULL;
  }

  /* realloc the buffer big enough to hold all the bits */
  Len = bi.biSize + PaletteSize((BYTE*)&bi) + bi.biSizeImage;

  h = GlobalReAlloc( hDIB, Len, 0 );
  if ( h != NULL )
  {
    hDIB = h;
  }
  else
  {
    GlobalFree( hDIB );
    hDIB = NULL;
    return NULL;
  }

  /* lock memory block and get pointer to it */

  lpbi = (BITMAPINFOHEADER*)GlobalLock(hDIB);
  if ( lpbi == NULL )
  {
    GlobalFree( hDIB );
    hDIB = NULL;
    return NULL;
  }

  // Daten in den Puffer kopieren
  *lpbi = bi;

  // Bild-Daten kopieren
  switch ( bi.biBitCount )
  {
    case 1:
      {
        // Palette in DC setzen
        if ( Image.Palette().Entries() )
        {
          RGBQUAD                 *bmiColor;

          bmiColor = (RGBQUAD*)( (BYTE*)lpbi + lpbi->biSize );

          for ( int i = 0; i < 2; i++ )
          {
            bmiColor[i].rgbRed    = Image.Palette().Red( i );
            bmiColor[i].rgbGreen  = Image.Palette().Green( i );
            bmiColor[i].rgbBlue   = Image.Palette().Blue( i );
            bmiColor[i].rgbReserved = 0;
          }
        }

        BYTE    *pData = (BYTE*)lpbi + lpbi->biSize + PaletteSize( (BYTE*)lpbi );

        int iLO = Image.Width() / 8;
        if ( Image.Width() & 7 )
        {
          iLO++;
        }
        if ( iLO % 4 )
        {
          iLO += 4 - ( iLO % 4 );
        }
        GR::Graphic::ContextDescriptor    cdImage( Image );
        GR::Graphic::ContextDescriptor    cdTarget;

        cdTarget.Attach( cdImage.Width(), cdImage.Height(), iLO, cdImage.ImageFormat(), pData );

        for ( int j = 0; j < Image.Height(); j++ )
        {
          cdTarget.HLine( 0, cdTarget.Width() - 1, j, 1 );
          cdTarget.HLine( 1, cdTarget.Width() - 2, j, 0 );
          //cdImage.CopyLine( 0, j, cdImage.GetWidth(), 0, cdImage.GetHeight() - j - 1, &cdTarget );
        }
      }
      break;
    case 4:
      {
        // Palette in DC setzen
        if ( Image.HasPaletteData() )
        {
          RGBQUAD                 *bmiColor;

          bmiColor = (RGBQUAD*)( (BYTE*)lpbi + lpbi->biSize );

          for ( int i = 0; i < 16; i++ )
          {
            bmiColor[i].rgbRed    = Image.Palette().Red( i );
            bmiColor[i].rgbGreen  = Image.Palette().Green( i );
            bmiColor[i].rgbBlue   = Image.Palette().Blue( i );
            bmiColor[i].rgbReserved = 0;
          }
        }

        BYTE    *pData = (BYTE*)lpbi + lpbi->biSize + PaletteSize( (BYTE*)lpbi );

        int iLO = Image.Width() / 2;
        if ( Image.Width() & 1 )
        {
          iLO++;
        }
        if ( iLO % 4 )
        {
          iLO += 4 - ( iLO % 4 );
        }
        GR::Graphic::ContextDescriptor    cdImage( Image );
        GR::Graphic::ContextDescriptor    cdTarget;

        cdTarget.Attach( cdImage.Width(), cdImage.Height(), iLO, cdImage.ImageFormat(), pData );

        for ( int j = 0; j < Image.Height(); j++ )
        {
          cdImage.CopyLine( 0, j, cdImage.Width(), 0, cdImage.Height() - j - 1, &cdTarget );
        }
      }
      break;
    case 8:
      {
        // Palette in DC setzen
        if ( Image.Palette().Entries() )
        {
          RGBQUAD                 *bmiColor;

          bmiColor = (RGBQUAD*)( (BYTE*)lpbi + lpbi->biSize );

          for ( int i = 0; i < 256; i++ )
          {
            bmiColor[i].rgbRed    = Image.Palette().Red( i );
            bmiColor[i].rgbGreen  = Image.Palette().Green( i );
            bmiColor[i].rgbBlue   = Image.Palette().Blue( i );
            bmiColor[i].rgbReserved = 0;
          }
        }

        BYTE    *pData = (BYTE*)lpbi + lpbi->biSize + PaletteSize( (BYTE*)lpbi );

        int iLO = Image.Width();
        if ( iLO % 4 )
        {
          iLO += 4 - ( iLO % 4 );
        }
        for ( int j = 0; j < Image.Height(); j++ )
        {
          for ( int i = 0; i < Image.Width(); i++ )
          {
            ((unsigned char*)pData)[i + ( Image.Height() - j - 1 ) * iLO] = (BYTE)( (BYTE*)Image.Data() )[i + j * Image.Width()];
          }
        }
      }
      break;
    case 16:
      {
        BYTE    *pData = (BYTE*)lpbi + lpbi->biSize + PaletteSize( (BYTE*)lpbi );

        int iLO = Image.Width() * 2;
        if ( iLO % 4 )
        {
          iLO += 4 - ( iLO % 4 );
        }
        for ( int j = 0; j < Image.Height(); j++ )
        {
          for ( int i = 0; i < Image.Width(); i++ )
          {
            ((WORD*)pData)[i + ( Image.Height() - j - 1 ) * iLO / 2] = (WORD)*( (WORD*)Image.Data() + i + j * Image.Width() );
          }
        }
      }
      break;
    case 24:
      {
        BYTE    *pData = (BYTE*)lpbi + lpbi->biSize + PaletteSize( (BYTE*)lpbi );

        int iLO = Image.Width() * 3;
        if ( iLO % 4 )
        {
          iLO += 4 - ( iLO % 4 );
        }
        for ( int j = 0; j < Image.Height(); j++ )
        {
          for ( int i = 0; i < Image.Width(); i++ )
          {
            ((BYTE*)pData)[3 * i + ( Image.Height() - j - 1 ) * iLO] = (BYTE)*( (BYTE*)Image.Data() + 3 * ( i + j * Image.Width() ) );
            ((BYTE*)pData)[3 * i + ( Image.Height() - j - 1 ) * iLO + 1] = (BYTE)*( (BYTE*)Image.Data() + 3 * ( i + j * Image.Width() ) + 1 );
            ((BYTE*)pData)[3 * i + ( Image.Height() - j - 1 ) * iLO + 2] = (BYTE)*( (BYTE*)Image.Data() + 3 * ( i + j * Image.Width() ) + 2 );
          }
        }
      }
      break;
    case 32:
      {
        BYTE    *pData = (BYTE*)lpbi + lpbi->biSize + PaletteSize( (BYTE*)lpbi );

        int iLO = Image.Width();
        for ( int j = 0; j < Image.Height(); j++ )
        {
          for ( int i = 0; i < Image.Width(); i++ )
          {
            ((DWORD*)pData)[i + ( Image.Height() - j - 1 ) * iLO] = (DWORD)*( (DWORD*)Image.Data() + i + j * Image.Width() );
          }
        }
      }
      break;
    default:
      dh::Log( "CreateHDIBFromImage unsupported depth %d\n", bi.biBitCount );
      break;
  }

  bi = *lpbi;

  /* clean up */
  GlobalUnlock(hDIB);

  /* return handle to the DIB */
  return hDIB;

}



bool Win::Util::IsRemoteDesktopSession()
{
#if WINVER >= 0x0500
#ifndef SM_REMOTESESSION
  #define SM_REMOTESESSION 0x1000
#endif
  if ( GetSystemMetrics( SM_REMOTESESSION ) )
  {
    return true;
  }
#endif
  // nabbed from MSDN, some "optimization" may return false even if inside remote session
  bool  isRemoteSession = false;
  HKEY  hRegKey = NULL;
  LONG  result = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\" ), 0, KEY_READ, &hRegKey );
  if ( result == ERROR_SUCCESS )
  {
    DWORD glassSessionId;
    DWORD glassSessionIdSize = sizeof( glassSessionId );
    DWORD type;

    result = RegQueryValueEx( hRegKey, _T( "GlassSessionId" ), NULL, &type, (BYTE*)&glassSessionId, &glassSessionIdSize );
    if ( result == ERROR_SUCCESS )
    {
      DWORD currentSessionId;

      if ( ProcessIdToSessionId( GetCurrentProcessId(), &currentSessionId ) )
      {
        isRemoteSession = ( currentSessionId != glassSessionId );
      }
    }
  }
  if ( hRegKey )
  {
    RegCloseKey( hRegKey );
  }
  return isRemoteSession;
}



