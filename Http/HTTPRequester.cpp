#include <HTTP/HTTPRequester.h>
#include <HTTP/SSLConnection.h>

#include <WinSys/RegistryHelper.h>

#include <Codec/Base64.h>

#include <debug/debugclient.h>

#include <Misc/Misc.h>


CHTTPRequester::CHTTPRequester() :
  m_pSSLConnection( NULL ),
  m_bSSLWithProxyWaitForConnection( false )
{

  m_Connection.AddListener( this );

}



CHTTPRequester::~CHTTPRequester()
{

  SafeDelete( m_pSSLConnection );

}


bool CHTTPRequester::Request( const GR::String& strAddress, const WORD wPort )
{

  CHTTPBody       RequestBody;

  m_bSSLWithProxyWaitForConnection = false;
  if ( strAddress.find( "https://" ) == 0 )
  {
    m_pSSLConnection = new CSSLConnection();
    if ( m_Connection.UsingProxy() )
    {
      RequestBody.Command( "CONNECT" );
      m_bSSLWithProxyWaitForConnection = true;
    }
    else
    {
      m_pSSLConnection->SSLHandShake( m_Connection, ByteBuffer() );
    }
  }
  else
  {
    RequestBody.Command( "GET" );
  }

  return m_Connection.Request( RequestBody, strAddress, wPort );

}



bool CHTTPRequester::Post( const GR::String& strAddress, const WORD wPort )
{

  CHTTPBody       RequestBody;

  m_Connection.AddListener( this );

  m_bSSLWithProxyWaitForConnection = false;
  if ( strAddress.find( "https://" ) == 0 )
  {
    m_pSSLConnection = new CSSLConnection();
    if ( m_Connection.UsingProxy() )
    {
      RequestBody.Command( "CONNECT" );
      m_bSSLWithProxyWaitForConnection = true;
    }
    else
    {
      m_pSSLConnection->SSLHandShake( m_Connection, ByteBuffer() );
    }
  }
  else
  {
    RequestBody.Command( "POST" );
  }

  size_t    iSeparatorPos = strAddress.find( "?" );
  if ( iSeparatorPos != GR::String::npos )
  {
    GR::String   strParams = strAddress.substr( iSeparatorPos + 1 );

    GR::String   strAddressArg = strAddress.substr( 0, iSeparatorPos );

    RequestBody.SetBody( strParams.c_str(), strParams.length() );

    return m_Connection.Request( RequestBody, strAddressArg, wPort );
  }

  return m_Connection.Request( RequestBody, strAddress, wPort );

}



bool CHTTPRequester::ProcessEvent( const CHTTPBody& BodyAnswer )
{

  if ( ( m_pSSLConnection )
  &&   ( !m_bSSLWithProxyWaitForConnection ) )
  {
    m_pSSLConnection->SSLHandShake( m_Connection, ByteBuffer( BodyAnswer.Body(), BodyAnswer.BodySize() ) );
    return true;
  }
  switch ( BodyAnswer.AnswerCode() )
  {
    case 302:
      {
        // Redirect
        //dh::Log( "HTTPRedirect to %s", BodyAnswer.Line( "Location" ).c_str() );

        CHTTPBody   RequestBody;

        RequestBody.Command( "GET" );
        m_Connection.Request( RequestBody, BodyAnswer.Line( "Location" ) );
      }
      return true;
    default:
    case 200:     // OK
      if ( m_bSSLWithProxyWaitForConnection )
      {
        m_pSSLConnection->SSLHandShake( m_Connection, ByteBuffer( BodyAnswer.Body(), BodyAnswer.BodySize() ) );
        return true;
      }
      if ( !m_Connection.UsingSSL() )
      {
        SendEvent( BodyAnswer );
        return true;
      }
      return true;
      break;
    case 401:     // unauthorized
    case 404:     // not found
    case 502:     // Proxy Error
      // FehlerMeldung
      SendEvent( BodyAnswer );
      return true;
  }

  //SendEvent( BodyAnswer );

  return false;

}



void CHTTPRequester::ConnectionConfig( const GR::String& strProxy, const WORD wProxyPort,
                                       const GR::String& strProxyUser, const GR::String& strProxyPassword )
{

  m_Connection.ConnectionConfig( strProxy, wProxyPort, strProxyUser, strProxyPassword );

}



bool CHTTPRequester::ConnectionConfigFromIERegistry()
{
  GR::String    proxy;

  DWORD   dwProxyEnable = 0;

  if ( !Registry::GetKey( HKEY_CURRENT_USER,
                                 "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                                 "ProxyEnable",
                                 dwProxyEnable ) )
  {
    return false;
  }
  if ( dwProxyEnable == 0 )
  {
    m_Connection.ConnectionConfig();
    return false;
  }

  if ( !Registry::GetKey( HKEY_CURRENT_USER,
                                 "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                                 "ProxyServer",
                                 proxy ) )
  {
    return false;
  }

  GR::String strProxy = proxy;

  size_t      iDoppelpunktPos = strProxy.find( ':' );

  if ( iDoppelpunktPos == GR::String::npos )
  {
    // kein Port drin
    m_Connection.ConnectionConfig( strProxy, 8080 );
  }
  else
  {
    m_Connection.ConnectionConfig( strProxy.substr( 0, iDoppelpunktPos ),
                                   atoi( strProxy.substr( iDoppelpunktPos + 1 ).c_str() ) );
  }

  return true;

}



GR::String CHTTPRequester::ProxyServer() const
{

  return m_Connection.ProxyServer();

}



WORD CHTTPRequester::ProxyPort() const
{

  return m_Connection.ProxyPort();

}



bool CHTTPRequester::Abort()
{

  m_Connection.Cancel();
  return true;

}