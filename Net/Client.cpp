#include "client.h"

#include <debug/debugclient.h>

#include "Message.h"


namespace GR
{


CClient::CClient() :
  m_bConnected( false ),
  m_dwID( tMessage::R_ALL ),
  m_pIncomingData( NULL ),
  m_dwIncomingDataSize( 0 )
{

}

CClient::~CClient()
{

}



bool CClient::Connect( const char* szIPAddress, DWORD dwPort, 
                       const char* szUser, const char* szPassword )
{

  if ( IsConnected() )
  {
    return false;
  }

  if ( !InitInstance() )
  {
    return false;
  }

  m_mapClients.clear();

  if ( szUser == NULL )
  {
    m_strUserName = "Anonymous";
  }
  else
  {
    m_strUserName = szUser;
  }
  if ( szPassword == NULL )
  {
    m_strPassword = "";
  }
  else
  {
    m_strPassword = szPassword;
  }

  unsigned long       ulIPAddress   = 0,
                      *pIPAddress   = NULL;

  LPHOSTENT           lpHostEntry   = NULL;

  sockaddr_in         stServerAddress;


  m_Socket = socket( AF_INET, SOCK_STREAM, 0 );
  if ( m_Socket == INVALID_SOCKET )
  { 
    return false;
  }
  memset( &stServerAddress, 0, sizeof( struct sockaddr_in ) );
  //if ( !bOnlyIP )
  {
    /*
    if ( !IsIPName( szIPAddress ) )
    {
      ExitInstance();
      return false;
    }
    */
    lpHostEntry = gethostbyname( szIPAddress );
    if ( lpHostEntry == NULL )
    {
      //ExitInstance();
      return false;
    }
    pIPAddress  = (unsigned long FAR *)lpHostEntry->h_addr;
    ulIPAddress = *pIPAddress;
  }
  if ( !ulIPAddress )
  {
    ulIPAddress  = inet_addr( szIPAddress );
  }
  if ( ulIPAddress == INADDR_NONE )
  {
    //ExitInstance();
    return false;
  }
  stServerAddress.sin_family       = AF_INET;
  stServerAddress.sin_addr.s_addr  = ulIPAddress;
  stServerAddress.sin_port         = htons( (WORD)dwPort );
  if ( connect( m_Socket, (LPSOCKADDR)&stServerAddress, sizeof( stServerAddress ) ) == SOCKET_ERROR )
  {
    //ExitInstance();
    return false;
  }
  if ( WSAAsyncSelect( m_Socket, m_hWndSocket, m_dwNotifyMessage, FD_READ | FD_CLOSE ) != 0 )
  {
    ReleaseSocket();
    //ExitInstance();
    return false;
  }

  m_bConnected = true;

  OnConnected();

  return true;

}



bool CClient::Disconnect()
{

  if ( !m_bConnected )
  {
    return true;
  }
  
  m_bConnected = false;
  m_mapClients.clear();
  ReleaseSocket();

  ExitInstance();

  OnDisconnected();

  m_dwID = tMessage::R_ALL;

  delete[] m_pIncomingData;
  m_pIncomingData = NULL;
  m_dwIncomingDataSize = 0;


  return true;

}



bool CClient::IsConnected() 
{

  return m_bConnected;

}



bool CClient::SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{

  if ( nMessage == m_dwNotifyMessage )
  {
    switch ( (WORD)WSAGETSELECTEVENT( lParam ) )
    {
      case FD_CLOSE:
        dh::Log( "[Client%d]Server shutdown", m_dwID );
        Disconnect();
        break;
    }
  }

  return CSocketHandler::SocketProc( hWnd, nMessage, wParam, lParam );

}



bool CClient::OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer )
{
  //dh::Log( "[Client%d]Received Message Type %d", m_dwID, Type );
  switch ( Type )
  {
    case tMessage::M_CONNECT_ID:
      // meine ID ist da
      {
        tMsgConnectId   MsgConnectId( MemBuffer );

        m_dwID = MsgConnectId.m_dwClientID;

        dh::Log( "Client: Connect with ID %d", m_dwID );
        m_mapClients[m_dwID].m_strName = m_strUserName;

        SendMessageToServer( GR::tMsgUserInfo( m_dwID, m_strUserName ) );
      }
      return true;
    case tMessage::M_DISCONNECT:
      {
        tMsgDisconnect    MsgDisconnect( MemBuffer );

        //dh::Log( "[Client%d]>Client %d verläßt uns", m_dwID, MsgDisconnect.m_dwClientID );
        OnUserDisconnected( MsgDisconnect.m_dwClientID );

        tMapClients::iterator   itClient( m_mapClients.find( MsgDisconnect.m_dwClientID ) );
        if ( itClient != m_mapClients.end() )
        {
          m_mapClients.erase( itClient );
        }
      }
      return true;
    case tMessage::M_TEXT:
      {
        tMsgText   MsgText( MemBuffer );
        //dh::Log( "[Client%d]Message %s", m_dwID, MsgText.m_strText.c_str() );

        OnTextMessage( MsgText.m_dwSender, MsgText.m_strText );
      }
      break;
    case tMessage::M_USER_INFO:
      {
        tMsgUserInfo    MsgUserInfo( MemBuffer );

        m_mapClients[MsgUserInfo.m_dwClientID].m_strName = MsgUserInfo.m_strName;
        //dh::Log( "[Client%d]Client %d nennt sich %s", m_dwID, MsgUserInfo.m_dwClientID, MsgUserInfo.m_strName.c_str() );

        OnUserConnected( MsgUserInfo.m_dwClientID );
      }
      break;
  }

  return false;
}



CClient::tClientInfo* CClient::GetClientInfo( DWORD dwID )
{
  tMapClients::iterator   itClient( m_mapClients.find( dwID ) );
  if ( itClient == m_mapClients.end() )
  {
    return NULL;
  }
  return &itClient->second;
}



DWORD CClient::ClientID() const
{

  return m_dwID;

}



void CClient::OnDataReceived( const SOCKET socket, const void* pData, DWORD dwBytesReceived )
{

  //dh::Log( "[Client%d] %d Bytes received", ClientID(), dwBytesReceived );

  DWORD   iDataStart = 0,
          iMessageLength = 0;

  // Daten merken
  GR::u8*     pNewData = new GR::u8[m_dwIncomingDataSize + dwBytesReceived];

  if ( m_pIncomingData )
  {
    memcpy( pNewData, m_pIncomingData, m_dwIncomingDataSize );
  }
  memcpy( pNewData + m_dwIncomingDataSize, pData, dwBytesReceived );

  delete[] m_pIncomingData;
  m_pIncomingData = pNewData;

  m_dwIncomingDataSize += dwBytesReceived;

  GR::u32        dwParsedData = 0;


  BYTE*   pTempData = (BYTE*)m_pIncomingData;

  if ( m_dwIncomingDataSize >= sizeof( tMessageHeader ) + 4 )
  {
    while ( iDataStart < m_dwIncomingDataSize )
    {
      iMessageLength = *( (DWORD*)( pTempData + iDataStart ) );
      //dh::Log( "Recv Msg Size %d", iMessageLength );
      if ( m_dwIncomingDataSize >= iDataStart + iMessageLength + 4 )
      {
        tMessageHeader*   pMessage = (tMessageHeader*)( pTempData + iDataStart + 4 );
        //dh::Log( "Recvd Msg Type %d", pMessage->m_Type );
        if ( !OnMsgReceived( pMessage->m_Type, MemoryStream( pTempData + iDataStart + 4, iMessageLength ) ) )
        {
          // Message not handled, discarding...
        }
        iDataStart += 4 + iMessageLength;
        dwParsedData += 4 + iMessageLength;
      }
      else
      {
        /*
        dh::Log( "[Client%d]Received message not complete! (%d >= %d)", ClientID(),
              m_dwIncomingDataSize, iMessageLength + 4 );
              */

        GR::u8*     pNewData = new GR::u8[m_dwIncomingDataSize - dwParsedData];

        memcpy( pNewData, m_pIncomingData + dwParsedData, m_dwIncomingDataSize - dwParsedData );

        delete[] m_pIncomingData;
        m_pIncomingData = pNewData;

        m_dwIncomingDataSize -= dwParsedData;
        return;
      }
    }
    if ( dwParsedData < m_dwIncomingDataSize )
    {
      // noch nicht alle Daten abgearbeitet
      //dh::Log( "[Client%d] %d Bytes left for later processing", ClientID(), m_dwIncomingDataSize - dwParsedData );

      GR::u8*     pNewData = new GR::u8[m_dwIncomingDataSize - dwParsedData];

      memcpy( pNewData, m_pIncomingData + dwParsedData, m_dwIncomingDataSize - dwParsedData );

      delete[] m_pIncomingData;
      m_pIncomingData = pNewData;

      m_dwIncomingDataSize -= dwParsedData;
    }
    else
    {
      //dh::Log( "[Client%d] no Bytes left for later processing", ClientID() );
      delete[] m_pIncomingData;
      m_pIncomingData = NULL;
      m_dwIncomingDataSize = 0;
    }
  }

}



bool CClient::SendMessageToOthers( tMessage& Message )
{

  Message.m_dwSender    = ClientID();
  Message.m_dwReceiver  = tMessage::R_ALL;

  return CMessageSocket::SendGeneralMessage( Message );

}



bool CClient::SendMessageToServer( tMessage& Message )
{

  Message.m_dwSender    = ClientID();
  Message.m_dwReceiver  = tMessage::R_SERVER;

  return CMessageSocket::SendGeneralMessage( Message );

}



void CClient::OnTextMessage( GR::u32 dwSenderID, const GR::String& strText )
{
}


void CClient::OnConnected()
{
}

void CClient::OnDisconnected()
{
}


void CClient::OnUserConnected( GR::u32 dwClientID )
{
}

void CClient::OnUserDisconnected( GR::u32 dwClientID )
{
}

};    // namespace GR


