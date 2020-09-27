#include "MessageClient.h"

#include <debug/debugclient.h>

#include "Message.h"



namespace GR
{
  namespace Net
  {
    namespace Socket
    {


      MessageClient::MessageClient() :
        m_dwID( tMessage::R_ALL )
      {

      }



      MessageClient::~MessageClient()
      {

      }



      bool MessageClient::OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer )
      {

        dh::Log( "[Client%d]Received Message Type %d", m_dwID, Type );
        switch ( Type )
        {
          case tMessage::M_CONNECT_ID:
            // meine ID ist da
            {
              tMsgConnectId   MsgConnectId( MemBuffer );

              m_dwID = MsgConnectId.m_dwClientID;

              dh::Log( "Client: Connect with ID %d", m_dwID );
              m_mapClients[m_dwID].m_strName = m_strUserName;

              SendMessageToServer( GR::tMsgUserInfo( m_dwID, m_strUserName ) );

              OnConnected();
            }
            return true;
          case tMessage::M_DISCONNECT:
            {
              tMsgDisconnect    MsgDisconnect( MemBuffer );

              dh::Log( "[Client%d]>Client %d verläßt uns", m_dwID, MsgDisconnect.m_dwClientID );
              OnUserDisconnected( MsgDisconnect.m_dwClientID );

              tMapClients::iterator   itClient( m_mapClients.find( MsgDisconnect.m_dwClientID ) );
              if ( itClient != m_mapClients.end() )
              {
                m_mapClients.erase( itClient );
              }
            }
            return true;
          case tMessage::M_TEXT:
            {
              tMsgText   MsgText( MemBuffer );
              dh::Log( "[Client%d]Message %s", m_dwID, MsgText.m_strText.c_str() );

              OnTextMessage( MsgText.m_dwSender, MsgText.m_strText );
            }
            break;
          case tMessage::M_USER_INFO:
            {
              tMsgUserInfo    MsgUserInfo( MemBuffer );

              m_mapClients[MsgUserInfo.m_dwClientID].m_strName = MsgUserInfo.m_strName;
              dh::Log( "[Client%d]Client %d nennt sich %s", m_dwID, MsgUserInfo.m_dwClientID, MsgUserInfo.m_strName.c_str() );

              OnUserConnected( MsgUserInfo.m_dwClientID );
            }
            break;
        }

        return false;

      }



      MessageClient::tClientInfo* MessageClient::GetClientInfo( const GR::u32 dwID )
      {

        tMapClients::iterator   itClient( m_mapClients.find( dwID ) );
        if ( itClient == m_mapClients.end() )
        {
          return NULL;
        }
        return &itClient->second;

      }



      GR::u32 MessageClient::ClientID() const
      {

        return m_dwID;

      }



      void MessageClient::OnDataReceived( ByteBuffer& bbIncoming )
      {
        dh::Log( "[Client%d] %d Bytes received", ClientID(), bbIncoming.Size() );

        DWORD   iDataStart = 0,
                iMessageLength = 0;

        m_bbIncomingData.AppendBuffer( bbIncoming );

        GR::u32        dwParsedData = 0;


        BYTE*   pTempData = (BYTE*)m_bbIncomingData.Data();

        if ( m_bbIncomingData.Size() >= sizeof( tMessageHeader ) + 4 )
        {
          while ( iDataStart < m_bbIncomingData.Size() )
          {
            iMessageLength = *( (DWORD*)( pTempData + iDataStart ) );
            //dh::Log( "Recv Msg Size %d", iMessageLength );
            if ( m_bbIncomingData.Size() >= iDataStart + iMessageLength + 4 )
            {
              tMessageHeader*   pMessage = (tMessageHeader*)( pTempData + iDataStart + 4 );
              //dh::Log( "Recvd Msg Type %d", pMessage->m_Type );
              if ( !OnMsgReceived( pMessage->m_Type, MemoryStream( pTempData + iDataStart + 4, iMessageLength ) ) )
              {
                // Message not handled, discarding...
              }
              iDataStart += 4 + iMessageLength;
              dwParsedData += 4 + iMessageLength;
            }
            else
            {
              /*
              dh::Log( "[Client%d]Received message not complete! (%d >= %d)", ClientID(),
                    m_dwIncomingDataSize, iMessageLength + 4 );
                    */
              m_bbIncomingData.TruncateFront( dwParsedData );
              return;
            }
          }
          if ( dwParsedData < m_bbIncomingData.Size() )
          {
            // noch nicht alle Daten abgearbeitet
            //dh::Log( "[Client%d] %d Bytes left for later processing", ClientID(), m_dwIncomingDataSize - dwParsedData );
            m_bbIncomingData.TruncateFront( dwParsedData );
          }
          else
          {
            //dh::Log( "[Client%d] no Bytes left for later processing", ClientID() );
            m_bbIncomingData.Clear();
          }
        }

      }



      bool MessageClient::SendGeneralMessage( tMessage& Message )
      {
        ByteBuffer     Buffer;

        Message.ToByteBuffer( Buffer );

        int   iSize = (int)Buffer.Size();

        //dh::Log( "Send Msg %d", iSize );

        if ( !SocketSend( (BYTE*)&iSize, 4 ) )
        {
          dh::Log( "Send Size failed" );
          return false;
        }
        return SocketSend( (BYTE*)Buffer.Data(), (DWORD)Buffer.Size() );
      }



      bool MessageClient::SendMessageToOthers( tMessage& Message )
      {

        Message.m_dwSender    = ClientID();
        Message.m_dwReceiver  = tMessage::R_ALL;

        return SendGeneralMessage( Message );

      }



      bool MessageClient::SendMessageToServer( tMessage& Message )
      {

        Message.m_dwSender    = ClientID();
        Message.m_dwReceiver  = tMessage::R_SERVER;

        return SendGeneralMessage( Message );

      }



      void MessageClient::OnTextMessage( GR::u32 dwSenderID, const GR::String& strText )
      {
      }


      void MessageClient::OnConnected()
      {
        dh::Log( "Client: Connected" );
        Connecter::OnConnected();
      }

      void MessageClient::OnDisconnected()
      {
        dh::Log( "Client: Disconnected" );
        Connecter::OnDisconnected();
      }


      void MessageClient::OnUserConnected( GR::u32 dwClientID )
      {
      }

      void MessageClient::OnUserDisconnected( GR::u32 dwClientID )
      {
      }

      bool MessageClient::Connect( const GR::String& strIPAddress, 
                                   GR::u16 wPort,
                                   const GR::String& strUser, 
                                   const GR::String& strPassword )
      {
        if ( !Connecter::Connect( strIPAddress, wPort ) )
        {
          return false;
        }

        m_strUserName = strUser;
        m_strPassword = strPassword;

        return true;
      }

    };
  };
};

