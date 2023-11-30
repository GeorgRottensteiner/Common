#include <HTTP/HTTPConnection.h>
#include <Codec/Base64.h>

#include <debug/debugclient.h>

#include <Misc/Misc.h>



CHTTPConnection::CHTTPConnection() :
  m_bHeaderRead( FALSE ),
  m_strProxyServer( "" ),
  m_strProxyUser( "" ),
  m_strProxyUserPassword( "" ),
  m_wProxyPort( 0 ),
  m_bUseProxy( false ),
  m_iAnswerCode( 0 ),
  m_dwContentLength( 0 ),
  m_bAutoCloseConnection( true ),
  m_bSSLConnection( false ),
  m_bBinaryDataConnection( false ),
  m_bCancel( false ),
  m_AllowedTimeOuts( 30 ),
  m_CurrentTimeOuts( 0 )
{

  m_iResult         = RES_READY;

  m_Connecter.SetSocketTimeout( 1000 );
  m_Connecter.AddListener( this );

}



CHTTPConnection::~CHTTPConnection()
{

  m_Connecter.Shutdown();
  m_Connecter.ReleaseSocket();

}



bool CHTTPConnection::Request( CHTTPBody& Body, const GR::String& strURL, const WORD wPort )
{

  if ( m_iResult == RES_WAITING )
  {
    Cancel();
  }

  m_bbIncomingData.Clear();
  m_ReceivedBody.Clear();
  m_bHeaderRead = false;

  m_strCurrentHost      = "";
  m_strCurrentProtocol  = "";
  m_strCurrentURLPath   = "";
  m_strCurrentURL       = "";

  m_wCurrentPort        = wPort;

  m_bBinaryDataConnection = false;

  // URL parsen
  size_t          iSlashPos = GR::String::npos;
  GR::String     strTemp   = strURL;

  if ( ( iSlashPos = strTemp.find( "//" ) ) != GR::String::npos )
  {
    m_strCurrentProtocol = strTemp.substr( 0, iSlashPos + 2 );
    strTemp = strTemp.substr( iSlashPos + 2 );
  }
  else
  {
    m_strCurrentProtocol = "http://";
  }
  if ( ( iSlashPos = strTemp.find( '/' ) ) != GR::String::npos )
  {
    m_strCurrentHost    = strTemp.substr( 0, iSlashPos );
    m_strCurrentURLPath = strTemp.substr( iSlashPos + 1 );
  }
  else
  {
    m_strCurrentHost = strTemp;
  }

  m_bSSLConnection = ( m_strCurrentProtocol == "https://" );

  m_strCurrentURL = m_strCurrentProtocol + m_strCurrentHost + '/' + m_strCurrentURLPath;

  // bei HTTPS gibt's einen anderen Port und Handshake vornedran
  if ( m_bSSLConnection )
  {
    m_bAutoCloseConnection = false;
    if ( m_strCurrentURLPath.length() )
    {
      Body.Address( m_strCurrentHost + '/' + m_strCurrentURLPath + ":443" );
    }
    else
    {
      Body.Address( m_strCurrentHost + ":443" );
    }
    /*
    Body.AddLine( "User-Agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; .NET CLR 1.1.4322; .NET CLR 1.0.3705)" );
    Body.AddLine( "Pragma", "no-cache" );
    Body.AddLine( "Host", m_strCurrentHost );
    Body.AddLine( "Content-Length", "0" );
    */
    //Body.Command( "CONNECT" );
  }
  else
  {
    Body.Address( m_strCurrentURL );
  }

  // Verbindung neu aufbauen
  if ( !Connect() )
  {
    return false;
  }

  // Request zusammenbauen
	m_bTempBuffer[0] = 0;

  if ( m_bUseProxy )
  {
    // bei Proxy soll er die Verbindung halten
    Body.AddLine( "Proxy-Connection", "keep-alive" );
    Body.AddLine( "Host", m_strCurrentHost );

    // Proxy-Authorizing
    if ( !m_strProxyUser.empty() )
    {
      GR::String   strCode = m_strProxyUser + ':' + m_strProxyUserPassword;

      GR::String     strResult = "";

      ByteBuffer      Result = Base64::Encode( ByteBuffer( strCode.c_str(), strCode.length() ) );

      strResult = (char*)Result.Data();

      Body.AddLine( "Proxy-Authorization", "Basic " + strResult );
    }
  }
  else
  {
    Body.AddLine( "Connection", "keep-alive" );
  }

  if ( Body.BodySize() )
  {
    Body.AddLine( "content-length", CMisc::printf( "%d", Body.BodySize() ) );
    if ( Body.Command() == "POST" )
    {
      Body.AddLine( "Content-Type", "application/x-www-form-urlencoded" );
    }
  }

  m_SentBody = Body;

  ByteBuffer*   pBlock = Body.CompleteRequest();
  if ( pBlock == NULL )
  {
    m_Connecter.DisconnectWithoutWaiting();

    m_iResult = RES_FAILED;
		strcpy_s( m_bTempBuffer, 16384, "Could not build Request Block" );
    return false;
  }

  //dh::Hex( pBlock->Data(), pBlock->Size() );

  m_iResult = RES_WAITING;

  /*
#ifdef _DEBUG
  dh::Log( "==REQUEST==========\n" );
  //dh::Log( m_bTempBuffer );
  dh::Log( (const char*)pBlock->Data() );
  dh::Log( "==REQUEST=END======\n" );
#endif
  */

  bool Result = SendBinary( pBlock->Data(), pBlock->Size() );

  delete pBlock;

  return Result;

}



void CHTTPConnection::Cancel()
{

  m_iResult = RES_READY;

  m_Connecter.Disconnect();

}



void CHTTPConnection::Ready()
{

  Cancel();

  m_iResult = RES_READY;

}



void CHTTPConnection::ConnectionConfig( const GR::String& strProxy, const WORD wProxyPort,
                                       const GR::String& strProxyUser, const GR::String& strProxyPassword )
{

  m_strProxyServer        = strProxy;
  m_strProxyUser          = strProxyUser;
  m_strProxyUserPassword  = strProxyPassword;
  m_wProxyPort            = wProxyPort;

  m_bUseProxy = !strProxy.empty();

}



int CHTTPConnection::Result()
{

  return m_iResult;

}



int CHTTPConnection::AnswerCode() const
{

  return m_iAnswerCode;

}



const CHTTPBody& CHTTPConnection::SentBody() const
{

  return m_SentBody;

}



void CHTTPConnection::AutoCloseConnection( bool bClose )
{

  m_bAutoCloseConnection = bClose;

}



bool CHTTPConnection::UsingSSL() const
{

  return m_bSSLConnection;

}



bool CHTTPConnection::UsingProxy() const
{

  return m_bUseProxy;

}



bool CHTTPConnection::SendBinary( const void* pData, const size_t iSize, bool bDoNotParseAnswer )
{

  if ( ( pData == NULL )
  ||   ( !m_Connecter.IsValid() ) )
  {
    return false;
  }

  m_bBinaryDataConnection = bDoNotParseAnswer;

  //dh::Log( "Try to send %d Bytes", iSize );
  if ( !m_Connecter.SocketSend( (const char*)pData, (int)iSize ) )
  {
    m_Connecter.DisconnectWithoutWaiting();

    m_iResult = RES_FAILED;
		strcpy_s( m_bTempBuffer, 16384, "Could not send URL request" );
		return false;
	}

  return true;

}



bool CHTTPConnection::Connect()
{

  GR::String       Server;

  if ( m_bUseProxy )
  {
    Server = m_strProxyServer;
    //hostent = gethostbyname( m_strProxyServer.c_str() );
  }
  else
  {
    Server = m_strCurrentHost;
    //hostent = gethostbyname( m_strCurrentHost.c_str() );
  }
  /*
	if ( hostent == NULL )
  {
    m_iResult = RES_FAILED;
		strcpy( m_bTempBuffer, "Host/Proxy was not found." );

    dh::Log( m_bTempBuffer );

    m_SocketHandler.ExitInstance();
		return false;
	}
  */

  GR::u16       Port = m_wCurrentPort;

  if ( m_bUseProxy )
  {
    Port = m_wProxyPort;
  }
  else if ( m_bSSLConnection )
  {
    Port = 443;
  }

  /*
	char *temp = (char *)inet_ntoa( *((struct in_addr *)hostent->h_addr ) );
	int IPInteger = inet_addr( temp );


	server_addr.sin_family      = AF_INET;
  if ( m_bUseProxy )
  {
    server_addr.sin_port        = htons( m_wProxyPort );
  }
  else
  {
    if ( m_bSSLConnection )
    {
      // SSL
      server_addr.sin_port        = htons( 443 );
    }
    else
    {
  	  server_addr.sin_port        = htons( m_wCurrentPort );
    }
  }
  */
  /*
	server_addr.sin_addr.s_addr = IPInteger;
	ZeroMemory( &server_addr.sin_zero, 8 );

  int returncode = connect( m_Socket.Socket(), (struct sockaddr*)&server_addr, sizeof( struct sockaddr ) );
	if ( returncode < 0 )
  {
		if ( WSAGetLastError() == WSAEWOULDBLOCK )
    {
      // das ist sogar in Ordnung (bei blocking)

      // bei blocking auf FD_CONNECT warten!
      Sleep( 500 );
		}
    else
    {
      wsprintf( m_bTempBuffer, "Could not connect to server (%d)", WSAGetLastError() );

      m_Socket.ReleaseSocket();
      m_iResult = RES_FAILED;

      dh::Log( m_bTempBuffer );
      m_SocketHandler.ExitInstance();
		  return false;
    }
	}
  */
  if ( !m_Connecter.Connect( Server, Port ) )
  {
    return false;
  }

  //dh::Log( "HTTP:Connected" );

  return true;

}



GR::String CHTTPConnection::ProxyServer() const
{

  return m_strProxyServer;

}



WORD CHTTPConnection::ProxyPort() const
{

  return m_wProxyPort;

}



bool CHTTPConnection::ProcessEvent( const GR::Net::Socket::tSocketEvent& SocketEvent )
{

  switch ( SocketEvent.m_Type )
  {
    case GR::Net::Socket::tSocketEvent::SE_DATA_RECEIVED:
      m_CurrentTimeOuts = 0;
      m_bbIncomingData.AppendData( SocketEvent.m_pData, SocketEvent.m_dwBytesReceived );
      //dh::Log( "Received %d (have now %d)", SocketEvent.m_dwBytesReceived, m_bbIncomingData.Size() );
      if ( IsBodyComplete( m_bbIncomingData ) )
      {
        //dh::Log( "body complete" );
        m_Connecter.DisconnectWithoutWaiting();

        ProcessData();
        return true;
      }
      /*
      else
      {
        dh::Log( "Not complete" );
      }
      */
      break;
    case GR::Net::Socket::tSocketEvent::SE_CONNECTED:
      //dh::Log( "Connected" );
      break;
    case GR::Net::Socket::tSocketEvent::SE_DISCONNECTED:
      //dh::Log( "Disconnected" );
      if ( m_iResult != RES_SUCCESS )
      {
        m_iResult = RES_FAILED;
        strcpy_s( m_bTempBuffer, 16384, "Connection was closed by host" );

        //dh::Log( "got disconnected - received" );
        //dh::Hex( m_bbIncomingData.Data(), m_bbIncomingData.Size() );


        m_ReceivedBody.AnswerCode( -1 );
        EventProducer<CHTTPBody>::SendEvent( m_ReceivedBody );
      }
      break;
    case GR::Net::Socket::tSocketEvent::SE_TIMEOUT:
      ++m_CurrentTimeOuts;
      //dh::Log( "Timeout tick %d", m_CurrentTimeOuts );
      if ( ( m_CurrentTimeOuts >= m_AllowedTimeOuts )
      ||   ( m_bbIncomingData.Size() ) )
      {
        //dh::Log( "Timeout count reached" );
        ProcessData();
        //dh::Log( "-processed data" );
      }
      break;
  }

  return true;

}



void CHTTPConnection::ProcessData()
{

  bool      bFirstHeaderLine = true;

  // SSL hat keinen Header
  if ( m_bBinaryDataConnection )
  {
    m_bHeaderRead = true;
  }

  int iOffset = 0;
  if ( !m_bHeaderRead )
  {
    char    cDummy;
    for ( size_t i = 0; i < m_bbIncomingData.Size(); i++ )
    {
      cDummy = (char)m_bbIncomingData.ByteAt( i );
      if ( ( cDummy != 13 )
      &&   ( cDummy != 10 ) )
      {
        m_strTemp += cDummy;
      }

      if ( cDummy == 10 )
      {
        // ein Zeilenumbruch
        if ( m_strTemp.length() == 0 )
        {
          // der Header ist fertig
          m_bHeaderRead = TRUE;
          iOffset = i + 1;
          m_bbIncomingData.TruncateFront( i + 1 );
          m_dwContentLength = m_bbIncomingData.Size();
          break;
        }
        // Header auswerten
        if ( m_strTemp.length() )
        {
          if ( bFirstHeaderLine )
          {
            m_ReceivedBody.Command( m_strTemp );
            bFirstHeaderLine = false;
          }
          else
          {
            size_t    iDotPos = m_strTemp.find( ':' );
            if ( iDotPos != GR::String::npos )
            {
              m_ReceivedBody.AddLine( m_strTemp.substr( 0, iDotPos ), m_strTemp.substr( iDotPos + 1 ) );
            }
          }
        }

        //dh::Log( "Header-Zeile: %s", m_strTemp.c_str() );
        if ( m_strTemp.find( "HTTP" ) == 0 )
        {
          // hier steht der Antwort-Code drin
          size_t    iPosCode = m_strTemp.find( " " );
          if ( iPosCode != GR::String::npos )
          {
            size_t  iPosCodeEnde = m_strTemp.find( " ", iPosCode + 1 );
            if ( iPosCodeEnde != GR::String::npos )
            {
              m_ReceivedBody.AnswerCode( atoi( m_strTemp.substr( iPosCode + 1, iPosCodeEnde - iPosCode - 1 ).c_str() ) );
            }
          }
        }

        m_strTemp = "";
      }
    }

    // Content-Length sollte drin sein
    GR::String   strContentLength = m_ReceivedBody.Line( "content-length" );
    if ( strContentLength.length() )
    {
      m_dwContentLength = atoi( strContentLength.c_str() );
    }
  }
  if ( !m_bHeaderRead )
  {
    // diese Daten gehören noch nicht in den Endpuffer
    //dh::Log( "Keine Daten? (Header noch nicht komplett?)" );
    if ( ( m_dwContentLength == 0 )
    &&   ( m_bbIncomingData.Size() == 0 ) )
    {
      /*
      if ( m_bAutoCloseConnection )
      {
        m_Connecter.DisconnectWithoutWaiting();
      }
      m_iResult = RES_FAILED;
      strcpy( m_bTempBuffer, "No answer received" );
      */
    }
    return;
  }

  //dh::Log( "Data Received %d bytes", m_dwContentLength );

  if ( m_dwContentLength != 0 )
  {
    // alles fertig
    m_iResult = RES_SUCCESS;

    if ( m_bAutoCloseConnection )
    {
      m_Connecter.DisconnectWithoutWaiting();
    }

    m_ReceivedBody.SetBody( m_bbIncomingData.Data(), m_bbIncomingData.Size() );

    EventProducer<CHTTPBody>::SendEvent( m_ReceivedBody );
    return;
  }

  if ( ( m_dwContentLength == 0 )
  &&   ( iOffset == m_bbIncomingData.Size() )
  &&   ( !m_bBinaryDataConnection ) )
  {
    // es kam nur ein Header
    //dh::Log( "nur header" );
    m_iResult = RES_SUCCESS;

    if ( m_bAutoCloseConnection )
    {
      m_Connecter.DisconnectWithoutWaiting();
    }

    m_ReceivedBody.SetBody( m_bbIncomingData.Data(), m_bbIncomingData.Size() );

    EventProducer<CHTTPBody>::SendEvent( m_ReceivedBody );
    return;
  }

  if ( ( ( m_bbIncomingData.Size() >= m_dwContentLength )
  &&     ( m_dwContentLength != 0 )
  &&     ( m_bbIncomingData.Size() != 0 ) )
  ||   ( ( m_dwContentLength == 0 )
  &&     ( m_bbIncomingData.Size() == 0 ) ) )
  //&&   ( m_dwContentLength > 0 ) )
  {
    // alles fertig
    //dh::Log( "fertig 2" );
    m_iResult = RES_SUCCESS;

    if ( m_bAutoCloseConnection )
    {
      m_Connecter.DisconnectWithoutWaiting();
    }
    m_ReceivedBody.SetBody( m_bbIncomingData.Data(), m_bbIncomingData.Size() );

    EventProducer<CHTTPBody>::SendEvent( m_ReceivedBody );
    return;
  }

  if ( m_bBinaryDataConnection )
  {
    // Binäre Daten einzeln weiterleiten
    m_ReceivedBody.SetBody( m_bbIncomingData.Data(), m_bbIncomingData.Size() );

    //dh::Log( "Send Binary Answer (%d Bytes)\n", m_dwReceivedBytes );
    EventProducer<CHTTPBody>::SendEvent( m_ReceivedBody );

    m_bbIncomingData.Clear();
  }

}



bool CHTTPConnection::IsBodyComplete( const ByteBuffer& bbIncoming )
{

  DWORD   dwContentLength = 0;

  CHTTPBody   httpTemp;

  char    cDummy;
  int     iOffset = 0;
  bool    bFirstHeaderLine = true;

  GR::String   strTemp;

  for ( size_t i = 0; i < bbIncoming.Size(); i++ )
  {
    cDummy = (char)bbIncoming.ByteAt( i );
    if ( ( cDummy != 13 )
    &&   ( cDummy != 10 ) )
    {
      strTemp += cDummy;
    }

    if ( cDummy == 10 )
    {
      // ein Zeilenumbruch
      if ( strTemp.length() == 0 )
      {
        // der Header ist fertig
        iOffset = i + 1;
        break;
      }
      // Header auswerten
      if ( strTemp.length() )
      {
        if ( bFirstHeaderLine )
        {
          httpTemp.Command( strTemp );
          bFirstHeaderLine = false;
        }
        else
        {
          size_t    iDotPos = strTemp.find( ':' );
          if ( iDotPos != GR::String::npos )
          {
            httpTemp.AddLine( strTemp.substr( 0, iDotPos ), strTemp.substr( iDotPos + 1 ) );
          }
        }
      }

      //dh::Log( "Header-Zeile: %s", m_strTemp.c_str() );
      if ( strTemp.find( "HTTP" ) == 0 )
      {
        // hier steht der Antwort-Code drin
        size_t    iPosCode = strTemp.find( " " );
        if ( iPosCode != GR::String::npos )
        {
          size_t  iPosCodeEnde = strTemp.find( " ", iPosCode + 1 );
          if ( iPosCodeEnde != GR::String::npos )
          {
            httpTemp.AnswerCode( atoi( strTemp.substr( iPosCode + 1, iPosCodeEnde - iPosCode - 1 ).c_str() ) );
          }
        }
      }

      strTemp = "";
    }
  }

  // Content-Length sollte drin sein
  GR::String   strContentLength = httpTemp.Line( "content-length" );
  if ( strContentLength.length() )
  {
    dwContentLength = atoi( strContentLength.c_str() );
  }

  if ( dwContentLength == 0 )
  {
    return false;
  }
  if ( bbIncoming.Size() == iOffset + dwContentLength )
  {
    return true;
  }
  return false;

}