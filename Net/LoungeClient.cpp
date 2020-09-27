#include ".\loungeclient.h"

#include <Net/Message.h>

#include <debug/debugclient.h>



CLoungeClient::CLoungeClient() :
  m_dwRoomID( -1 )
{
}



bool CLoungeClient::OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer )
{
  //dh::Log( "[Client%d]Msg (%d) received", m_dwID, Type );
  switch ( Type )
  {
    case GR::tMessage::M_CREATE_GROUP:
      {
        GR::tMsgCreateGroup    MsgCreateGroup( MemBuffer );

        dh::Log( "[Client%d]Group (%d) %s created", m_dwID, MsgCreateGroup.m_dwGroupID, MsgCreateGroup.m_strName.c_str() );
        CreateRoom( MsgCreateGroup.m_strName.c_str(), MsgCreateGroup.m_dwGroupID, MsgCreateGroup.m_dwFlags );
      }
      break;
    case GR::tMessage::M_DESTROY_GROUP:
      {
        GR::tMsgDestroyGroup    MsgDestroyGroup( MemBuffer );

        CLoungeRoom*    pRoom = GetRoom( MsgDestroyGroup.m_dwGroupID );

        //dh::Log( "[Client%d]Group (%d) %s destroyed", m_dwID, MsgDestroyGroup.m_dwGroupID, pRoom->Name().c_str() );
        DestroyRoom( MsgDestroyGroup.m_dwGroupID );
      }
      break;
    case GR::tMessage::M_ENTER_GROUP:
      {
        GR::tMsgEnterGroup    MsgEnterGroup( MemBuffer );

        CLoungeRoom*    pRoom = GetRoom( MsgEnterGroup.m_dwGroupID );
        if ( pRoom == NULL )
        {
          dh::Log( "[Client%d]M_ENTER_GROUP Group %d for Client %d not found!", m_dwID, MsgEnterGroup.m_dwGroupID, MsgEnterGroup.m_dwClientID );
          return true;
        }
        dh::Log( "[Client%d]Client %d wechselt in Group %s",
                 m_dwID,
                 MsgEnterGroup.m_dwClientID,
                 pRoom->Name().c_str() );
        pRoom->AddClient( MsgEnterGroup.m_dwClientID );
      }
      break;
    case GR::tMessage::M_LEAVE_GROUP:
      {
        GR::tMsgLeaveGroup    MsgLeaveGroup( MemBuffer );

        CLoungeRoom*    pRoom = GetRoom( MsgLeaveGroup.m_dwGroupID );
        if ( pRoom == NULL )
        {
          dh::Log( "[Client%d]M_LEAVE_GROUP Group %d not found!", m_dwID, MsgLeaveGroup.m_dwGroupID );
          return true;
        }
        /*
        dh::Log( "[Client%d]Client %d verlässt Group %s",
                 m_dwID,
                 MsgLeaveGroup.m_dwClientID,
                 pRoom->Name().c_str() );
                 */
        pRoom->RemoveClient( MsgLeaveGroup.m_dwClientID );
      }
      break;
  }

  return CClient::OnMsgReceived( Type, MemBuffer );

}



CLoungeRoom* CLoungeClient::CreateRoom( const GR::String& strRoomName,
                                        const GR::u32 dwID,
                                        const GR::u32 dwFlags )
{

  tMapRooms::iterator     it( m_mapRooms.find( dwID ) );
  if ( it != m_mapRooms.end() )
  {
    // gibt es schon!
    dh::Log( "[Server]Room (%d) %s exists already!", dwID, strRoomName.c_str() );
    return NULL;
  }

  CLoungeRoom&  Room = m_mapRooms[dwID] = CLoungeRoom( strRoomName, dwID, dwFlags );

  return &Room;

}



void CLoungeClient::DestroyRoom( const GR::u32 dwID )
{

  tMapRooms::iterator     it( m_mapRooms.find( dwID ) );
  if ( it == m_mapRooms.end() )
  {
    // gibt es gar nicht!
    dh::Log( "[Client%d]DestroyRoom: Room (%d) doesn't exist!", m_dwID, dwID );
    return;
  }

  // TODO - evtl. prüfen, ob beim Client da noch jemand drin ist!

  m_mapRooms.erase( it );

}



CLoungeRoom* CLoungeClient::GetRoom( const GR::u32 dwID )
{

  tMapRooms::iterator     it( m_mapRooms.find( dwID ) );
  if ( it == m_mapRooms.end() )
  {
    return NULL;
  }
  return &it->second;

}



bool CLoungeClient::Disconnect()
{

  m_mapRooms.clear();

  return CClient::Disconnect();

}



void CLoungeClient::EnterRoom( const GR::u32 dwRoomID, const GR::String& strPassword )
{

  SendMessageToServer( GR::tMsgEnterGroup( m_dwID, dwRoomID, strPassword ) );

}



void CLoungeClient::LeaveRoom()
{

  SendMessageToServer( GR::tMsgLeaveGroup( m_dwID, RoomID() ) );

}



GR::u32 CLoungeClient::RoomID() const
{

  return m_dwRoomID;

}