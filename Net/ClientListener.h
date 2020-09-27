#pragma once

#include <winsock2.h>
#include <map>

#include <GR/GRTypes.h>

#include "Listener.h"
#include "Message.h"



namespace GR
{
  namespace Net
  {
    namespace Socket
    {

      class ClientListener : public Listener
      {

        protected:

          struct tClientInfoEx
          {
            GR::u32       m_dwID;
            bool          m_bLoginCompleted;
            GR::up        m_dwUserData;
            GR::String    m_strName;
            ByteBuffer    m_bbIncomingData;
            tClientInfo*  m_pInfo;

            tClientInfoEx() :
              m_dwID( 0 ),
              m_bLoginCompleted( false ),
              m_dwUserData( 0 ),
              m_pInfo( NULL )
            {
            }
          };

          bool                SendUserInfoFromOthers( GR::u32 dwClientID );
          bool                SendToClient( GR::CSocket& Socket, GR::tMessage& Msg );


        public:

          ClientListener();
          ~ClientListener();

          tClientInfo*        AddClient( SOCKET sockClient );
          bool                RemoveClient( SOCKET sockClient );

          tClientInfoEx*      GetClientByID( const GR::u32 dwID );

          bool                Send( GR::u32 dwSender, GR::u32 dwReceiver, GR::tMessage& Msg );
          bool                Send( GR::u32 dwSender, GR::u32 dwReceiver, MemoryStream& MemBuffer );

          bool                SendToClient( GR::u32 dwReceiver, GR::tMessage& Msg );
          bool                SendToClient( GR::CSocket& Socket, MemoryStream& MemBuffer );

          bool                SendToAllClients( tMessage& Msg, GR::u32 dwSender = GR::tMessage::R_SERVER );
          bool                SendToAllClients( MemoryStream& MemBuffer, GR::u32 dwSender );

          void                OnDataReceived( tClientInfo& ClientInfo, const ByteBuffer& bbData );

          bool                OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer );

          void                OnClientConnects( GR::u32 dwClientID );
          void                OnClientDisconnects( GR::u32 dwClientID );

      };


    };
  };
};