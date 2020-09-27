#ifndef GR_NET_SOCKET_CLIENT
#define GR_NET_SOCKET_CLIENT

#ifndef INCL_WINSOCK_API_TYPEDEFS
#define INCL_WINSOCK_API_TYPEDEFS 1
#endif

#ifndef INCL_WINSOCK_API_TYPEDEFS
#define INCL_WINSOCK_API_TYPEDEFS 0
#endif

#include <Winsock2.h>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>

#include <Interface/IEventProducer.h>

#include "SocketEvent.h"
#include "Socket.h"

#define WSAEVENT                HANDLE


namespace GR
{
  namespace Net
  {
    namespace Socket
    {

      class Connecter : public GR::CSocket,
                        public EventProducer<GR::Net::Socket::tSocketEvent>
      {

        private:

          bool                m_bInitialised;

          WSAEVENT            m_hSocketEvent;
          WSAEVENT            m_hShutdownEvent;

          volatile HANDLE     m_hThread;
          volatile bool       m_DisconnectPending;



          static DWORD WINAPI ConnecterProcHelper( LPVOID lpParam );

          DWORD               ThreadProc();
          


        public:

          Connecter();
          ~Connecter();

          bool                Initialize();

          void                Shutdown();

          bool                Connect( const GR::String& strIP, const GR::u16 wPort );
          void                Disconnect();
          void                DisconnectWithoutWaiting();

          bool                Send( const ByteBuffer& bbData );

          virtual void        OnDataReceived( ByteBuffer& bbIncoming );

          virtual void        OnConnected();
          virtual void        OnDisconnected();

          bool                IsConnected();

      };

    };
  };
};


#endif // GR_NET_SOCKET_CLIENT