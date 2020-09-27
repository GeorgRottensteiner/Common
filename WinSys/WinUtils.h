#ifndef WINUTILS_H
#define WINUTILS_H



#include <windows.h>
#include <tchar.h>

#include <list>
#include <string>

#include <Memory/MemoryStream.h>

#include <Grafik/ImageData.h>



namespace Win
{
  namespace Util
  {
    enum eWindowsVersion
    {
      WV_UNKNOWN,
      WV_WIN311,
      WV_WIN95,
      WV_WIN98,
      WV_WINME,
      WV_NT351,
      WV_NT4,
      WV_WIN2000,
      WV_WINXP,
      WV_WINSERVER2003,
      WV_VISTA,
      WV_WIN7,
      WV_HIGHER_THAN_KNOWN
    };

    std::list<HWND>      EnumWindows();

    BOOL CALLBACK        EnumWindowProc( HWND hwnd, LPARAM lParam );

    HWND                 FindWindowWhichContains( const GR::Char* Text );

    GR::String       BrowseForFolder( HWND hwndParent, const GR::Char* DisplayText );

    HICON                CreateGrayscaleIcon( HICON hIcon, bool bFlat = false );

    GR::u32              InternetExplorerVersion();

    GR::u32              GetDXVersion();

    eWindowsVersion      GetWindowsVersion();

    bool                 StartProgram( const GR::Char* CommandLine, int iShowWindowFlags = SW_SHOW );

    bool                 StartAndWaitForProgram( const GR::Char* CommandLine, GR::u32* pExitCode = NULL, int iShowWindowFlags = SW_SHOW );

    GR::String       LoadStringEx( GR::u32 ResourceId, GR::u16 wLanguage );

    GR::String       LoadString( GR::u32 ResourceId );

    bool                 GetVersionInfo( HINSTANCE hInstance, GR::u32 ResourceID, GR::u32& dwMajor, GR::u32& dwMinor, 
                                         GR::u32& dwBuildNumber, GR::u32& dwSubBuild, const GR::String& VersionType = "FileVersion" );

    HICON                BitmapToIconEx( HDC hdcRef, HBITMAP hBitmap, COLORREF crTransparent );

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    BOOL                 BitmapToIconInfoEx( HDC hdcRef, HBITMAP hBmpSrc, ICONINFO* pii, COLORREF crTransparent );
#endif

    HRGN                 BitmapToRegion( HBITMAP hBmp, COLORREF cTransparentColor = 0 );

    MemoryStream         MemoryStreamFromResource( HINSTANCE hInstance, const GR::Char* Resource, const GR::Char* ResourceType );

    bool                 IsUserAdmin();

    bool                 SleepMsg( GR::u32 Timeout );

    GR::Graphic::ImageData  CreateImageFromHDIB( HGLOBAL hmem );

    HANDLE               CreateHDIBFromImage( const GR::Graphic::ImageData& Image );

  }
}


#endif // WINUTILS_H



