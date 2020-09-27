#ifndef CLIPBOARD_HELPER_INCLUDED
#define CLIPBOARD_HELPER_INCLUDED


#include <OS/OS.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <windows.h>
#if OPERATING_SYSTEM_SUB != OS_SUB_UNIVERSAL_APP
#include <WinSys/WinUtils.h>
#endif
#endif

#include <String/Convert.h>

#include <Grafik/ImageData.h>



namespace System
{
  namespace Clipboard
  {
    static bool     TextToClipboard( const GR::String& strText )
    {
#if OPERATING_SYSTEM == OS_WINDOWS
#if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
      if ( !OpenClipboard( NULL ) )
      {
        return false;
      }

      HGLOBAL clipbuffer;

      EmptyClipboard();

#ifdef UNICODE
        
      GR::Char* buffer;
  		  
		  clipbuffer = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, 2 * ( strText.length() + 1 ) );
      buffer = (GR::Char*)GlobalLock( clipbuffer );
      memcpy( buffer, strText.c_str(), 2 * ( strText.length() + 1 ) );
		  GlobalUnlock( clipbuffer );

      HANDLE  hHandle = SetClipboardData( CF_UNICODETEXT, clipbuffer );
#else
		  char * buffer;
		  clipbuffer = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strText.length() + 1 );
		  buffer = (char*)GlobalLock( clipbuffer );
      strcpy_s( buffer, strText.length() + 1, strText.c_str() );
		  GlobalUnlock( clipbuffer );

      HANDLE  hHandle = SetClipboardData( CF_TEXT, clipbuffer );
#endif

		  CloseClipboard();

      return ( hHandle != NULL );
#else
      return true;
#endif
#else
      return false;
#endif
    }



    static bool     TextFromClipboard( GR::String& strTarget )
    {
#if OPERATING_SYSTEM == OS_WINDOWS
#if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
      strTarget.erase();
      if ( !IsClipboardFormatAvailable( CF_TEXT ) ) 
      {
        return false;
      }

      if ( !OpenClipboard( NULL ) )
      {
        return false;
      }

      GR::Char* cBuffer;

#ifdef UNICODE
      HANDLE      hData = GetClipboardData( CF_UNICODETEXT );
#else
      HANDLE      hData = GetClipboardData( CF_TEXT );
#endif
      cBuffer = (GR::Char*)GlobalLock( hData );
      if ( cBuffer == NULL )
      {
        GlobalUnlock( hData );
        CloseClipboard();
        return false;
      }

      strTarget = cBuffer;

      GlobalUnlock( hData );
	    CloseClipboard();

      return true;
#else
      return false;
#endif
#else
      return false;
#endif
    }



    static GR::Graphic::ImageData ImageFromClipboard()
    {
#if OPERATING_SYSTEM == OS_WINDOWS
#if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
      if ( !OpenClipboard( NULL ) ) 
      { 
        return GR::Graphic::ImageData();
      }
      HANDLE hBitmap = GetClipboardData( CF_DIB );
      GR::Graphic::ImageData    Img = Win::Util::CreateImageFromHDIB( hBitmap );
      CloseClipboard(); 
      return Img;
#else
      return GR::Graphic::ImageData();
#endif
#else
      return GR::Graphic::ImageData();
#endif
    }



    static bool ImageToClipboard( const GR::Graphic::ImageData& Image )
    {
#if OPERATING_SYSTEM == OS_WINDOWS
#if ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
      if ( !::OpenClipboard( NULL ) )
      {
        return false;
      }
      EmptyClipboard();

      HANDLE  hDib = Win::Util::CreateHDIBFromImage( Image );

      if ( hDib == NULL )
      {
        CloseClipboard();
        return false;
      }

      if ( SetClipboardData( CF_DIB, hDib ) == NULL )
      {
        CloseClipboard();
        return false;
      }

      CloseClipboard();
      return true;
#else
      return false;
#endif
#else
      return false;
#endif
    }

  }

}

#endif // _CLIPBOARD_HELPER_INCLUDED_