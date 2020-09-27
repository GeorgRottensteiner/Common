#include <Net/Socket.h>

#include "SocketHandler.h"

#include <map>

#include <debug/debugclient.h>


#pragma comment( lib, "wsock32.lib" )

HINSTANCE   hInstDummy = NULL;

namespace GR
{


std::map<HWND,CSocketHandler*>        CSocketHandler::m_mapHWNDToSocket;

DWORD                                 CSocketHandler::m_dwNotifyMessage = RegisterWindowMessageA( "GRSocketHandlerMsg" );



CSocketHandler::CSocketHandler( bool bCacheing ) :
  m_bInitialized( false ),
  m_hWndSocket( NULL ),
  m_bCacheing( bCacheing )
{


}



CSocketHandler::~CSocketHandler()
{

}



LRESULT CALLBACK CSocketHandler::SocketWndProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{

  std::map<HWND,CSocketHandler*>::iterator   it( m_mapHWNDToSocket.find( hWnd ) );
  if ( it != m_mapHWNDToSocket.end() )
  {
    CSocketHandler*    pSocketHandler = it->second;

    size_t    iMapSize = m_mapHWNDToSocket.size();

    if ( IsWindow( hWnd ) )
    {
      if ( pSocketHandler->SocketProc( hWnd, nMessage, wParam, lParam ) )
      {
        return 0;
      }
    }
    if ( iMapSize != m_mapHWNDToSocket.size() )
    {
      return 0;
    }

    ++it;
  }

  if ( !::IsWindow( hWnd ) )
  {
    return TRUE;
  }

  return DefWindowProc( hWnd, nMessage, wParam, lParam );

}



bool CSocketHandler::SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{

  if ( nMessage == m_dwNotifyMessage )
  {
    switch ( (WORD)WSAGETSELECTEVENT( lParam ) )
    {
      case FD_READ:
        {
          std::map<SOCKET,GR::CSocket*>::iterator   it( CSocket::m_mapSocketToClass.find( (SOCKET)wParam ) );
          if ( it != CSocket::m_mapSocketToClass.end() )
          {
            GR::CSocket*    pSocket = it->second;

            pSocket->TouchSocket();
          }

          char      data[20000];

          int bytes_recv = recv( wParam, (char*)&data, 20000, 0 );

          if ( bytes_recv == SOCKET_ERROR )
          {
            dh::Log( "CSocketHandler recv error %x", WSAGetLastError() );
          }
          else
          {
            if ( m_bCacheing )
            {
              m_bbIncomingData.AppendData( &data, bytes_recv );
            }
            else
            {
              OnDataReceived( (SOCKET)wParam, &data, bytes_recv );
            }
          }
        }
        break;
    }
    return true;
  }

  switch ( nMessage )
  {
    case WM_DESTROY:
      KillTimer( hWnd, 1 );
      break;
    case WM_TIMER:
      {
        //dh::Log( "%d Sockets in map", CSocket::m_mapSocketToClass.size() );
        std::map<SOCKET,CSocket*>::iterator   it( CSocket::m_mapSocketToClass.begin() );
        while ( it != CSocket::m_mapSocketToClass.end() )
        {
          CSocket* pSocket = it->second;

          std::map<SOCKET,CSocket*>::iterator    itNext = it;
          itNext++;

          /*
          dh::Log( "Socket Timer %d (%d - %d) >= %d ?", GetTickCount() - pSocket->LastActivityTime(), 
              GetTickCount(), pSocket->LastActivityTime(),
              pSocket->TimeOut() );
              */

          if ( ( pSocket->TimeOut() )
          &&   ( GetTickCount() - pSocket->LastActivityTime() >= pSocket->TimeOut() ) )
          {
            pSocket->TouchSocket();

            SendEvent( tSocketData( tSocketData::SE_TIMEOUT, it->first, 0, 0 ) );
          }
          it = itNext;
        }
      }
      break;
  }
  return false;

}



void CSocketHandler::OnDataReceived( const SOCKET socket, const void* pData, DWORD dwBytesReceived )
{

  SendEvent( tSocketData( tSocketData::SE_DATA_RECEIVED, socket, pData, dwBytesReceived ) );

}



bool CSocketHandler::InitInstance()
{

  if ( m_bInitialized )
  {
    return true;
  }

  WSADATA     wsaData;

  int         iError = 0;


  iError = WSAStartup( 0x0101, &wsaData );
  if ( iError )
  {
    return false;
  }
  if ( wsaData.wVersion < 0x0001 )
  {
    WSACleanup();
    return false;
  }

  WNDCLASS   wc;


  wc.style          = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc    = (WNDPROC)SocketWndProc;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;
  wc.hInstance      = GetModuleHandle( NULL );
  wc.hIcon          = NULL;
  wc.hCursor        = NULL;
  wc.hbrBackground  = NULL;
  wc.lpszMenuName   = NULL;
  wc.lpszClassName  = _T( "GRSocketClass" );
  RegisterClass( &wc );
  m_hWndSocket = CreateWindow( _T( "GRSocketClass" ),
                               _T( "" ),
                               WS_OVERLAPPED,
                               0,
                               0,
                               100,
                               100,
                               NULL,
                               NULL,
                               GetModuleHandle( NULL ),
                               NULL );
  if ( m_hWndSocket == NULL )
  {
    WSACleanup();
    return false;
  }

  //dh::Log( "HWND %x created (Thread %x)", m_hWndSocket, GetCurrentThreadId() );

  SetTimer( m_hWndSocket, 1, 1000, 0 );

  m_mapHWNDToSocket[m_hWndSocket] = this;

  m_bInitialized = true;

  return true;

}



bool CSocketHandler::ExitInstance()
{

  if ( !m_bInitialized )
  {
    return true;
  }

  std::map<HWND,CSocketHandler*>::iterator    it( m_mapHWNDToSocket.find( m_hWndSocket ) );
  if ( it != m_mapHWNDToSocket.end() )
  {
    //dh::Log( "HWND %x destroyed", it->first );
    DestroyWindow( it->first );
    m_mapHWNDToSocket.erase( it );
  }

  WSACleanup();

  SendEvent( tSocketData( tSocketData::SE_DISCONNECTED, 0, 0, 0 ) );

  m_bInitialized = false;

  return true;

}



size_t CSocketHandler::CachedDataSize()
{

  return m_bbIncomingData.Size();

}



size_t CSocketHandler::ReadCachedData( void* pTarget, size_t iDataSize )
{

  if ( iDataSize > m_bbIncomingData.Size() )
  {
    iDataSize = m_bbIncomingData.Size();
  }

  if ( iDataSize )
  {
    memcpy( pTarget, m_bbIncomingData.Data(), iDataSize );
    m_bbIncomingData.TruncateFront( iDataSize );
  }

  return iDataSize;

}


}; // namespace GR
