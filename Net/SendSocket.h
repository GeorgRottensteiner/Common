#pragma once

#include <map>
#include <string>

#include "Socket.h"
#include "SocketHandler.h"


namespace GR
{

class CSendSocket : public CSocketHandler, public CSocket
{

  protected:

    bool                              m_bConnected;


  public:


    CSendSocket( bool bCacheing = false );
    virtual ~CSendSocket();


    virtual bool                      Connect( const char* szIPAddress, DWORD dwPort );
    virtual bool                      Disconnect();
    bool                              IsConnected();

    virtual bool                      SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );

};


};