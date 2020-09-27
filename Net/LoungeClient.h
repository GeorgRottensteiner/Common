#pragma once

//#define NON_BLOCKING_SOCKETS

#ifdef NON_BLOCKING_SOCKETS
#include <Net/Connecter.h>
#else
#include <Net/Client.h>
#endif // NON_BLOCKING_SOCKETS

#include "LoungeRoom.h"


#ifdef NON_BLOCKING_SOCKETS
class CLoungeClient : public GR::Net::Socket::Connecter
#else
class CLoungeClient : public GR::CClient
#endif
{

  protected:

    typedef std::map<GR::u32,CLoungeRoom>     tMapRooms;

    GR::u32                           m_dwRoomID;

    tMapRooms                         m_mapRooms;


    CLoungeRoom*                      CreateRoom( const GR::String& strRoomName, const GR::u32 dwID, const GR::u32 dwFlags );
    void                              DestroyRoom( const GR::u32 dwID );
    CLoungeRoom*                      GetRoom( const GR::u32 dwID );


  public:


    CLoungeClient();


    virtual bool                      OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer );

    virtual bool                      Disconnect();

    void                              EnterRoom( const GR::u32 dwRoomID, const GR::String& strPassword = "" );
    void                              LeaveRoom();

    GR::u32                           RoomID() const;

};
