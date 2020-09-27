#include <OS/OS.h>



#ifdef _WIN32
#include <windows.h>
#endif

#if OPERATING_SYSTEM == OS_TANDEM
#if OPERATING_SUB_SYSTEM == OS_SUB_OSS
#define _XOPEN_SOURCE_EXTENDED 1
#include <unistd.h>
#endif
#include <cextdecs.h>
#endif


#include <stdio.h>

#include <String/Convert.h>
#include <String/Path.h>

#include <DateTime/DateTime.h>

#include <cwchar>

#include "Misc.h"



GR::String CMisc::AppPath( const GR::WChar* formatstr, ... )
{
  static GR::WChar          buffer[3000];

  static GR::String     storedAppPath;

  static bool               appPathSet = false;

#if OPERATING_SYSTEM == OS_WINDOWS
  vswprintf( buffer, 3000, formatstr, (char *)( &formatstr + 1 ) );
#else
  va_list args;
  va_start( args, formatstr );

  vswprintf ( buffer, 3000, formatstr, args );

  va_end( args );
#endif
  if ( appPathSet )
  {
    return storedAppPath + GR::Convert::ToUTF8( buffer );
  }

#ifdef GR_APP_ALTERNATIVE_APP_PATH
  storedAppPath = GR_APP_ALTERNATIVE_APP_PATH;
#else

#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) && ( OPERATING_SYSTEM == OS_WINDOWS ) )
  GR::WChar          tempBuffer[65536];
  GetModuleFileNameW( NULL, tempBuffer, 65536 );
  while ( tempBuffer[wcslen( tempBuffer ) - 1] != '\\' )
  {
    tempBuffer[wcslen( tempBuffer ) - 1] = 0;
  }

#ifndef GR_APP_PATH_UNTOUCHED
  if ( wcslen( tempBuffer ) >= 8 )
  {
    // falls wir im RELEASE-Verzeichnis sind, das RELEASE wegmachen
    // (faule-Programmierer-Aktion)
    if ( ( toupper( tempBuffer[wcslen( tempBuffer ) - 8] ) == 'R' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 7] ) == 'E' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 6] ) == 'L' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 5] ) == 'E' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 4] ) == 'A' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 3] ) == 'S' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 2] ) == 'E' ) )
    {
      tempBuffer[wcslen( tempBuffer ) - 1] = 1;
      while ( tempBuffer[wcslen( tempBuffer ) - 1] != 92 )
      {
        tempBuffer[wcslen( tempBuffer ) - 1] = 0;
      }
    }
  }
  if ( wcslen( tempBuffer ) >= 6 )
  {
    // falls wir im DEBUG-Verzeichnis sind, das DEBUG wegmachen
    // (faule-Programmierer-Aktion)
    if ( ( toupper( tempBuffer[wcslen( tempBuffer ) - 6] ) == 'D' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 5] ) == 'E' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 4] ) == 'B' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 3] ) == 'U' )
      && ( toupper( tempBuffer[wcslen( tempBuffer ) - 2] ) == 'G' ) )
    {
      tempBuffer[wcslen( tempBuffer ) - 1] = 1;
      while ( tempBuffer[wcslen( tempBuffer ) - 1] != 92 )
      {
        tempBuffer[wcslen( tempBuffer ) - 1] = 0;
      }
    }
  }
#endif // GR_APP_PATH_UNTOUCHED
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
  using namespace Windows::Storage;

  StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;

  Platform::String^   fullPath = localFolder->Path;

  storedAppPath = GR::Convert::ToUTF8( fullPath->Data() );
#elif ( ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN ) )
  char programfile[64];

  short processhandle = 0;
  short programfilelen = 0;

  short retcode = PROCESSHANDLE_GETMINE_( &processhandle );
  if ( retcode == 0 )
  {
    retcode = PROCESS_GETINFO_( &processhandle,,,,,,,,,,,,,,programfile,64,&programfilelen );
    if ( retcode == 0 )
    {
      storedAppPath = programfile;
      appPathSet = true;
    }
  }
#elif ( ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_OSS ) )
  char programfile[64];

  short processhandle = 0;
  short programfilelen = 0;

  short retcode = PROCESSHANDLE_GETMINE_( &processhandle );
  if ( retcode == 0 )
  {
    retcode = PROCESS_GETINFO_( &processhandle,,,,,,,,,,,,,,programfile,64,&programfilelen );
    if ( retcode == 0 )
    {
      char    ossPathBuf[2048];
      short   ossPathLen = 0;

      retcode = FILENAME_TO_PATHNAME_( programfile,
                                       programfilelen,
                                       ossPathBuf,
                                       2048,
                                       &ossPathLen );
      if ( retcode == 0 )
      {
        storedAppPath = Path::AddBackslash( Path::RemoveFileSpec( ossPathBuf ) );
        appPathSet = true;
      }
    }
  }
#elif OPERATING_SYSTEM == OS_ANDROID
  extern GR::String     s_AndroidInternalDataPath;

  storedAppPath = s_AndroidInternalDataPath + "/";
  appPathSet = true;
#elif OPERATING_SYSTEM == OS_WEB
  // TODO ?
  storedAppPath = "/";
  appPathSet = true;
#else
  // unixoid
  char    temp[2048];

  readlink( "/proc/self/exe", temp, 2048 );
  storedAppPath = temp + GR::String( "/" );
  appPathSet = true;
#endif
#endif // GR_APP_ALTERNATIVE_APP_PATH
  return storedAppPath + GR::Convert::ToUTF8( buffer );
}



GR::String CMisc::AppPath( const GR::Char* formatstr, ... )
{
  static GR::Char         buffer[3000];

  static GR::String   storedAppPath;

  static bool             appPathSet = false;

#if OPERATING_SYSTEM == OS_WINDOWS
  vsprintf_s( buffer, 3000, formatstr, (char *)( &formatstr + 1 ) );
#else
  va_list args;
  va_start( args, formatstr );

  vsprintf( buffer, formatstr, args );

  va_end( args );
#endif

  if ( appPathSet )
  {
    return Path::Append( storedAppPath, buffer );
  }

#ifdef GR_APP_ALTERNATIVE_APP_PATH
  storedAppPath = GR_APP_ALTERNATIVE_APP_PATH;
#else
#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) && ( OPERATING_SYSTEM == OS_WINDOWS ) )
  GR::WChar            tempBuffer[65536];
  GetModuleFileNameW( NULL, tempBuffer, 65536 );

  while ( tempBuffer[wcslen( tempBuffer ) - 1] != 92 )
  {
    tempBuffer[wcslen( tempBuffer ) - 1] = 0;
  }

#ifndef GR_APP_PATH_UNTOUCHED
  if ( wcslen( tempBuffer ) >= 8 )
  {
    // falls wir im RELEASE-Verzeichnis sind, das RELEASE wegmachen
    // (faule-Programmierer-Aktion)
    if ( ( toupper( tempBuffer[wcslen( tempBuffer ) - 8] ) == 'R' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 7] ) == 'E' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 6] ) == 'L' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 5] ) == 'E' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 4] ) == 'A' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 3] ) == 'S' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 2] ) == 'E' ) )
    {
      tempBuffer[wcslen( tempBuffer ) - 1] = 1;
      while ( tempBuffer[wcslen( tempBuffer ) - 1] != 92 )
      {
        tempBuffer[wcslen( tempBuffer ) - 1] = 0;
      }
    }
  }
  if ( wcslen( tempBuffer ) >= 6 )
  {
    // falls wir im DEBUG-Verzeichnis sind, das DEBUG wegmachen
    // (faule-Programmierer-Aktion)
    if ( ( toupper( tempBuffer[wcslen( tempBuffer ) - 6] ) == 'D' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 5] ) == 'E' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 4] ) == 'B' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 3] ) == 'U' )
    &&   ( toupper( tempBuffer[wcslen( tempBuffer ) - 2] ) == 'G' ) )
    {
      tempBuffer[wcslen( tempBuffer ) - 1] = 1;
      while ( tempBuffer[wcslen( tempBuffer ) - 1] != 92 )
      {
        tempBuffer[wcslen( tempBuffer ) - 1] = 0;
      }
    }
  }
#endif // GR_APP_PATH_UNTOUCHED
  storedAppPath = GR::Convert::ToUTF8( tempBuffer );
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
  using namespace Windows::Storage;

  StorageFolder^ localFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

  Platform::String^   fullPath = localFolder->Path;
  storedAppPath = GR::Convert::ToUTF8( fullPath->Data() );
#elif ( ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN ) )
  char programfile[64];

  short processhandle = 0;
  short programfilelen = 0;

  short retcode = PROCESSHANDLE_GETMINE_( &processhandle );
  if ( retcode == 0 )
  {
    retcode = PROCESS_GETINFO_( &processhandle,,,,,,,,,,,,,,programfile,64,&programfilelen );
    if ( retcode == 0 )
    {
      // programfile is NOT zero ended
      storedAppPath.assign( programfile, programfilelen );
      storedAppPath = Path::RemoveFileSpec( storedAppPath );
      appPathSet = true;
    }
  }
#elif ( ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_OSS ) )
  char programfile[64];

  short processhandle = 0;
  short programfilelen = 0;

  short retcode = PROCESSHANDLE_GETMINE_( &processhandle );
  if ( retcode == 0 )
  {
    retcode = PROCESS_GETINFO_( &processhandle,,,,,,,,,,,,,,programfile,64,&programfilelen );
    if ( retcode == 0 )
    {
      char    ossPathBuf[2048];
      short   ossPathLen = 0;

      retcode = FILENAME_TO_PATHNAME_( programfile,
                                       programfilelen,
                                       ossPathBuf,
                                       2048,
                                       &ossPathLen );
      if ( retcode == 0 )
      {
        storedAppPath = Path::AddBackslash( Path::RemoveFileSpec( ossPathBuf ) );
        appPathSet = true;
      }
    }
  }
#elif OPERATING_SYSTEM == OS_ANDROID
  extern GR::String     s_AndroidInternalDataPath;;

  storedAppPath = s_AndroidInternalDataPath + "/";
  appPathSet = true;
#elif OPERATING_SYSTEM == OS_WEB
  // TODO ?
  storedAppPath = "/";
  appPathSet = true;
#else
  // unixoid / OSS
  char    temp[2048];

  readlink( "/proc/self/exe", temp, 2048 );
  storedAppPath = temp + GR::String( "/" );
  appPathSet = true;
#endif
#endif // GR_APP_ALTERNATIVE_APP_PATH
  return Path::Append( storedAppPath, buffer );
}



const GR::Char* CMisc::printf( const GR::Char* lpszFormat, ... )
{
  static GR::Char         buffer[3000];

#if OPERATING_SYSTEM == OS_WINDOWS
  vsprintf_s( buffer, 3000, lpszFormat, (char *)( &lpszFormat + 1 ) );
#else
  va_list args;
  va_start( args, lpszFormat );

  vsprintf( buffer, lpszFormat, args );

  va_end( args );
#endif

  return buffer;
}



void CMisc::FillValue( GR::u8* pDestination, unsigned long Width, unsigned char Value )
{
  memset( pDestination, Value, Width );

  /*
  _asm
  {
    mov     edi, pDestination
    mov     al, ucValue
    mov     ah, ucValue
    shl     eax, 16
    mov     ah, ucValue
    mov     al, ah
    mov     ecx, ulWidth
    shr     ecx, 1
    jnc     NO_BYTE
    stosb
NO_BYTE:
    shr     ecx, 1
    jnc     NO_WORD
    stosw
NO_WORD:
    rep stosd
  }
  */

}



void CMisc::FillValue16( GR::u16* pDestination, unsigned long Width, GR::u16 Value )
{
  GR::u16*    pDest = pDestination;
  for ( unsigned long i = 0; i < Width / 2; ++i )
  {
    *pDest = Value;
    ++pDest;
  }
  /*
  _asm
  {
    mov     edi, pDestination
    mov     ax, wValue
    shl     eax, 16
    mov     ax, wValue
    mov     ecx, ulWidth
    shr     ecx, 1
    jnc     NO_BYTE
    stosb
NO_BYTE:
    shr     ecx, 1
    jnc     NO_WORD
    stosw
NO_WORD:
    rep stosd
  }*/

}



void CMisc::FillValue24( GR::u32* pDest, unsigned long Width, GR::u32 Value )
{
  GR::u32   value1,
            value2,
            value3;

  GR::u8*   pDestination = ( GR::u8* )pDest;

  if ( Width >= 12 )
  {
    value1 =   ( Value & 0xffffff )         + ( ( Value & 0xff ) << 24 );
    value2 = ( ( Value & 0xffff ) << 16 )   + ( ( Value & 0xffff00 ) >> 8 );
    value3 = ( ( Value & 0xffffff ) << 8 )  + ( ( Value & 0xff0000 ) >> 16 );
    while ( Width >= 12 )
    {
      *(GR::u32 *)pDestination = value1;
      pDestination += 4;
      *(GR::u32 *)pDestination = value2;
      pDestination += 4;
      *(GR::u32 *)pDestination = value3;
      pDestination += 4;

      Width -= 12;
    }
    Width %= 12;
  }
  while ( Width > 0 )
  {
    *( (GR::u8*)( pDestination )++ ) = (GR::u8)( Value & 0xff );
    *( (GR::u8*)( pDestination )++ ) = (GR::u8)( ( Value & 0xff00 ) >> 8 );
    *( (GR::u8*)( pDestination )++ ) = (GR::u8)( ( Value & 0xff0000 ) >> 16 );
    Width -= 3;
  }
}



void CMisc::FillValue32( GR::u32* pDestination, unsigned long Width, GR::u32 Value )
{
  GR::u32*    pDest = pDestination;
  for ( unsigned long i = 0; i < Width / 4; ++i )
  {
    *pDest = Value;
    ++pDest;
  }

  /*
  _asm
  {
    mov     edi, pDestination
    mov     eax, dwValue
    mov     ecx, ulWidth
    shr     ecx, 2
    rep stosd
  }*/

}



const GR::Char* CMisc::TimeStamp()
{
  GR::DateTime::DateTime    today = GR::DateTime::DateTime::Now();

  static GR::Char          buffer[1000];

#if OPERATING_SYSTEM == OS_WINDOWS
  sprintf_s( buffer, 1000, "<%02d.%02d.%04d-%02d:%02d:%02d:%03d>",
             today.Day(),
             today.Month(),
             today.Year(),
             today.Hour(),
             today.Minute(),
             today.Second(),
             today.MicroSecond() / 1000 );
#else
  sprintf( buffer, "<%02d.%02d.%04d-%02d:%02d:%02d:%03d>",
           today.Day(),
           today.Month(),
           today.Year(),
           today.Hour(),
           today.Minute(),
           today.Second(),
           today.MicroSecond() * 1000 );
#endif

  return buffer;
}



