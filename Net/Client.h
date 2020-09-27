#pragma once

#include <map>
#include <string>

#include "Message.h"
#include "MessageSocket.h"
#include "SocketHandler.h"


namespace GR
{

class CClient : public CSocketHandler, public CMessageSocket
{

  public:

    struct tClientInfo
    {
      DWORD             m_dwID;
      GR::String       m_strName;

      tClientInfo() :
        m_dwID( 0 ),
        m_strName( "" )
      {
      }
    };


  protected:

    typedef std::map<DWORD,tClientInfo>   tMapClients;


    DWORD                             m_dwID;

    bool                              m_bConnected;

    GR::String                       m_strUserName,
                                      m_strPassword;

    tMapClients                       m_mapClients;

    GR::u8*                           m_pIncomingData;
    GR::u32                           m_dwIncomingDataSize;


  public:


    CClient();
    virtual ~CClient();


    virtual bool                      Connect( const char* szIPAddress, DWORD dwPort,
                                               const char* szUser = NULL, const char* szPassword = NULL );
    virtual bool                      Disconnect();
    bool                              IsConnected();

    DWORD                             ClientID() const;

    virtual bool                      SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );

    virtual bool                      OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer );

    virtual void                      OnDataReceived( const SOCKET socket, const void* pData, DWORD dwBytesReceived );

    virtual void                      OnTextMessage( GR::u32 dwSenderID, const GR::String& strText );
    virtual void                      OnConnected();
    virtual void                      OnDisconnected();
    virtual void                      OnUserConnected( GR::u32 dwClientID );
    virtual void                      OnUserDisconnected( GR::u32 dwClientID );

    tClientInfo*                      GetClientInfo( DWORD dwID );

    bool                              SendMessageToServer( tMessage& Message );
    bool                              SendMessageToOthers( tMessage& Message );

};


};