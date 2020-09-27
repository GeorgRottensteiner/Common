#pragma once

#include <winsock2.h>

#include <map>
#include <string>

#include "Connecter.h"
#include "Message.h"


namespace GR
{
  namespace Net
  {
    namespace Socket
    {

      class MessageClient : public Connecter
      {

        public:

          struct tClientInfo
          {
            GR::u32           m_dwID;
            GR::String        m_strName;

            tClientInfo() :
              m_dwID( 0 ),
              m_strName( "" )
            {
            }
          };


        protected:

          typedef std::map<GR::u32,tClientInfo>   tMapClients;


          GR::u32                           m_dwID;

          bool                              m_bConnected;

          GR::String                       m_strUserName,
                                            m_strPassword;

          tMapClients                       m_mapClients;

          ByteBuffer                        m_bbIncomingData;


        public:


          MessageClient();
          virtual ~MessageClient();


          bool                              Connect( const GR::String& strIPAddress, 
                                                     GR::u16 dwPort,
                                                     const GR::String& strUser = GR::String(), 
                                                     const GR::String& strPassword = GR::String() );
          GR::u32                           ClientID() const;

          bool                              OnMsgReceived( const GR::tMessage::eMType Type, MemoryStream& MemBuffer );

          void                              OnDataReceived( ByteBuffer& bbIncoming );

          virtual void                      OnTextMessage( GR::u32 dwSenderID, const GR::String& strText );
          virtual void                      OnConnected();
          virtual void                      OnDisconnected();
          virtual void                      OnUserConnected( GR::u32 dwClientID );
          virtual void                      OnUserDisconnected( GR::u32 dwClientID );

          tClientInfo*                      GetClientInfo( const GR::u32 dwID );

          bool                              SendMessageToServer( tMessage& Message );
          bool                              SendMessageToOthers( tMessage& Message );

          bool                              SendGeneralMessage( tMessage& Message );

      };

    };
  };
};