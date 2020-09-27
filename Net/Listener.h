#ifndef GR_NET_SOCKET_SERVER
#define GR_NET_SOCKET_SERVER

#include <Winsock2.h>

#include <GR/GRTypes.h>

#include <Interface/IEventProducer.h>

#include <map>

#include "SocketEvent.h"
#include "Socket.h"



class ByteBuffer;

namespace GR
{
  namespace Net
  {
    namespace Socket
    {

      class Listener : public GR::CSocket, 
                       public EventProducer<tSocketEvent>
      {

        protected:

          struct tClientInfo
          {
            WSAEVENT            m_SocketEvent;
            GR::CSocket         m_Socket;
            GR::up              m_dwUserData;

            tClientInfo() :
              m_SocketEvent( WSA_INVALID_EVENT ),
              m_dwUserData( 0 )
            {
            }
          };

          std::map<SOCKET,tClientInfo>      m_mapClients;

          virtual tClientInfo*  AddClient( SOCKET sockClient );
          virtual bool          RemoveClient( SOCKET sockClient );


        private:


          bool                m_bInitialised;

          WSAEVENT            m_hSocketEvent;
          WSAEVENT            m_hShutdownEvent;

          HANDLE              m_hThread;



          static DWORD WINAPI ListenerProcHelper( LPVOID lpParam );

          DWORD               ThreadProc();
          


        public:

          Listener();
          ~Listener();

          bool                Initialize();

          void                Shutdown();

          bool                Listen( const GR::u16 wPort );
          void                StopListening();

          bool                SendToClient( SOCKET sockClient, const ByteBuffer& bbData );
          bool                SendToAllClients( const ByteBuffer& bbData );

          virtual void        OnClientConnected( SOCKET sockClient );
          virtual void        OnClientDisconnected( SOCKET sockClient );
          virtual void        OnDataReceived( tClientInfo& ClientInfo, const ByteBuffer& bbData );

          tClientInfo*        GetClientBySocket( SOCKET sockClient );

      };

    };
  };
};


#endif // GR_NET_SOCKET_SERVER