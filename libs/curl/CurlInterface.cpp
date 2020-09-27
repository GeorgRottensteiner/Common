#include <curl/curl.h>

#include <Debug/debugclient.h>

#include ".\CurlInterface.h"

#include <WinSys/RegistryHelper.h>
#include <Net/ProxyDetect.h>

#include <String/Convert.h>



#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Wldap32.lib" )



namespace CurlInterface
{

  int CurlImpl::DebugFunction( CURL*, int, char* Text, size_t Length, void* )
  {
    GR::String     text;

    text.assign( Text, Length );
    dh::Log( "%s", text.c_str() );
    return 0;
  }



  size_t CurlImpl::WriteFunction( void* buf, size_t size, size_t num, void* data )
  {
    CurlImpl*   pImpl = (CurlImpl*)data;
    if ( pImpl )
    {
      pImpl->m_ReceivedData.AppendData( buf, size * num );
    }
    return num;
  }



  bool CurlImpl::Post( const GR::String& URL, const ByteBuffer& PostParams, ByteBuffer& Result, GR::u32& CurlResultCode, bool TrySystemProxy )
  {
    CurlResultCode = 0;
    CURL* curl = curl_easy_init();
    if ( curl == NULL )
    {
      CurlResultCode = -1;
      dh::Log( "curl_easy_init failed" );
      return false;
    }
    curl_version_info_data*   pData = curl_version_info( CURLVERSION_NOW );
    if ( pData->features & CURL_VERSION_SSL )
    {
      // SSL support enabled
      int a = 0;
      dh::Log( "ssl enabled" );
    }
    else
    {
      // No SSL
      int a = 1;
      dh::Log( "No ssl" );
    }
    if ( pData->features & CURL_VERSION_SSPI )
    {
      // SSL support enabled
      int a = 0;
      dh::Log( "sspi enabled" );
    }
    else
    {
      // No SSL
      int a = 1;
      dh::Log( "no sspi" );
    }

    dh::Log( "Set Post Fields (size %d)", PostParams.Size() );

    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteFunction );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );
    curl_easy_setopt( curl, CURLOPT_URL, URL.c_str() );
    curl_easy_setopt( curl, CURLOPT_HTTPAUTH, CURLAUTH_NTLM );
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, 90 );
    curl_easy_setopt( curl, CURLOPT_DEBUGFUNCTION, DebugFunction );
    curl_easy_setopt( curl, CURLOPT_VERBOSE, 1 );

    curl_easy_setopt( curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3 );
    curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );

    if ( TrySystemProxy )
    {
      if ( !SetProxyFromIE( curl, URL ) )
      {
        SetProxyFromSystem( curl, URL );
      }
    }

    curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, PostParams.Size() );
    curl_easy_setopt( curl, CURLOPT_POSTFIELDS, PostParams.Data() );

    CurlResultCode = curl_easy_perform( curl );

    dh::Log( "CurlResultCode = %d", CurlResultCode );

    curl_easy_cleanup( curl );

    Result = m_ReceivedData;


    return ( CurlResultCode == 0 );
  }



  bool CurlImpl::SetProxyFromSystem( CURL* curl, const GR::String& URL )
  {
    GR::String Proxy = GR::ProxyDetect::DetectFromURL( URL );
    if ( Proxy.empty() )
    {
      return false;
    }
    if ( Proxy.find( "PROXY " ) == 0 )
    {
      Proxy = Proxy.substr( 6 );
      Proxy = Proxy.substr( 0, Proxy.find( ';' ) );
      size_t      DoppelpunktPos = Proxy.find( ':' );
      if ( DoppelpunktPos == GR::String::npos )
      {
        // kein Port drin
        curl_easy_setopt( curl, CURLOPT_PROXY, Proxy.c_str() );
        curl_easy_setopt( curl, CURLOPT_PROXYPORT, 8080 );
      }
      else
      {
        curl_easy_setopt( curl, CURLOPT_PROXY, Proxy.substr( 0, DoppelpunktPos ).c_str() );
        curl_easy_setopt( curl, CURLOPT_PROXYPORT, GR::Convert::ToU16( Proxy.substr( DoppelpunktPos + 1 ) ) );
      }
      //curl_easy_setopt( curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY );
      curl_easy_setopt( curl, CURLOPT_PROXYAUTH, CURLAUTH_NTLM );
      curl_easy_setopt( curl, CURLOPT_PROXYUSERPWD, ":" );
      return true;
    }
    return false;
  }



  bool CurlImpl::SetProxyFromIE( CURL* curl, const GR::String& URL )
  {
    GR::String     baseURL;

    char            ProxyArg[MAX_PATH];

    DWORD           ProxyEnable = 0;


    size_t    slashPos = URL.find( "\\" );
    if ( slashPos != GR::String::npos )
    {
      size_t    slashPosAfterDomain = URL.find( "\\" );
      baseURL = URL.substr( slashPos + 2, URL.find( "/" ) );
    }

    if ( !Registry::GetKey( HKEY_CURRENT_USER,
                                  "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                                  "ProxyEnable",
                                  ProxyEnable ) )
    {
      return false;
    }
    if ( ProxyEnable == 0 )
    {
      return false;
    }

    if ( !Registry::GetKey( HKEY_CURRENT_USER,
                                  "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                                  "ProxyServer",
                                  ProxyArg ) )
    {
      return false;
    }

    GR::String Proxy = ProxyArg;

    size_t      DoppelpunktPos = Proxy.find( ':' );
    if ( DoppelpunktPos == GR::String::npos )
    {
      // kein Port drin
      curl_easy_setopt( curl, CURLOPT_PROXY, Proxy.c_str() );
      curl_easy_setopt( curl, CURLOPT_PROXYPORT, 8080 );
    }
    else
    {
      curl_easy_setopt( curl, CURLOPT_PROXY, Proxy.substr( 0, DoppelpunktPos ).c_str() );
      curl_easy_setopt( curl, CURLOPT_PROXYPORT, GR::Convert::ToU16( Proxy.substr( DoppelpunktPos + 1 ) ) );
    }
    return true;
  }



  bool Post( const GR::String& URL, const ByteBuffer& PostParams, ByteBuffer& Result, GR::u32& CurlResultCode )
  {
    CurlImpl      Impl;

    return Impl.Post( URL, PostParams, Result, CurlResultCode );
  }



}
