#include <winsock2.h>
#include <windows.h>

#include <debug/debugclient.h>

#include "ListenSocket.h"



namespace GR
{


CListenSocket::CListenSocket( bool bCacheing ) :
  CSocketHandler( bCacheing ),
  m_bStarted( false ),
  m_dwTCPPort( 0 )
{


}

CListenSocket::~CListenSocket()
{

  StopListening();

}



bool CListenSocket::IsStarted() const
{
  
  return m_bStarted;

}



bool CListenSocket::Listen( WORD wPort )
{

  if ( IsStarted() )
  {
    return true;
  }

  if ( m_bInitialized )
  {
    return false;
  }

  if ( !InitInstance() )
  {
    return false;
  }

  m_dwTCPPort = wPort;

  sockaddr_in     stServerAddress;


  m_Socket = socket( AF_INET, SOCK_STREAM, 0 );
  if ( m_Socket == INVALID_SOCKET )
  {
    m_dwTCPPort = 0;
    return false;
  }

  stServerAddress.sin_family      = AF_INET;
  stServerAddress.sin_addr.s_addr = htonl( INADDR_ANY );
  stServerAddress.sin_port        = htons( wPort );
  if ( bind( m_Socket, (LPSOCKADDR)&stServerAddress, sizeof( stServerAddress ) ) == SOCKET_ERROR )
  {
    dh::Log( "Bind failed %d", WSAGetLastError() );
    m_dwTCPPort = 0;
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  if ( listen( m_Socket, SOMAXCONN ) == SOCKET_ERROR )
  {
    dh::Log( "Listen failed %d", WSAGetLastError() );
    m_dwTCPPort = 0;
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  if ( WSAAsyncSelect( m_Socket, m_hWndSocket, m_dwNotifyMessage, FD_ACCEPT | FD_WRITE | FD_READ | FD_CLOSE ) != 0 )
  {
    m_dwTCPPort = 0;
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  SendEvent( GR::tSocketData( GR::tSocketData::SE_CONNECTED, m_Socket, 0, 0 ) );

  SetTimer( m_hWndSocket, 1, 1000, NULL );
  m_bStarted = true;

  return true;

}



bool CListenSocket::StopListening()
{

  if ( !IsStarted() )
  {
    return true;
  }

  KillTimer( m_hWndSocket, 1 );

  // alle client-sockets freigeben!
  tSetSockets::iterator   it( m_setSockets.begin() );
  while ( it != m_setSockets.end() )
  {
    CSocket*    pSocket = *it;

    pSocket->ReleaseSocket();
    delete pSocket;

    ++it;
  }

  m_setSockets.clear();

  closesocket( m_Socket );
  m_Socket = INVALID_SOCKET;

  ExitInstance();

  m_bStarted = false;
  m_dwTCPPort = 0;

  return true;

}



bool CListenSocket::SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{

  if ( nMessage == m_dwNotifyMessage )
  {
    switch ( (WORD)WSAGETSELECTEVENT( lParam ) )
    {
      case FD_ACCEPT:
        {
          // ein neuer Client
          int     iClientInfoSize = sizeof( sockaddr );

          sockaddr_in         addrClient;
          SOCKET              sockClient;

          sockClient = accept( m_Socket, (sockaddr*)&addrClient, &iClientInfoSize );

          if ( sockClient != INVALID_SOCKET )
          {
            OnAcceptConnection( sockClient, addrClient );
          }
          else
          {
            // kein Socket mehr frei?
            return true;
          }
        }
        break;
      case FD_CLOSE:
        // ein Client geht
        OnCloseConnection( (SOCKET)wParam );
        break;
    }
  }

  return CSocketHandler::SocketProc( hWnd, nMessage, wParam, lParam );

}



bool CListenSocket::OnAcceptConnection( const SOCKET sockClient, const sockaddr_in& sockAddr )
{

  CSocket*    pNewClient = new CSocket();

  pNewClient->Socket( sockClient );
  pNewClient->m_SocketAddress = sockAddr;

  m_setSockets.insert( pNewClient );

  /*
  m_ListenerSocket.Socket( sockClient );
  m_ListenerSocket.m_SocketAddress  = sockAddr;
  */

  SendEvent( GR::tSocketData( GR::tSocketData::SE_CLIENT_CONNECTED, sockClient, 0, 0, &sockAddr ) );

  return true;

}


bool CListenSocket::OnCloseConnection( const SOCKET sockClient )
{

  SendEvent( GR::tSocketData( GR::tSocketData::SE_CLIENT_DISCONNECTED, sockClient, 0, 0 ) );

  tSetSockets::iterator   it( m_setSockets.begin() );
  while ( it != m_setSockets.end() )
  {
    CSocket*    pSocket = *it;

    if ( pSocket->Socket() == sockClient )
    {
      delete pSocket;
      m_setSockets.erase( it );
      break;
    }

    ++it;
  }

  //m_ListenerSocket.ReleaseSocket();
  return true;

}



WORD CListenSocket::Port() const
{

  return (WORD)m_dwTCPPort;

}



};  // namespace GR
