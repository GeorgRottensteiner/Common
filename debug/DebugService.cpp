#include <IO/FileStream.h>

#include <Misc/Misc.h>

#include <debug/debugclient.h>

#include <stdio.h>
#include <stdarg.h>

#include "DebugService.h"



void DebugService::LogEnable( const GR::String& System, bool Enable )
{
  m_mapLogSystems[System].m_Enabled = Enable;
}



bool DebugService::LogEnabled( const GR::String& System )
{
  tMapLogSystems::const_iterator    it( m_mapLogSystems.find( System ) );
  if ( it == m_mapLogSystems.end() )
  {
    return false;
  }
  return it->second.m_Enabled;
}



void DebugService::LogToFile( const GR::String& System, bool Enable )
{
  m_mapLogSystems[System].m_ToFile = Enable;
}



bool DebugService::LogToFileEnabled( const GR::String& System )
{
  tMapLogSystems::iterator    it( m_mapLogSystems.find( System ) );
  if ( it == m_mapLogSystems.end() )
  {
    return false;
  }
  return it->second.m_ToFile;
}



void DebugService::LogDirect( const GR::String& System, const GR::String& Text )
{
  if ( LogEnabled( System ) )
  {
    dh::Log( Text.c_str() );
  }
  if ( LogToFileEnabled( System ) )
  {
    GR::IO::FileStream     ioLog;

    if ( ioLog.Open( CMisc::AppPath( "log.txt" ).c_str(), IIOStream::OT_WRITE_APPEND ) )
    {
      ioLog.WriteLine( Text );
      ioLog.Close();
    }
  }
}



void DebugService::Log( const GR::String& System, const GR::Char* Format, ... )
{
  if ( LogEnabled( System ) )
  {
    static GR::Char    szMiscBuffer[5000];
#if ( ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_WEB ) )
    vsprintf( szMiscBuffer, Format, (char *)( &Format + 1 ) );
#elif ( OPERATING_SYSTEM == OS_ANDROID )
    va_list args;
    va_start( args, Format );
    vsprintf( szMiscBuffer, Format, args );
    va_end( args );
#else
    va_list args;

    va_start( args, Format );
    vsprintf_s( szMiscBuffer, 5000, Format, args );
    va_end( args );
#endif
    LogDirect( System, szMiscBuffer );
  }
}



