#pragma once

#include <winsock2.h>
#include <map>

#include <GR/GRTypes.h>



namespace GR
{

class CSocket
{

  protected:

    SOCKET              m_Socket;

    GR::u32             m_dwLastActivityTicks;

    GR::u32             m_dwTimeout;

    GR::u32             m_dwLastError;


  public:

    enum eSocketState
    {
      ST_SOCKET_INVALID,
      ST_SOCKET_VALID,
      ST_SOCKET_DISCONNECTED,
      ST_SOCKET_ERROR,
      ST_SOCKET_DATA_AVAILABLE,
    };

    static std::map<SOCKET,CSocket*>                m_mapSocketToClass;


    sockaddr_in         m_SocketAddress;


    CSocket();
    virtual ~CSocket();


    void                Socket( SOCKET sktNew );
    SOCKET              Socket();
    void                ReleaseSocket();

    bool                IsValid() const;

    virtual bool        SocketSend( const void* pData, GR::up dwLength );

    void                SetSocketTimeout( GR::u32 dwTimeout );
    GR::u32             TimeOut() const;

    void                SetAutoFlush( bool Flush = true );

    void                TouchSocket();

    GR::u32             LastActivityTime() const;
    GR::u32             LastError() const;

    CSocket::eSocketState   SocketState();


};


};