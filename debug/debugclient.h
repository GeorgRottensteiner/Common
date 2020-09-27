#ifndef _DEBUGCLIENT_TEST_H
#define _DEBUGCLIENT_TEST_H



//- Wenn _DH_LOGGING_DISABLED definiert ist, ist das
//- Loggen abgedreht
#include <string>

#include <Misc/Format.h>
#include <String/Convert.h>

#include <OS/OS.h>

#include <GR/GRTypes.h>



#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn (262)
#endif 

#if ( OPERATING_SUB_SYSTEM != OS_SUB_DESKTOP )
//#pragma message ( "Logging is disabled" )
#define _DH_LOGGING_DISABLED
#endif

#if OPERATING_SYSTEM == OS_ANDROID
#include <android/log.h>
namespace dh
{
  static bool Log( const Misc::CFormat& Format )
  {
    __android_log_print( ANDROID_LOG_INFO, "GRGames", "%s", Format.Result().c_str() );
    return true;
  }

  static bool Log( const GR::Char* szFormat, ... )
  {
    static GR::Char szMiscBuffer[5000];

    va_list args;
    va_start( args, szFormat );

    vsprintf( szMiscBuffer, szFormat, args );

    va_end( args );

    __android_log_print( ANDROID_LOG_INFO, "GRGames", "%s", szMiscBuffer );
    return true;
  }
}
#elif OPERATING_SYSTEM == OS_WEB
namespace dh
{
  static bool Log( const Misc::CFormat& Format )
  {
    printf( "%s\n", Format.Result().c_str() );
    return true;
  }

  static bool Log( const GR::Char* szFormat, ... )
  {
    static GR::Char szMiscBuffer[5000];

    va_list args;
    va_start( args, szFormat );

    vsprintf( szMiscBuffer, szFormat, args );

    va_end( args );

    printf( "%s\n", szMiscBuffer );
    return true;
  }
}
#else

#ifndef _WIN32
namespace dh
{
  static bool Log( const Misc::CFormat& Format )
  {
	  return false;
  }
  
  static bool Log( const GR::Char* szFormat, ... )
  {
	  return false;
  }
}
#else

#include <windows.h>
#include <tchar.h>


#define DH_DEF_NAME     "DebugHost V2"
#define DH_DEF_NAME_W   L"DebugHost V2"
#define DH_DEF_WINID  NULL

namespace dh
{

#ifndef _DH_LOGGING_DISABLED

  enum eLogLevelFlags
  {
    LLF_ALL         = 0xffffffff,
    LLF_WARNING     = 0x00000001,
    LLF_ERROR       = 0x00000002,
    LLF_MEMORY      = 0x00000004,
    LLF_GENERIC     = 0x00000008,
    LLF_DEFAULT     = LLF_WARNING | LLF_ERROR | LLF_GENERIC,
  };

  class Logger
  {
    public:

      GR::u32           m_dwLogLevel;

      GR::String    m_LogPrefix;

      Logger() :
        m_dwLogLevel( LLF_DEFAULT )
      {
      }

      static Logger& Instance()
      {
        static Logger   theLogger;

        return theLogger;
      }

      void ModifyLogLevel( GR::u32 dwAdd, GR::u32 dwRemove = 0 )
      {
        m_dwLogLevel &= ~dwRemove;
        m_dwLogLevel |= dwAdd;
      }

  };

  static int g_iLogLevel = LLF_DEFAULT;

  //- Methoden in aufsteigender Schwierigkeitsstufe:
  //- dem debugger eine nachricht schicken (der entscheidet, was er damit macht - normalerweise loggen)
  inline static bool sendV3( const GR::String& message,
                           HWND myHwnd  = 0,
                           const GR::Char* DHName = "DebugHost V3",
                           const GR::Char* DHWinID = NULL )
  {
    if ( !DHName )
    {
      return false;
    }

    HWND    hwndTarget = ::FindWindowA( DHWinID, DHName );
    if ( hwndTarget == NULL )
    {
      return false;
    }
    COPYDATASTRUCT    cds;
    cds.cbData = (DWORD)message.length() + 1;
    cds.lpData = (void*)message.c_str();
    ::SendMessageA( hwndTarget, WM_COPYDATA, (WPARAM)myHwnd, (LPARAM)&cds );
    return true;
  }

  inline static bool send( const GR::String& message,
                           HWND myHwnd  = 0,
                           const GR::Char* DHName = DH_DEF_NAME,
                           const GR::Char* DHWinID = DH_DEF_WINID )
  {
    if ( !DHName )
    {
      return false;
    }

    HWND    hwndTarget = ::FindWindowA( DHWinID, DHName );
    if ( hwndTarget == NULL )
    {
      return false;
    }
    COPYDATASTRUCT    cds;

    GR::string   strMessage = Logger::Instance().m_LogPrefix + message;
    cds.cbData = (DWORD)strMessage.length() + 1;
    cds.lpData = (void*)strMessage.c_str();
    ::SendMessageA( hwndTarget, WM_COPYDATA, (WPARAM)myHwnd, (LPARAM)&cds );
    return true;
  }


  //- dem debugger eine lognachricht schicken im Georg-Stil
  inline static bool Warning( const GR::Char* szFormat, ... )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_WARNING ) )
    {
      return false;
    }
    static GR::Char szMiscBuffer[5000];

    _vsnprintf_s( szMiscBuffer, 4999, 4999, szFormat, (char *)( &szFormat + 1 ) );
    return send( "[Warning]" + GR::String( szMiscBuffer ) + "\n" );
  }



  inline static bool Memory( const GR::Char* szFormat, ... )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_MEMORY ) )
    {
      return false;
    }
    static GR::Char szMiscBuffer[5000];

    _vsnprintf_s( szMiscBuffer, 4999, 4999, szFormat, (char *)( &szFormat + 1 ) );
    return send( "[Memory]" + GR::String( szMiscBuffer ) + "\n" );
  }



  inline static bool LogFlags( GR::u32 dwCategory, const GR::Char* szFormat, ... )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dwCategory ) )
    {
      return false;
    }
    static GR::Char szMiscBuffer[5000];

    _vsnprintf_s( szMiscBuffer, 4999, 4999, szFormat, (char *)( &szFormat + 1 ) );
    return send( GR::String( szMiscBuffer ) + "\n" );
  }



  //- dem debugger eine lognachricht schicken im Georg-Stil
  inline static bool Error( const GR::Char* szFormat, ... )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_ERROR ) )
    {
      return false;
    }

    static GR::Char    szMiscBuffer[5000];
    _vsnprintf_s( szMiscBuffer, 4999, 4999, szFormat, (char *)( &szFormat + 1 ) );
    return send( "[Error]" + GR::String( szMiscBuffer ) + "\n" );
  }



  //- dem debugger eine lognachricht schicken im Georg-Stil
  inline static bool Log( const GR::Char* szFormat, ... )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_GENERIC ) )
    {
      return false;
    }

    static GR::Char    szMiscBuffer[25000];
    vsprintf_s( szMiscBuffer, 25000, szFormat, (char *)( &szFormat + 1 ) );
    return send( GR::String( szMiscBuffer ) + "\n", 0, DH_DEF_NAME, DH_DEF_WINID );
  }



  static bool Log( const Misc::CFormat& Format )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_GENERIC ) )
    {
      return false;
    }
    return send( Format.Result() + "\n", 0, DH_DEF_NAME, DH_DEF_WINID );
  }



  inline static bool LogNL( const GR::Char* szFormat, ... )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_GENERIC ) )
    {
      return false;
    }
    static GR::Char    szMiscBuffer[5000];
    vsprintf_s( szMiscBuffer, 5000, szFormat, (char*)( &szFormat + 1 ) );
    return send( szMiscBuffer, 0, DH_DEF_NAME, DH_DEF_WINID );
  }



  inline static void SetPrefix( const GR::String& Prefix )
  {
    Logger::Instance().m_LogPrefix = Prefix;
  }



  inline static bool Hex( const void* pData, size_t iLength )
  {
    if ( !( Logger::Instance().m_dwLogLevel & dh::LLF_GENERIC ) )
    {
      return false;
    }

    static GR::Char   szDummy[5000];

    BYTE*   pByte = (BYTE*)pData;

    GR::String    strLine,
                      strLineEnd;

    for ( size_t i = 0; i < iLength; ++i )
    {
      if ( ( i % 16 ) == 0 )
      {
        strLineEnd.erase();
        sprintf_s( szDummy, 5000, "%05d: ", (int)i );
        strLine = szDummy;
      }
      sprintf_s( szDummy, 5000, "%02X ", pByte[i] );
      strLine += szDummy;
      if ( pByte[i] >= 32 )
      {
        strLineEnd += (char)pByte[i];
      }
      else
      {
        strLineEnd += '.';
      }
      if ( ( i % 16 ) == 15 )
      {
        Log( "%s", ( strLine + strLineEnd ).c_str() );
      }
    }
    if ( iLength % 16 )
    {
      for ( size_t i = iLength % 16; i < 16; ++i )
      {
        strLine += "   ";
      }
      Log( "%s", ( strLine + strLineEnd ).c_str() );
    }
    Log( "\n" );
    return true;
  }



#endif//!_DH_LOGGING_DISABLED

  
#ifdef _DH_LOGGING_DISABLED
  inline static bool Log( const GR::Char* szFormat, ... )
  { 
#if ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
    #if defined( DEBUG ) || defined( _DEBUG )
    static GR::Char    szMiscBuffer[25000];
    vsprintf_s( szMiscBuffer, 25000, szFormat, (char *)( &szFormat + 1 ) );

    OutputDebugStringA( szMiscBuffer );
    OutputDebugStringA( "\r\n" );
    //return send( GR::String( szMiscBuffer ) + "\n", 0, DH_DEF_NAME, DH_DEF_WINID );
    #endif // DEBUG
#endif // OS_SUB_WINDOWS_PHONE
    return true; 
  }

  inline static bool Error( const GR::Char* szFormat, ... )
  {
    return true;
  }

  inline static bool Warning( const GR::Char* szFormat, ... )
  {
    return true;
  }

  inline static bool Hex( const void* pData, size_t iLength )
  {
    return true;
  }

#endif // _DH_LOGGING_DISABLED

};

#endif // WIN32

#endif

#endif//_DEBUGCLIENT_TEST_H
