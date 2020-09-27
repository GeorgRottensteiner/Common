#pragma once

#include <winsock2.h>


#include <set>

#include "Message.h"
#include "Socket.h"
#include "SocketHandler.h"


namespace GR
{

class CListenSocket : public CSocketHandler, 
                      public CSocket
{

  protected:

    typedef std::set<CSocket*>  tSetSockets;

    bool                        m_bStarted;

    DWORD                       m_dwTCPPort;

    tSetSockets                 m_setSockets;

    //CSocket                     m_ListenerSocket;


    virtual bool                OnAcceptConnection( const SOCKET sockClient, const sockaddr_in& sockAddr );
    virtual bool                OnCloseConnection( const SOCKET sockClient );


  public:


    CListenSocket( bool bCacheing = false );
    virtual ~CListenSocket();

    bool                        IsStarted() const;

    bool                        Listen( WORD wPort = 0 );

    WORD                        Port() const;

    bool                        StopListening();

    virtual bool                SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );

};


}; // namespace GR