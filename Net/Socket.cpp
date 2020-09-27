#include "Socket.h"

#include <map>

#include <debug/debugclient.h>


#pragma comment( lib, "wsock32.lib" )

namespace GR
{


std::map<SOCKET,CSocket*>                CSocket::m_mapSocketToClass;



CSocket::CSocket() :
  m_Socket( INVALID_SOCKET ),
  m_dwLastError( 0 ),
  m_dwTimeout( 0 )
{

  memset( &m_SocketAddress, 0, sizeof( m_SocketAddress ) );
  m_dwLastActivityTicks = GetTickCount();

}



CSocket::~CSocket()
{

  ReleaseSocket();

}



void CSocket::ReleaseSocket()
{

  if ( m_Socket != INVALID_SOCKET )
  {
    if ( !m_mapSocketToClass.empty() )
    {
      std::map<SOCKET,CSocket*>::iterator   it( m_mapSocketToClass.find( m_Socket ) );
      if ( it != m_mapSocketToClass.end() )
      {
        m_mapSocketToClass.erase( it );
      }
    }

    // Hard close
    LINGER      ling;

    ling.l_onoff  = TRUE;
    ling.l_linger = 0;
    setsockopt( m_Socket, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof( ling ) );
    closesocket( m_Socket );
    m_Socket = INVALID_SOCKET;
  }

}



bool CSocket::SocketSend( const void* pData, GR::up dwLength )
{

  int   iSendOffset = 0;

  TouchSocket();

  /*
  dh::Log( "Sending %d Bytes", dwLength );
  dh::Hex( pData, dwLength );
  */

  while ( dwLength )
  {
    int   iResult = send( m_Socket, ( (const char*)pData ) + iSendOffset, (int)dwLength, 0 );
    if ( iResult == SOCKET_ERROR )
    {
      m_dwLastError = WSAGetLastError();
      dh::Log( "send failed %d", m_dwLastError );
      return false;
    }
    iSendOffset += iResult;
    dwLength -= iResult;
    /*
    if ( iResult < dwLength )
    {
      dh::Log( "Sent partially %d of %d", iResult, dwLength );
    }
    */
  }

  return true;

}



SOCKET CSocket::Socket()
{
  return m_Socket;
}



void CSocket::Socket( SOCKET sktNew )
{
  std::map<SOCKET,CSocket*>::iterator   it( m_mapSocketToClass.find( m_Socket ) );
  if ( it != m_mapSocketToClass.end() )
  {
    m_mapSocketToClass.erase( it );
  }
  m_Socket = sktNew;

  if ( m_Socket != INVALID_SOCKET )
  {
    m_mapSocketToClass[m_Socket] = this;
  }
}



void CSocket::TouchSocket()
{

  m_dwLastActivityTicks = GetTickCount();

}



GR::u32 CSocket::LastActivityTime() const
{

  return m_dwLastActivityTicks;

}



GR::u32 CSocket::LastError() const
{

  return m_dwLastError;

}



bool CSocket::IsValid() const
{

  return ( m_Socket != INVALID_SOCKET );

}



void CSocket::SetSocketTimeout( GR::u32 dwTimeout )
{

  m_dwTimeout = dwTimeout;

}



GR::u32 CSocket::TimeOut() const
{

  return m_dwTimeout;

}


CSocket::eSocketState CSocket::SocketState()
{

  if ( m_Socket == INVALID_SOCKET )
  {
    return ST_SOCKET_INVALID;
  }

  fd_set  fds;

  FD_ZERO( &fds );
  FD_SET( m_Socket, &fds );

  struct timeval tmo;

  tmo.tv_sec  = 0;
  tmo.tv_usec = 100;

  int   iError = NULL;
 
  int iRet = select( FD_SETSIZE, &fds, NULL, NULL, &tmo );

  if ( iRet == 0 )
  {
    return ST_SOCKET_VALID;
  }
  else if ( iRet == SOCKET_ERROR )
  {
    return ST_SOCKET_ERROR;
  }
  return ST_SOCKET_DATA_AVAILABLE;

}



void CSocket::SetAutoFlush( bool Flush )
{

  if ( m_Socket == INVALID_SOCKET )
  {
    return;
  }

  int flag = 1;

  if ( !Flush )
  {
    flag = 0;
  }

  setsockopt( m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof( int ) );

}


}; // namespace GR





