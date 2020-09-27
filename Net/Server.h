#pragma once

#include <winsock2.h>


#include <string>
#include <map>

#include "Message.h"
#include "MessageSocket.h"
#include "SocketHandler.h"


namespace GR
{

class CServer : public CSocketHandler, public CMessageSocket
{

  protected:

    struct tClientInfo
    {
      CMessageSocket    m_Socket;
      GR::u32           m_dwID;
      GR::String       m_strName;
      bool              m_bLoginCompleted;

      tClientInfo() :
        m_Socket(),
        m_dwID( 0 ),
        m_strName( "" ),
        m_bLoginCompleted( false )
      {
      }
    };

    typedef std::map<DWORD,tClientInfo>   tMapClients;

    typedef std::map<SOCKET,ByteBuffer>  tMapIncomingData;

    bool                        m_bStarted;

    DWORD                       m_dwTCPPort;

    tMapClients                 m_mapClients;

    tMapIncomingData            m_mapIncomingData;
    

    bool                        Send( GR::u32 dwSender, GR::u32 dwReceiver, MemoryStream& MemBuffer );
    
    bool                        SendToAllClients( MemoryStream& MemBuffer, GR::u32 dwSender );

    tClientInfo&                AddClient();
    virtual bool                RemoveClient( GR::u32 dwId );
    tClientInfo*                GetClientInfoBySocket( SOCKET sktClient );
    tClientInfo*                GetClientInfo( GR::u32 dwId );
    GR::up                      GetClientCount() const;

    // Client hat neu connected, bekommt Daten von anderen Usern (überschreiben, wenn mehr Daten gesendet werden soll)
    virtual bool                SendUserInfoFromOthers( GR::u32 dwClientID );


  public:


    CServer();
    virtual ~CServer();

    bool                        IsStarted() const;

    bool                        Start( DWORD dwPort = 0 );

    bool                        Stop();

    GR::String                 GetClientName( GR::u32 dwId );

    virtual bool                SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );

    virtual bool                OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer );

    virtual void                OnDataReceived( const SOCKET socket, const void* pData, DWORD dwBytesReceived );

    virtual void                OnClientConnects( GR::u32 dwClientID );
    virtual void                OnClientDisconnects( GR::u32 dwClientID );
    virtual void                OnServerStarted();
    virtual void                OnServerStopped();

    virtual bool                OnAllowNewClient();

    bool                        Send( GR::u32 dwSender, GR::u32 dwReceiver, tMessage& Msg );
    bool                        SendToClient( GR::u32 dwReceiver, tMessage& Msg );
    bool                        SendToAllClients( tMessage& Msg, GR::u32 dwSender = GR::tMessage::R_SERVER );

};


}; // namespace GR