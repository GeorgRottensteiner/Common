#pragma once

#include <map>
#include <winsock.h>

#include <Interface/IEventProducer.h>

#include <Memory/ByteBuffer.h>



namespace GR
{

struct tSocketData
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
  DWORD               m_dwBytesReceived;
  const sockaddr_in*  m_pSockAddr;

  tSocketData( const eEventType Type, const SOCKET sock, const void* pSrc, DWORD dwBytes, const sockaddr_in* pSockAddr = NULL ) :
    m_Type( Type ),
    m_Socket( sock ),
    m_pData( pSrc ),
    m_dwBytesReceived( dwBytes ),
    m_pSockAddr( pSockAddr )
  {
  }
    
};

class CSocketHandler : public EventProducer<tSocketData>
{

  protected:

    static  std::map<HWND,CSocketHandler*>        m_mapHWNDToSocket;


    bool                m_bInitialized;
    bool                m_bCacheing;

    ByteBuffer         m_bbIncomingData;

    static LRESULT      CALLBACK SocketWndProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );


  public:

    HWND                m_hWndSocket;

    static DWORD        m_dwNotifyMessage;


    CSocketHandler( bool bCacheing = false );
    virtual ~CSocketHandler();


    virtual bool        InitInstance();
    virtual bool        ExitInstance();

    virtual bool        SocketProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );

    virtual void        OnDataReceived( const SOCKET sock, const void* pData, DWORD dwBytesReceived );

    static DWORD        SocketHandlerThreadProc( LPVOID lpParam );

    size_t              CachedDataSize();
    size_t              ReadCachedData( void* pTarget, size_t iDataSize );

};


};