#pragma once

//#define NON_BLOCKING_SOCKETS

#ifdef NON_BLOCKING_SOCKETS
#include <Net/Listener.h>
#else
#include <Net/Server.h>
#endif // NON_BLOCKING_SOCKETS

#include <Net/Message.h>

#include "LoungeRoom.h"


#ifdef NON_BLOCKING_SOCKETS
class CLoungeServer : public GR::Net::Socket::Listener
#else
class CLoungeServer : public GR::CServer
#endif
{

  protected:

    typedef std::map<GR::u32,CLoungeRoom>     tMapRooms;

    typedef std::map<GR::u32,GR::u32>         tMapClientToRooms;


    tMapRooms                 m_mapRooms;

    tMapClientToRooms         m_mapClientToRooms;


    virtual bool              SendUserInfoFromOthers( GR::u32 dwClientID );

    virtual bool              OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer );

    virtual bool              IsClientAllowedToEnter( const GR::u32 dwClientID,
                                                      const GR::u32 dwGroupID,
                                                      const GR::String& strPassword );

    virtual void              OnClientEntersRoom( GR::u32 dwClientID, GR::u32 dwRoomID );
    virtual void              OnClientLeavesRoom( GR::u32 dwClientID, GR::u32 dwRoomID );
    virtual void              OnRoomCreated( GR::u32 dwRoomID, CLoungeRoom& Room );
    virtual void              OnRoomDestroyed( GR::u32 dwRoomID, CLoungeRoom& Room );


  public:


    CLoungeRoom*              CreateRoom( const GR::String& strRoomName,
                                          GR::u32 dwID = -1,
                                          GR::u32 dwLoungeFlags = 0,
                                          GR::u32 dwMaxClients = 0,
                                          const GR::String& strPassword = "" );
    bool                      DestroyRoom( GR::u32 dwID );
    CLoungeRoom*              GetRoom( const GR::u32 dwID );

    bool                      ClientEnterRoom( GR::u32 dwClientID, GR::u32 dwRoomID );
    bool                      ClientLeaveRoom( GR::u32 dwClientID );
    bool                      ClientChangeRoom( GR::u32 dwClientID, GR::u32 dwRoomID );

    virtual bool              Start( DWORD dwPort );
    virtual bool              Stop();

    virtual void              DestroyAllRooms();

    virtual bool              RemoveClient( GR::u32 dwId );




};
