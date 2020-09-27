#include ".\loungeserver.h"

#include <Net/Message.h>

#include <debug/debugclient.h>



bool CLoungeServer::OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer )
{
  switch ( Type )
  {
    /*
    case GR::tMessage::M_USER_INFO:
      {
        GR::tMsgUserInfo  MsgUserInfo( MemBuffer );

        MemBuffer.SetPosition( 0 );

        bool    bResult = CServer::OnMsgReceived( Type, MemBuffer );

        if ( !bResult )
        {
          return false;
        }

        // ab in die Lobby
        //dh::Log( "Put new Client in lobby" );
        //ClientEnterRoom( MsgUserInfo.m_dwClientID, 0 );
        return true;
      }
      break;
      */
    case GR::tMessage::M_ENTER_GROUP:
      {
        GR::tMsgEnterGroup    MsgEnterGroup( MemBuffer );

        //dh::Log( "[Server]Client %d möchte in Raum %d", MsgEnterGroup.m_dwClientID, MsgEnterGroup.m_dwGroupID );
        // darf der Client ins spiel?
        if ( IsClientAllowedToEnter( MsgEnterGroup.m_dwClientID, MsgEnterGroup.m_dwGroupID, MsgEnterGroup.m_strPassword ) )
        {
          ClientEnterRoom( MsgEnterGroup.m_dwClientID, MsgEnterGroup.m_dwGroupID );
        }
        else
        {
          // TODO - denied!
        }
      }
      return true;
    case GR::tMessage::M_LEAVE_GROUP:
      {
        GR::tMsgLeaveGroup    MsgLeaveGroup( MemBuffer );

        ClientLeaveRoom( MsgLeaveGroup.m_dwClientID );
      }
      return true;
  }


  return CServer::OnMsgReceived( Type, MemBuffer );

}



bool CLoungeServer::Start( DWORD dwPort )
{

  if ( !CServer::Start( dwPort ) )
  {
    return false;
  }
  CreateRoom( "Lobby", 0 );

  return true;
}



bool CLoungeServer::Stop()
{

  bool    bResult = CServer::Stop();

  DestroyAllRooms();
  m_mapClientToRooms.clear();

  return bResult;

}



CLoungeRoom* CLoungeServer::CreateRoom( const GR::String& strRoomName,
                                        GR::u32 dwID,
                                        GR::u32 dwLoungeFlags,
                                        GR::u32 dwMaxClients,
                                        const GR::String& strPassword )
{

  if ( dwID == -1 )
  {
    // eine freie ID suchen
    dwID = 0;

    while ( m_mapRooms.find( dwID ) != m_mapRooms.end() )
    {
      ++dwID;
    }
  }
  tMapRooms::iterator     it( m_mapRooms.find( dwID ) );
  if ( it != m_mapRooms.end() )
  {
    // gibt es schon!
    dh::Log( "[Server]Room (%d) %s exists already!", dwID, strRoomName.c_str() );
    return NULL;
  }

  bool  bResult = SendToAllClients( GR::tMsgCreateGroup( dwID,
                                                         dwLoungeFlags,
                                                         dwMaxClients,
                                                         strRoomName ) );

  CLoungeRoom&    Room = m_mapRooms[dwID] = CLoungeRoom( strRoomName, dwID, dwLoungeFlags, dwMaxClients, strPassword );

  OnRoomCreated( dwID, Room );
  //dh::Log( "[Server]Room (%d) %s created!", dwID, strRoomName.c_str(), dwID );

  return &Room;

}



bool CLoungeServer::DestroyRoom( const GR::u32 dwID )
{

  tMapRooms::iterator     it( m_mapRooms.find( dwID ) );
  if ( it == m_mapRooms.end() )
  {
    // gibt es nicht!
    dh::Log( "[Server]Room (%d) doesn't exist!", dwID );
    return false;
  }

  CLoungeRoom*    pLobby = GetRoom( 0 );

  // erst alle Clients aus dem Raum werfen (in die Lobby, die muss es immer geben!)
  CLoungeRoom&    Room = it->second;

  GR::String   strRoomName = Room.Name();

  if ( Room.ID() != 0 )
  {
    while ( !Room.m_setClients.empty() )
    {
      GR::u32   dwClientInRoomID = GR::u32( *Room.m_setClients.begin() );

      ClientLeaveRoom( dwClientInRoomID );
      ClientEnterRoom( dwClientInRoomID, 0 );
    }
  }

  bool  bResult = SendToAllClients( GR::tMsgDestroyGroup( dwID ) );

  OnRoomDestroyed( dwID, Room );

  m_mapRooms.erase( it );

  //dh::Log( "[Server]Room (%d) %s destroyed!", dwID, strRoomName.c_str(), dwID );

  return true;

}



CLoungeRoom* CLoungeServer::GetRoom( const GR::u32 dwID )
{

  tMapRooms::iterator     it( m_mapRooms.find( dwID ) );
  if ( it == m_mapRooms.end() )
  {
    return NULL;
  }
  return &it->second;

}



void CLoungeServer::DestroyAllRooms()
{

  m_mapRooms.clear();

}



bool CLoungeServer::SendUserInfoFromOthers( GR::u32 dwClientID )
{

  tClientInfo*    pNewClientInfo = GetClientInfo( dwClientID );
  if ( pNewClientInfo == NULL )
  {
    return false;
  }

  if ( !CServer::SendUserInfoFromOthers( dwClientID ) )
  {
    return false;
  }

  // Room-Info senden
  tMapRooms::iterator   it( m_mapRooms.begin() );
  while ( it != m_mapRooms.end() )
  {
    CLoungeRoom&    Room = it->second;

    if ( !pNewClientInfo->m_Socket.SendGeneralMessage( GR::tMsgCreateGroup( Room.ID(), Room.m_dwFlags, Room.m_dwMaxClients, Room.Name() ) ) )
    {
      dh::Log( "[Server]CLoungeServer::SendGroupInfo failed" );
      return false;
    }

    // neuen Client in Lobby setzen
    if ( Room.ID() == 0 )
    {
      dh::Log( "[Server]Put new Client %d in lobby orig", dwClientID );
      //ClientEnterRoom( dwClientID, 0 );
      Send( GR::tMessage::R_SERVER, dwClientID, GR::tMsgEnterGroup( dwClientID, Room.ID(), "" ) );
    }

    // Clients in den Raum setzen
    CLoungeRoom::tSetClients::iterator    itClients( Room.m_setClients.begin() );
    while ( itClients != Room.m_setClients.end() )
    {
      GR::u32   dwClientInRoomID = GR::u32( *itClients );

      dh::Log( "An Client %d: Setze Client %d in Raum %d", dwClientID, dwClientInRoomID, Room.ID() );
      if ( dwClientInRoomID != dwClientID )
      {
        if ( !Send( GR::tMessage::R_SERVER, dwClientID, GR::tMsgEnterGroup( dwClientInRoomID, Room.ID(), "" ) ) )
        {
          dh::Log( "[Server]CLoungeServer Send Clients in Group failed" );
          return false;
        }
      }

      ++itClients;
    }


    ++it;
  }

  ClientEnterRoom( dwClientID, 0 );

  return true;

}



bool CLoungeServer::ClientLeaveRoom( GR::u32 dwClientID )
{

  tMapClientToRooms::iterator   it( m_mapClientToRooms.find( dwClientID ) );
  if ( it == m_mapClientToRooms.end() )
  {
    dh::Log( "[Server]ClientLeaveRoom: No room entry for client %d", dwClientID );
    return false;
  }
  GR::u32   dwRoomID = it->second;
  m_mapClientToRooms.erase( it );

  CLoungeRoom*    pRoom = GetRoom( dwRoomID );
  if ( pRoom == NULL )
  {
    dh::Log( "[Server]ClientLeaveRoom: No room for %d", dwRoomID );
    return false;
  }

  pRoom->RemoveClient( dwClientID );

  OnClientLeavesRoom( dwClientID, dwRoomID );

  return SendToAllClients( GR::tMsgLeaveGroup( dwClientID, dwRoomID ) );

}



bool CLoungeServer::ClientEnterRoom( GR::u32 dwClientID, GR::u32 dwRoomID )
{

  CLoungeRoom*    pRoom = GetRoom( dwRoomID );
  if ( pRoom == NULL )
  {
    dh::Log( "[Server]Client %d tries to enter non existing Room %d", dwClientID, dwRoomID );
    return false;
  }
  m_mapClientToRooms[dwClientID] = dwRoomID;
  pRoom->AddClient( dwClientID );

  OnClientEntersRoom( dwClientID, dwRoomID );

  return SendToAllClients( GR::tMsgEnterGroup( dwClientID, dwRoomID, "" ) );

}



bool CLoungeServer::ClientChangeRoom( GR::u32 dwClientID, GR::u32 dwRoomID )
{

  tClientInfo*    pCI = GetClientInfo( dwClientID );
  if ( pCI == NULL )
  {
    dh::Log( "[Server]non existing Client %d tries to change to Room %d", dwClientID, dwRoomID );
    return false;
  }
  CLoungeRoom*    pRoom = GetRoom( dwRoomID );
  if ( pRoom == NULL )
  {
    dh::Log( "[Server]Client %d tries to change to non existing Room %d", dwClientID, dwRoomID );
    return false;
  }

  if ( pRoom->IsClientInside( dwClientID ) )
  {
    // ist ja schon drin
    dh::Log( "[Server]Client %d changes to Room %d, already inside", dwClientID, dwRoomID );
    return true;
  }

  ClientLeaveRoom( dwClientID );
  ClientEnterRoom( dwClientID, dwRoomID );

  return true;

}



bool CLoungeServer::IsClientAllowedToEnter( const GR::u32 dwClientID, const GR::u32 dwGroupID, const GR::String& strPassword )
{

  CLoungeRoom*    pRoom = GetRoom( dwGroupID );
  if ( pRoom == NULL )
  {
    dh::Log( "[Server]Client %d tries to enter non-existing Room %d", dwClientID, dwGroupID );
    return false;
  }
  if ( ( pRoom->m_dwFlags & CLoungeRoom::LR_PASSWORD )
  &&   ( pRoom->m_strPassword != strPassword ) )
  {
    dh::Log( "[Server]Client %d tries to enter Room %d with wrong Password", dwClientID, dwGroupID );
    return false;
  }
  if ( ( pRoom->m_dwMaxClients > 0 )
  &&   ( pRoom->ClientCount() >= pRoom->m_dwMaxClients ) )
  {
    dh::Log( "[Server]Client %d tries to enter full Room %d", dwClientID, dwGroupID );
    return false;
  }
  return true;

}



void CLoungeServer::OnClientEntersRoom( GR::u32 dwClientID, GR::u32 dwRoomID )
{
}



void CLoungeServer::OnClientLeavesRoom( GR::u32 dwClientID, GR::u32 dwRoomID )
{
}



void CLoungeServer::OnRoomCreated( GR::u32 dwRoomID, CLoungeRoom& Room )
{
}



void CLoungeServer::OnRoomDestroyed( GR::u32 dwRoomID, CLoungeRoom& Room )
{
}



bool CLoungeServer::RemoveClient( GR::u32 dwId )
{
  bool    bResult = CServer::RemoveClient( dwId );

  ClientLeaveRoom( dwId );

  return bResult;
}



