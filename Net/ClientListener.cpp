#include "ClientListener.h"
#include "Message.h"

#include <debug/debugclient.h>



namespace GR
{
  namespace Net
  {
    namespace Socket
    {


      ClientListener::ClientListener()
      {

      }



      ClientListener::~ClientListener()
      {


      }


      GR::Net::Socket::Listener::tClientInfo* ClientListener::AddClient( SOCKET sockClient )
      {
        tClientInfo*    pInfo = Listener::AddClient( sockClient );

        tClientInfoEx*  pInfoEx = new tClientInfoEx();

        GR::u32         dwFreeID = 1;

        while ( GetClientByID( dwFreeID ) )
        {
          dwFreeID++;
        }
        pInfoEx->m_dwID   = dwFreeID;
        pInfoEx->m_pInfo  = pInfo;

        pInfo->m_dwUserData = (GR::up)pInfoEx;

        // Beginnnachricht senden
        tMsgConnectId     MsgConnectId( dwFreeID );

        dh::Log( "[Server]Client %d connected", dwFreeID );
        Send( GR::tMessage::R_SERVER, dwFreeID, MsgConnectId );

        return pInfo;
      }



      bool ClientListener::RemoveClient( SOCKET sockClient )
      {
        tClientInfo*    pInfo = GetClientBySocket( sockClient );
        if ( pInfo )
        {
          tClientInfoEx*  pEx = (tClientInfoEx*)pInfo->m_dwUserData;

          OnClientDisconnects( pEx->m_dwID );

          tMsgDisconnect    MsgDisconnect( pEx->m_dwID );

          delete pEx;
          pInfo->m_dwUserData = 0;

          Listener::RemoveClient( sockClient );

          Send( GR::tMessage::R_SERVER, GR::tMessage::R_ALL, MsgDisconnect );
          return true;
        }
        return false;
      }



      ClientListener::tClientInfoEx* ClientListener::GetClientByID( const GR::u32 dwID )
      {
        std::map<SOCKET,tClientInfo>::iterator    it( m_mapClients.begin() );
        while ( it != m_mapClients.end() )
        {
          tClientInfo& Info( it->second );

          tClientInfoEx*  pEx = (tClientInfoEx*)Info.m_dwUserData;

          if ( ( pEx )
          &&   ( pEx->m_dwID == dwID ) )
          {
            return pEx;
          }

          ++it;
        }
        return NULL;
      }

      bool ClientListener::SendToClient( GR::CSocket& Socket, GR::tMessage& Msg )
      {
        ByteBuffer     Buffer;

        Msg.ToByteBuffer( Buffer );

        int   iSize = (int)Buffer.Size();

        //dh::Log( "Send Msg %d", iSize );
        if ( !Socket.SocketSend( (BYTE*)&iSize, 4 ) )
        {
          dh::Log( "Send Size failed" );
          return false;
        }

        return Socket.SocketSend( (BYTE*)Buffer.Data(), (DWORD)Buffer.Size() );
      }

      bool ClientListener::SendToClient( GR::CSocket& Socket, MemoryStream& MemBuffer )
      {
        GR::u32     dwSize = (GR::u32)MemBuffer.GetSize();

        //dh::Log( "Socket send Msg size (2) %d", dwSize );
        if ( !Socket.SocketSend( (GR::u8*)&dwSize, 4 ) )
        {
          dh::Log( "[Server]Send failed" );
          return false;
        }
        //dh::Log( "calling Socket send size %d", dwSize );
        return Socket.SocketSend( (BYTE*)MemBuffer.Data(), (GR::up)MemBuffer.GetSize() );
      }

      bool ClientListener::Send( GR::u32 dwSender, GR::u32 dwReceiver, MemoryStream& MemBuffer )
      {

        if ( dwReceiver == tMessage::R_ALL )
        {
          // geht an alle
          return SendToAllClients( MemBuffer, dwSender );
        }
        tClientInfoEx*  pClientInfo = GetClientByID( dwReceiver );
        if ( pClientInfo == NULL )
        {
          return false;
        }
        return SendToClient( pClientInfo->m_pInfo->m_Socket, MemBuffer );
      }



      bool ClientListener::Send( GR::u32 dwSender, GR::u32 dwReceiver, tMessage& Msg )
      {

        Msg.m_dwSender    = dwSender;
        Msg.m_dwReceiver  = dwReceiver;

        if ( Msg.m_dwReceiver == tMessage::R_ALL )
        {
          // geht an alle
          return SendToAllClients( Msg );
        }
        tClientInfoEx*  pClientInfo = GetClientByID( Msg.m_dwReceiver );
        if ( pClientInfo == NULL )
        {
          return false;
        }

        return SendToClient( pClientInfo->m_pInfo->m_Socket, Msg );
      }

      bool ClientListener::SendToAllClients( tMessage& Msg, GR::u32 dwSender )
      {
        bool    bResult = true;

        std::map<SOCKET,tClientInfo>::iterator   it( m_mapClients.begin() );
        while ( it != m_mapClients.end() )
        {
          tClientInfoEx*    pInfoEx = (tClientInfoEx*)it->second.m_dwUserData;

          if ( pInfoEx->m_bLoginCompleted )
          {
            Msg.m_dwReceiver  = pInfoEx->m_dwID;
            Msg.m_dwSender    = dwSender;

            if ( !SendToClient( it->second.m_Socket, Msg ) )
            {
              bResult = false;
            }
          }

          ++it;
        }
        return bResult;
      }

      void ClientListener::OnDataReceived( tClientInfo& ClientInfo, const ByteBuffer& bbData )
      {
        tClientInfoEx*    pInfoEx = (tClientInfoEx*)ClientInfo.m_dwUserData;

        pInfoEx->m_bbIncomingData.AppendBuffer( bbData );

        GR::u32         iDataStart = 0,
                        iMessageLength = 0;

        GR::u32         dwParsedData = 0;

        /*
        dh::Log( "Complete stored size %d", m_dwIncomingDataSize );
        dh::Hex( m_pIncomingData, m_dwIncomingDataSize );
        */

        ByteBuffer&    bbIncoming( pInfoEx->m_bbIncomingData );

        BYTE*   pTempData = (BYTE*)bbIncoming.Data();

        if ( bbIncoming.Size() >= sizeof( tMessageHeader ) + 4 )
        {
          while ( iDataStart + 4 <= bbIncoming.Size() )
          {
            iMessageLength = *( (DWORD*)( pTempData + iDataStart ) );
            if ( bbIncoming.Size() >= iDataStart + iMessageLength + 4 )
            {
              tMessageHeader*   pMessage = (tMessageHeader*)( pTempData + iDataStart + 4 );
              if ( !OnMsgReceived( pMessage->m_Type, MemoryStream( pTempData + iDataStart + 4, iMessageLength ) ) )
              {
                // Message not handled, discarding...
              }
              iDataStart += 4 + iMessageLength;
              dwParsedData += 4 + iMessageLength;
              //dh::Log( "ProcessedMessage (%d) - iDataStart %d dwParsedData %d", iMessageLength, iDataStart, dwParsedData );
            }
            else
            {
              //dh::Log( "[Server]Received message (%d) not complete!", iMessageLength );
              break;
            }
          }
          if ( dwParsedData < bbIncoming.Size() )
          {
            // noch nicht alle Daten abgearbeitet
            if ( dwParsedData )
            {
              //dh::Log( "[Server] %d Bytes left for later processing", bbIncoming.Size() - dwParsedData );
              bbIncoming.TruncateFront( dwParsedData );
            }
          }
          else
          {
            //dh::Log( "[Server] no Bytes left for later processing" );
            bbIncoming.Clear();
          }
        }
      }

      bool ClientListener::OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer )
      {

        dh::Log( "[Server]Received Msg %d", Type );
        switch ( Type )
        {
          case tMessage::M_TEXT:
            {
              GR::tMsgText    MsgText( MemBuffer );

              dh::Log( "[Server]Client %d - %s", MsgText.m_dwSender, MsgText.m_strText.c_str() );

              if ( MsgText.m_dwReceiver == tMessage::R_ALL )
              {
                SendToAllClients( MsgText, MsgText.m_dwSender );
                return true;
              }
              else
              {
                tClientInfoEx*  pClientInfo = GetClientByID( MsgText.m_dwReceiver );
                if ( pClientInfo )
                {
                  Send( MsgText.m_dwSender, MsgText.m_dwReceiver, MsgText );
                }
              }
            }
            return true;
          case tMessage::M_USER_INFO:
            {
              tMsgUserInfo  MsgUserInfo( MemBuffer );

              tClientInfoEx*  pClientInfo = GetClientByID( MsgUserInfo.m_dwClientID );
              if ( pClientInfo )
              {
                pClientInfo->m_strName = MsgUserInfo.m_strName;
              }

              SendToAllClients( MsgUserInfo );

              dh::Log( "Server: Client %d nennt sich %s", MsgUserInfo.m_dwClientID, MsgUserInfo.m_strName.c_str() );
              SendUserInfoFromOthers( MsgUserInfo.m_dwClientID );

              pClientInfo->m_bLoginCompleted = true;

              // bevor komplett eingeloggt
              OnClientConnects( MsgUserInfo.m_dwClientID );

              // als Bestätigung, daß der Client jetzt alle Infos über andere User hat
              SendToClient( MsgUserInfo.m_dwClientID, MsgUserInfo );
            }
            return true;
          default:
            {
              tMessage    Msg( MemBuffer );


              if ( Msg.m_dwReceiver == tMessage::R_SERVER )
              {
                break;
              }
              if ( Msg.m_dwReceiver == tMessage::R_ALL )
              {
                SendToAllClients( MemBuffer, Msg.m_dwSender );
              }
              else
              {
                Send( Msg.m_dwSender, Msg.m_dwReceiver, MemBuffer );
              }
            }
            break;
        }
        /*
        dh::Log( "Message Type %d", pMessage->m_Type );
        dh::Log( "Message Length %d", pMessage->m_dwLength );
        */

        return false;

      }

      bool ClientListener::SendUserInfoFromOthers( GR::u32 dwClientID )
      {

        tClientInfoEx*    pNewClientInfo = GetClientByID( dwClientID );
        if ( pNewClientInfo == NULL )
        {
          return false;
        }

        bool      bResult = true;

        std::map<SOCKET,tClientInfo>::iterator   it( m_mapClients.begin() );
        while ( it != m_mapClients.end() )
        {
          if ( ( (tClientInfoEx*)it->second.m_dwUserData )->m_dwID != dwClientID )
          {
            tClientInfo&    ClientInfo = it->second;

            tClientInfoEx*  pInfoEx = (tClientInfoEx*)ClientInfo.m_dwUserData;

            //dh::Log( "[Server]SendUserInfo über %d (%s) an %d (%s)", it->first, ClientInfo.m_strName.c_str(), dwClientID, pNewClientInfo->m_strName.c_str() );
            if ( !SendToClient( ClientInfo.m_Socket, GR::tMsgUserInfo( dwClientID, pInfoEx->m_strName ) ) )
            {
              bResult = false;
            }
          }

          ++it;
        }

        return bResult;

      }

      void ClientListener::OnClientConnects( GR::u32 dwClientID )
      {
        dh::Log( "[Server]Client %d connected", dwClientID );
      }

      void ClientListener::OnClientDisconnects( GR::u32 dwClientID )
      {
        dh::Log( "[Server]Client %d disconnected", dwClientID );
      }

      bool ClientListener::SendToClient( GR::u32 dwReceiver, tMessage& Msg )
      {
        return Send( GR::tMessage::R_SERVER, dwReceiver, Msg );
      }

      bool ClientListener::SendToAllClients( MemoryStream& MemBuffer, GR::u32 dwSender )
      {
        bool    bResult = true;

        std::map<SOCKET,tClientInfo>::iterator   it( m_mapClients.begin() );
        while ( it != m_mapClients.end() )
        {
          tClientInfo&   ClientInfo = it->second;

          if ( !SendToClient( ClientInfo.m_Socket, MemBuffer ) )
          {
            bResult = false;
          }

          ++it;
        }
        return false;

      }

    };
  };
}; // namespace GR





