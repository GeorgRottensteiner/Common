#include <winsock2.h>
#include <windows.h>

#include <debug/debugclient.h>

#include "Server.h"
#include "Message.h"



namespace GR
{


CServer::CServer() :
  m_bStarted( false ),
  m_dwTCPPort( 0 )
{

}



CServer::~CServer()
{

  Stop();

}



bool CServer::IsStarted() const
{
  
  return m_bStarted;

}



bool CServer::Start( DWORD dwPort )
{

  if ( IsStarted() )
  {
    return true;
  }

  if ( !InitInstance() )
  {
    return false;
  }

  sockaddr_in     stServerAddress;


  m_Socket = socket( AF_INET, SOCK_STREAM, 0 );
  if ( m_Socket == INVALID_SOCKET )
  {
    dh::Log( "[Server]Failed to create socket %x", WSAGetLastError() );

    ExitInstance();
    return false;
  }

  stServerAddress.sin_family      = AF_INET;
  stServerAddress.sin_addr.s_addr = htonl( INADDR_ANY );
  stServerAddress.sin_port        = htons( (WORD)dwPort );
  if ( bind( m_Socket, (LPSOCKADDR)&stServerAddress, sizeof( stServerAddress ) ) == SOCKET_ERROR )
  {
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  if ( listen( m_Socket, SOMAXCONN ) == SOCKET_ERROR )
  {
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  if ( WSAAsyncSelect( m_Socket, m_hWndSocket, m_dwNotifyMessage, FD_ACCEPT | FD_READ | FD_CLOSE ) != 0 )
  {
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  dh::Log( "[Server]Server started" );
  SetTimer( m_hWndSocket, 1, 1000, NULL );
  m_bStarted = true;

  OnServerStarted();

  return true;

}



bool CServer::Stop()
{

  if ( !IsStarted() )
  {
    return true;
  }

  // TODO alle client-sockets freigeben!
  m_mapClients.clear();
  KillTimer( m_hWndSocket, 1 );

  ExitInstance();

  dh::Log( "[Server]Server stopped" );

  m_bStarted = false;

  OnServerStopped();

  return true;

}



bool CServer::OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer )
{
  //dh::Log( "[Server]Received Msg %d", Type );
  switch ( Type )
  {
    case tMessage::M_TEXT:
      {
        GR::tMsgText    MsgText( MemBuffer );

        //dh::Log( "[Server]Client %d - %s", MsgText.m_dwSender, MsgText.m_strText.c_str() );

        if ( MsgText.m_dwReceiver == tMessage::R_ALL )
        {
          SendToAllClients( MsgText, MsgText.m_dwSender );
          return true;
        }
        else
        {
          tClientInfo*  pClientInfo = GetClientInfo( MsgText.m_dwReceiver );
          if ( pClientInfo )
          {
            Send( MsgText.m_dwSender, MsgText.m_dwReceiver, MsgText );
          }
        }
      }
      return true;
    case tMessage::M_USER_INFO:
      {
        tMsgUserInfo  MsgUserInfo( MemBuffer );

        tClientInfo*  pClientInfo = GetClientInfo( MsgUserInfo.m_dwClientID );
        if ( pClientInfo )
        {
          pClientInfo->m_strName = MsgUserInfo.m_strName;
        }

        SendToAllClients( MsgUserInfo );

        //dh::Log( "Server: Client %d nennt sich %s", pUser->m_dwClientID, pUser->m_szName );
        SendUserInfoFromOthers( MsgUserInfo.m_dwClientID );

        pClientInfo->m_bLoginCompleted = true;

        // bevor komplett eingeloggt
        OnClientConnects( MsgUserInfo.m_dwClientID );

        SendToClient( MsgUserInfo.m_dwClientID, MsgUserInfo );
      }
      return true;
    default:
      {
        tMessage    Msg( MemBuffer );


        if ( Msg.m_dwReceiver == tMessage::R_SERVER )
        {
          break;
        }
        if ( Msg.m_dwReceiver == tMessage::R_ALL )
        {
          SendToAllClients( MemBuffer, Msg.m_dwSender );
        }
        else
        {
          Send( Msg.m_dwSender, Msg.m_dwReceiver, MemBuffer );
        }
      }
      break;
  }
  /*
  dh::Log( "Message Type %d", pMessage->m_Type );
  dh::Log( "Message Length %d", pMessage->m_dwLength );
  */

  return false;

}



bool CServer::Send( GR::u32 dwSender, GR::u32 dwReceiver, tMessage& Msg )
{

  Msg.m_dwSender    = dwSender;
  Msg.m_dwReceiver  = dwReceiver;

  if ( Msg.m_dwReceiver == tMessage::R_ALL )
  {
    // geht an alle
    return SendToAllClients( Msg );
  }
  tClientInfo*  pClientInfo = GetClientInfo( Msg.m_dwReceiver );
  if ( pClientInfo == NULL )
  {
    return false;
  }
  return pClientInfo->m_Socket.SendGeneralMessage( Msg );

}



bool CServer::Send( GR::u32 dwSender, GR::u32 dwReceiver, MemoryStream& MemBuffer )
{

  if ( dwReceiver == tMessage::R_ALL )
  {
    // geht an alle
    return SendToAllClients( MemBuffer, dwSender );
  }
  tClientInfo*  pClientInfo = GetClientInfo( dwReceiver );
  if ( pClientInfo == NULL )
  {
    return false;
  }
  GR::u32     dwSize = (GR::u32)MemBuffer.GetSize();

  //dh::Log( "Socket send Msg size (2) %d", dwSize );
  if ( !pClientInfo->m_Socket.SocketSend( (GR::u8*)&dwSize, 4 ) )
  {
    dh::Log( "[Server]Send failed" );
    return false;
  }
  //dh::Log( "calling Socket send size %d", dwSize );
  return pClientInfo->m_Socket.SocketSend( (BYTE*)MemBuffer.Data(), (GR::up)MemBuffer.GetSize() );

}



bool CServer::SendToAllClients( tMessage& Msg, GR::u32 dwSender )
{
  
  bool    bResult = true;

  tMapClients::iterator   it( m_mapClients.begin() );
  while ( it != m_mapClients.end() )
  {
    if ( it->second.m_bLoginCompleted )
    {
      Msg.m_dwReceiver  = it->first;
      Msg.m_dwSender    = dwSender;
      if ( !it->second.m_Socket.SendGeneralMessage( Msg ) )
      {
        dh::Log( "[Server]SendMessage failed" );
        bResult = false;
      }
    }

    ++it;
  }
  return false;

}



bool CServer::SendToAllClients( MemoryStream& MemBuffer, GR::u32 dwSender )
{
  
  bool    bResult = true;

  tMapClients::iterator   it( m_mapClients.begin() );
  while ( it != m_mapClients.end() )
  {
    GR::CServer::tClientInfo&   ClientInfo = it->second;

    GR::u32     dwSize = (GR::u32)MemBuffer.GetSize();

    if ( !ClientInfo.m_Socket.SocketSend( (BYTE*)&dwSize, 4 ) )
    {
      bResult = false;
    }

    if ( !ClientInfo.m_Socket.SocketSend( (BYTE*)MemBuffer.Data(), (GR::up)MemBuffer.GetSize() ) )
    {
      bResult = false;
    }

    ++it;
  }
  return false;

}



CServer::tClientInfo& CServer::AddClient()
{

  DWORD   dwFreeId = 1;

  tMapClients::iterator   it;
  while ( ( it = m_mapClients.find( dwFreeId ) ) != m_mapClients.end() )
  {
    dwFreeId++;
  }

  m_mapClients[dwFreeId].m_dwID   = dwFreeId;

  return m_mapClients[dwFreeId];

}



CServer::tClientInfo* CServer::GetClientInfoBySocket( SOCKET sktClient )
{

  tMapClients::iterator   it( m_mapClients.begin() );

  while ( it != m_mapClients.end() )
  {
    tClientInfo&    ClientInfo = it->second;

    if ( ClientInfo.m_Socket.Socket() == sktClient )
    {
      return &ClientInfo;
    }

    ++it;
  }

  return NULL;

}



GR::up CServer::GetClientCount() const
{

  return m_mapClients.size();

}



CServer::tClientInfo* CServer::GetClientInfo( GR::u32 dwId )
{

  tMapClients::iterator   it( m_mapClients.find( dwId ) );
  if ( it == m_mapClients.end() )
  {
    return NULL;
  }
  tClientInfo&    ClientInfo = it->second;

  return &ClientInfo;

}



GR::String CServer::GetClientName( GR::u32 dwId )
{

  tMapClients::iterator   it( m_mapClients.find( dwId ) );
  if ( it == m_mapClients.end() )
  {
    return "";
  }
  tClientInfo&    ClientInfo = it->second;

  return ClientInfo.m_strName;

}



bool CServer::RemoveClient( GR::u32 dwId )
{

  tMapClients::iterator   it( m_mapClients.find( dwId ) );
  if ( it == m_mapClients.end() )
  {
    return false;
  }

  dh::Log( "[Server]Client %d disconnected", dwId );

  tClientInfo&    ClientInfo = it->second;

  ClientInfo.m_Socket.ReleaseSocket();

  m_mapClients.erase( it );

  return true;

}



bool CServer::SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{

  if ( nMessage == m_dwNotifyMessage )
  {
    switch ( (WORD)WSAGETSELECTEVENT( lParam ) )
    {
      case FD_ACCEPT:
        if ( OnAllowNewClient() )
        {
          // ein neuer Client
          /*
          if ( (SOCKET)wParam != m_Socket )
          {
            // nur der Serversocket sollte FD_ACCEPT erhalten
            return true;
          }
          */

          int     iClientInfoSize = sizeof( tClientInfo );

          tClientInfo&    ClientInfo = AddClient();

          ClientInfo.m_Socket.Socket( accept( m_Socket, (sockaddr*)&ClientInfo.m_Socket.m_SocketAddress, &iClientInfoSize ) );
          if ( ClientInfo.m_Socket.Socket() == INVALID_SOCKET )
          {
            // kein Socket mehr frei?
            RemoveClient( ClientInfo.m_dwID );
            return true;
          }

          m_mapIncomingData[ClientInfo.m_Socket.Socket()].Clear();

          tMsgConnectId     MsgConnectId( ClientInfo.m_dwID );

          dh::Log( "[Server]Client %d connected", ClientInfo.m_dwID );
          Send( GR::tMessage::R_SERVER, ClientInfo.m_dwID, MsgConnectId );
        }
        else
        {
          sockaddr   tempAddr;

          SOCKET    tempSock = accept( m_Socket, (sockaddr*)&tempAddr, 0 );

          // Hard close
          LINGER      ling;

          ling.l_onoff  = TRUE;
          ling.l_linger = 0;
          setsockopt( tempSock, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof( ling ) );
          closesocket( tempSock );
        }
        break;
        /*
      case FD_READ:
        {
          char      data[20000];
          dh::Log( "fd_read\n" );
          int bytes_recv = recv( wParam, (char*)&data, 20000, 0 );

          dh::Log( "bytes received %d\n", bytes_recv );

          tMessage*   pMessage = (tMessage*)&data;

          dh::Log( "Message Type %d\n", pMessage->m_Type );
          dh::Log( "Message Length %d\n", pMessage->m_dwLength );
        }
        */
        break;
      case FD_CLOSE:
        // ein Client geht
        tClientInfo*    pClientInfo = GetClientInfoBySocket( (SOCKET)wParam );
        if ( pClientInfo )
        {
          DWORD   dwClientId = pClientInfo->m_dwID;

          OnClientDisconnects( dwClientId );

          RemoveClient( dwClientId );

          tMsgDisconnect    MsgDisconnect( dwClientId );

          Send( GR::tMessage::R_SERVER, GR::tMessage::R_ALL, MsgDisconnect );
        }
        break;
    }
  }
  else if ( nMessage == WM_TIMER )
  {
    // TODO - Timeouts
  }

  return CSocketHandler::SocketProc( hWnd, nMessage, wParam, lParam );

}



bool CServer::SendUserInfoFromOthers( GR::u32 dwClientID )
{

  tClientInfo*    pNewClientInfo = GetClientInfo( dwClientID );
  if ( pNewClientInfo == NULL )
  {
    return false;
  }

  bool      bResult = true;

  tMapClients::iterator   it( m_mapClients.begin() );
  while ( it != m_mapClients.end() )
  {
    if ( it->first != dwClientID )
    {
      tClientInfo&    ClientInfo = it->second;

      //dh::Log( "[Server]SendUserInfo über %d (%s) an %d (%s)", it->first, ClientInfo.m_strName.c_str(), dwClientID, pNewClientInfo->m_strName.c_str() );
      if ( !pNewClientInfo->m_Socket.SendGeneralMessage( GR::tMsgUserInfo( it->first, ClientInfo.m_strName ) ) )
      {
        bResult = false;
      }
    }

    ++it;
  }

  return bResult;

}



void CServer::OnDataReceived( const SOCKET socket, const void* pData, DWORD dwBytesReceived )
{
  tMapIncomingData::iterator    it( m_mapIncomingData.find( socket ) );
  if ( it == m_mapIncomingData.end() )
  {
    dh::Log( "[Server] Incoming Data from unknown socket!" );
    return;
  }

  ByteBuffer&    bbIncoming = it->second;

  //dh::Log( "[Server] %d Bytes received (%d already stored)", dwBytesReceived, m_dwIncomingDataSize );
  bbIncoming.AppendData( pData, dwBytesReceived );
  

  DWORD   iDataStart = 0,
          iMessageLength = 0;


  GR::u32        dwParsedData = 0;

  /*
  dh::Log( "Complete stored size %d", m_dwIncomingDataSize );
  dh::Hex( m_pIncomingData, m_dwIncomingDataSize );
  */


  BYTE*   pTempData = (BYTE*)bbIncoming.Data();

  if ( bbIncoming.Size() >= sizeof( tMessageHeader ) + 4 )
  {
    while ( iDataStart + 4 <= bbIncoming.Size() )
    {
      iMessageLength = *( (DWORD*)( pTempData + iDataStart ) );
      if ( bbIncoming.Size() >= iDataStart + iMessageLength + 4 )
      {
        tMessageHeader*   pMessage = (tMessageHeader*)( pTempData + iDataStart + 4 );
        if ( !OnMsgReceived( pMessage->m_Type, MemoryStream( pTempData + iDataStart + 4, iMessageLength ) ) )
        {
          // Message not handled, discarding...
        }
        iDataStart += 4 + iMessageLength;
        dwParsedData += 4 + iMessageLength;
        //dh::Log( "ProcessedMessage (%d) - iDataStart %d dwParsedData %d", iMessageLength, iDataStart, dwParsedData );
      }
      else
      {
        //dh::Log( "[Server]Received message (%d) not complete!", iMessageLength );
        break;
      }
    }
    if ( dwParsedData < bbIncoming.Size() )
    {
      // noch nicht alle Daten abgearbeitet
      if ( dwParsedData )
      {
        //dh::Log( "[Server] %d Bytes left for later processing", bbIncoming.Size() - dwParsedData );
        bbIncoming.TruncateFront( dwParsedData );
      }
    }
    else
    {
      //dh::Log( "[Server] no Bytes left for later processing" );
      bbIncoming.Clear();
    }
  }

}




bool CServer::SendToClient( GR::u32 dwReceiver, tMessage& Msg )
{

  return Send( GR::tMessage::R_SERVER, dwReceiver, Msg );

}



void CServer::OnClientConnects( GR::u32 dwClientID )
{

  tClientInfo*    pClientInfo = GetClientInfo( dwClientID );
  if ( pClientInfo )
  {
    m_mapIncomingData[pClientInfo->m_Socket.Socket()].Clear();
  }

}


void CServer::OnClientDisconnects( GR::u32 dwClientID )
{
  tClientInfo*    pClientInfo = GetClientInfo( dwClientID );
  if ( pClientInfo )
  {
    tMapIncomingData::iterator    it( m_mapIncomingData.find( pClientInfo->m_Socket.Socket() ) );
    if ( it != m_mapIncomingData.end() )
    {
      m_mapIncomingData.erase( it );
    }
  }
}



void CServer::OnServerStarted()
{
}



void CServer::OnServerStopped()
{
  m_mapIncomingData.clear();
}



bool CServer::OnAllowNewClient()
{
  return true;
}


};  // namespace GR