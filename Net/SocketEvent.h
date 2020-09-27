#ifndef GR_NET_SOCKET_EVENT
#define GR_NET_SOCKET_EVENT

#include <winsock2.h>

#include <GR/GRTypes.h>

typedef UINT_PTR        SOCKET;

struct sockaddr_in;


namespace GR
{
  namespace Net
  {
    namespace Socket
    {
      struct tSocketEvent
      {
        enum eEventType
        {
          SE_DATA_RECEIVED,
          SE_CONNECTED,
          SE_DISCONNECTED,
          SE_TIMEOUT,
          SE_CLIENT_CONNECTED,
          SE_CLIENT_DISCONNECTED,
        };

        const eEventType  m_Type;

        const SOCKET        m_Socket;
        const void*         m_pData;
        unsigned long       m_dwBytesReceived;
        const sockaddr_in*  m_pSockAddr;

        tSocketEvent( const eEventType Type, const SOCKET sock, const void* pSrc, DWORD dwBytes, const sockaddr_in* pSockAddr = NULL ) :
          m_Type( Type ),
          m_Socket( sock ),
          m_pData( pSrc ),
          m_dwBytesReceived( dwBytes ),
          m_pSockAddr( pSockAddr )
        {
        }
          
      };

    };
  };
};


#endif // GR_NET_SOCKET_EVENT