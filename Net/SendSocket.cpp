#include "SendSocket.h"

#include <debug/debugclient.h>


namespace GR
{


CSendSocket::CSendSocket( bool bCacheing ) :
  CSocketHandler( bCacheing ),
  m_bConnected( false )
{

}

CSendSocket::~CSendSocket()
{

  RemoveAllListeners();
  Disconnect();

}



bool CSendSocket::Connect( const char* szIPAddress, DWORD dwPort )
{

  if ( IsConnected() )
  {
    return false;
  }

  if ( !InitInstance() )
  {
    return false;
  }
  
  unsigned long       ulIPAddress   = 0,
                      *pIPAddress   = NULL;

  LPHOSTENT           lpHostEntry   = NULL;

  sockaddr_in         stServerAddress;


  m_Socket = socket( AF_INET, SOCK_STREAM, 0 );
  if ( m_Socket == INVALID_SOCKET )
  { 
    dh::Log( "Socket Creation failed %d", WSAGetLastError() );
    return false;
  }
  memset( &stServerAddress, 0, sizeof( struct sockaddr_in ) );

  // try IP first
  ulIPAddress  = inet_addr( szIPAddress );
  if ( ulIPAddress == INADDR_NONE )
  {
    lpHostEntry = gethostbyname( szIPAddress );
    if ( lpHostEntry == NULL )
    {
      ReleaseSocket();
      return false;
    }
    pIPAddress  = (unsigned long FAR *)lpHostEntry->h_addr;
    ulIPAddress = *pIPAddress;
  }
  if ( !ulIPAddress )
  {
    ReleaseSocket();
    return false;
  }
  stServerAddress.sin_family       = AF_INET;
  stServerAddress.sin_addr.s_addr  = ulIPAddress;
  stServerAddress.sin_port         = htons( (WORD)dwPort );

  if ( !InitInstance() )
  {
    ReleaseSocket();
    return false;
  }

  if ( WSAAsyncSelect( m_Socket, m_hWndSocket, CSocketHandler::m_dwNotifyMessage, FD_READ | FD_CLOSE | FD_CONNECT ) == SOCKET_ERROR )
  {
    m_dwLastError = WSAGetLastError();
    ReleaseSocket();
    ExitInstance();
    return false;
  }

  Socket( m_Socket );
  m_SocketAddress = stServerAddress;

  if ( connect( m_Socket, (LPSOCKADDR)&stServerAddress, sizeof( stServerAddress ) ) == SOCKET_ERROR )
  {
    m_dwLastError = WSAGetLastError();
		if ( m_dwLastError == WSAEWOULDBLOCK) 
    {
      // das ist sogar in Ordnung (bei blocking)

      // bei blocking auf FD_CONNECT warten!
      Sleep( 500 );
		}
    else
    {
      ReleaseSocket();
      ExitInstance();
      return false;
    }
  }
  m_bConnected = true;

  return true;

}



bool CSendSocket::Disconnect()
{

  if ( !m_bConnected )
  {
    return true;
  }
  
  ReleaseSocket();
  ExitInstance();

  m_bConnected = false;

  return true;

}



bool CSendSocket::IsConnected() 
{

  return m_bConnected;

}



bool CSendSocket::SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{

  if ( nMessage == CSocketHandler::m_dwNotifyMessage )
  {
    switch ( (WORD)WSAGETSELECTEVENT( lParam ) )
    {
      case FD_CLOSE:
        Disconnect();
        break;
    }
  }

  return CSocketHandler::SocketProc( hWnd, nMessage, wParam, lParam );

}




};    // namespace GR
