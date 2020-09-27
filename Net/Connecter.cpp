#include <winsock2.h>

#include "Connecter.h"

#include <Memory/ByteBuffer.h>

#include <debug/debugclient.h>


#pragma comment( lib, "ws2_32.lib" )




namespace GR
{
  namespace Net
  {
    namespace Socket
    {



      Connecter::Connecter() :
        m_bInitialised( false ),
        m_hSocketEvent( WSA_INVALID_EVENT ),
        m_hShutdownEvent( WSA_INVALID_EVENT ),
        m_hThread( NULL ),
        m_DisconnectPending( false )
      {
        // safety measure, 1000 ms
        SetSocketTimeout( 1000 );
      }

      Connecter::~Connecter()
      {
        Shutdown();
      }

      bool Connecter::Initialize()
      {
        if ( !m_bInitialised )
        {
          WSADATA     wsd;

          int iResult = WSAStartup( 0x0202, &wsd );

          if ( iResult )
          {
            return false;
          }

          m_bInitialised = true;
        }

        return true;
      }

      void Connecter::Shutdown()
      {
        if ( m_bInitialised )
        {
          Disconnect();
          ReleaseSocket();
          WSACleanup();
          m_bInitialised = false;
        }
      }

      bool Connecter::Connect( const GR::String& strIP, const GR::u16 wPort )
      {
        if ( !m_bInitialised )
        {
          if ( !Initialize() )
          {
            dh::Log( "Connecter: Initialise failed" );
            return false;
          }
        }
        if ( m_Socket != INVALID_SOCKET )
        {
          // schon listening
          dh::Log( "Connecter: already connected" );
          return false;
        }

	      m_hSocketEvent = WSACreateEvent();
	      if ( m_hSocketEvent == WSA_INVALID_EVENT )
	      {
          dh::Log( "Connecter: WSACreateEvent failed" );
          Disconnect();
		      return false;
	      }

        m_hShutdownEvent = WSACreateEvent();
	      if ( m_hShutdownEvent == WSA_INVALID_EVENT )
	      {
          dh::Log( "Connecter: WSACreateEvent failed" );
          Disconnect();
		      return false;
	      }

	      LPHOSTENT lpHostEntry;
        lpHostEntry = gethostbyname( strIP.c_str() );
	      if ( lpHostEntry == NULL )
	      {
          dh::Log( "Connecter: Gethostbyname failed" );
          Disconnect();
		      return false;
	      }

	      SOCKADDR_IN sa;
	      sa.sin_family = AF_INET;
	      sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	      sa.sin_port = htons( wPort );

	      m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	      if ( m_Socket == INVALID_SOCKET )
	      {
          dh::Log( "Connecter: Socket creation failed" );
          Disconnect();
		      return false;
	      }

	      int nRet = connect( m_Socket, (LPSOCKADDR)&sa, sizeof( SOCKADDR_IN ) );
	      if ( nRet == SOCKET_ERROR )
	      {
		      nRet = WSAGetLastError();
		      if ( nRet == WSAEWOULDBLOCK )
		      {
			      //fprintf(stderr,"\nConnect would block");
		      }
		      else
		      {
            dh::Log( "Connecter: Connect failed" );
            Disconnect();
            m_dwLastError = nRet;
			      return false;
		      }
	      }

        nRet = WSAEventSelect( m_Socket, m_hSocketEvent, FD_ACCEPT | FD_WRITE | FD_READ | FD_CLOSE );
	      if ( nRet == SOCKET_ERROR )
	      {
          dh::Log( "Connecter: WSAEventSelect failed" );
          Disconnect();
		      return false;
	      }

        DWORD dwThreadID = 0;
        m_hThread = CreateThread( NULL, 0, ConnecterProcHelper, (LPVOID)this, 0, &dwThreadID );
        if ( m_hThread == NULL )
        {
          dh::Log( "Connecter: CreateThread failed" );
          Disconnect();
          return false;
        }
        //dh::Log( "Connecter: connected" );
        return true;
      }

      DWORD WINAPI Connecter::ConnecterProcHelper( LPVOID lpParam )
      {
        Connecter*   pConnecter = (Connecter*)lpParam;

        DWORD dwResult = pConnecter->ThreadProc();

        return dwResult;
      }

      DWORD Connecter::ThreadProc()
      {
        WSAEVENT      Events[2];

        while ( true )
        {
          if ( m_DisconnectPending )
          {
            break;
          }
          Events[0] = m_hShutdownEvent;
          Events[1] = m_hSocketEvent;
          //dh::Log( "Connecter: ThreadProc WSAWaitForMultipleEvents (%x/%x)", m_hShutdownEvent, m_hSocketEvent );
          DWORD dwRet = WSAWaitForMultipleEvents( 2, Events, FALSE, m_dwTimeout, FALSE );
          //dh::Log( "Connecter: ThreadProc WSAWaitForMultipleEvents %d (%x/%x)", dwRet, m_hShutdownEvent, m_hSocketEvent );
		      if ( dwRet == WSA_WAIT_TIMEOUT )
          {
            //dh::Log( "Connecter: WSAWaitForMultipleEvents Wait timeout" );
            SendEvent( tSocketEvent( tSocketEvent::SE_TIMEOUT, m_Socket, NULL, 0 ) );
            //dh::Log( "Connecter: -timeout handled" );
          }
          else if ( dwRet == WSA_WAIT_FAILED )
          {
            DWORD LastError = WSAGetLastError();
            //dh::Log( "Connecter: WSAWaitForMultipleEvents Wait failed (Hex %x)", LastError );
            //dh::Log( "-Handle[0] = %x, Handle[1] = %x", Events[0], Events[1] );
            break;
          }
          else if ( dwRet == WSA_WAIT_EVENT_0 )
          {
            //dh::Log( "Connecter: WSAWaitForMultipleEvents Shutdown event" );
            break;
          }
          else if ( dwRet == WSA_WAIT_EVENT_0 + 1 )
          {
            //dh::Log( "Connecter: WSAWaitForMultipleEvents Socket Event received" );
            WSANETWORKEVENTS      events;

            dwRet = WSAEnumNetworkEvents( m_Socket, m_hSocketEvent, &events );
		        if ( dwRet == SOCKET_ERROR )
		        {
              //dh::Log( "Connecter: WSAEnumNetworkEvents failed" );
			        break;
		        }

		        // Connect event?
		        if ( events.lNetworkEvents & FD_CONNECT )
		        {
              //dh::Log( "Connecter: Connect %d", events.iErrorCode[FD_CONNECT_BIT] );
              OnConnected();
		        }
		        // Read event?
		        if ( events.lNetworkEvents & FD_READ )
		        {
              //dh::Log( "Connecter: FD_READ %d", events.iErrorCode[FD_READ_BIT] );
			        // Read the data and write it to stdout
              ByteBuffer     bbTemp;

              bbTemp.Resize( 4096 );

              int nRet = recv( m_Socket, (char*)bbTemp.Data(), 4096, 0 );
			        if ( nRet == SOCKET_ERROR )
			        {
                dh::Error( "Connector: Error while receiving" );
			        }
              else
              {
                //dh::Log( "Received %d bytes", nRet );
                bbTemp.Resize( nRet );
                OnDataReceived( bbTemp );
              }
		        }
		        // Write event?
		        if ( events.lNetworkEvents & FD_WRITE )
		        {
              //dh::Log( "Connecter: FD_WRITE %d", events.iErrorCode[FD_WRITE_BIT] );
		        }
		        // Close event?
		        //if ( events.lNetworkEvents & FD_CLOSE )
            if ( events.lNetworkEvents == FD_CLOSE ) 
		        {
              // only close if close is the last event
              //dh::Log( "Connecter: FD_CLOSE %d", events.iErrorCode[FD_CLOSE_BIT] );

              //dh::Log( "Connecter: Auto-disconnecting" );
              OnDisconnected();
              CloseHandle( m_hThread );
              m_hThread = NULL;
              Disconnect();
              break;
		        }

            // Accept
            if ( events.lNetworkEvents & FD_ACCEPT )
		        {
              dh::Log( "Connecter: FD_ACCEPT %d ????", events.iErrorCode[FD_ACCEPT_BIT] );
		        }

          }
        }
        //dh::Log( "Connecter: Leaving Threadproc (%x)", m_hThread );
        if ( m_hThread != NULL )
        {
          CloseHandle( m_hThread );
          m_hThread = NULL;
        }
        m_DisconnectPending = false;
        return 0;
      }



      void Connecter::DisconnectWithoutWaiting()
      {
        //dh::Log( "Connecter::DisconnectWithoutWaiting called" );
        m_DisconnectPending = true;
        WSASetEvent( m_hShutdownEvent );
        if ( m_hShutdownEvent != WSA_INVALID_EVENT )
        {
          OnDisconnected();

          WSACloseEvent( m_hShutdownEvent );
          m_hShutdownEvent = WSA_INVALID_EVENT;
        }
        if ( m_hSocketEvent != WSA_INVALID_EVENT )
        {
          WSACloseEvent( m_hSocketEvent );
          m_hSocketEvent = WSA_INVALID_EVENT;
        }
        ReleaseSocket();
        m_DisconnectPending = false;
      }

      void Connecter::Disconnect()
      {
        //dh::Log( "Connecter::Disconnect called" );
        if ( m_hShutdownEvent != WSA_INVALID_EVENT )
        {
          WSASetEvent( m_hShutdownEvent );
          //dh::Log( "Connecter::Disconnect shutdown event set (%x)", m_hShutdownEvent );
        }
        // auf Threadproc warten
        //dh::Log( "Connecter::Disconnect wait for thread (%x) to shut down", m_hThread );
        while ( m_hThread )
        {
          Sleep( 10 );
        }

        if ( m_hShutdownEvent != WSA_INVALID_EVENT )
        {
          OnDisconnected();

          WSACloseEvent( m_hShutdownEvent );
          m_hShutdownEvent = WSA_INVALID_EVENT;
        }
        if ( m_hSocketEvent != WSA_INVALID_EVENT )
        {
          WSACloseEvent( m_hSocketEvent );
          m_hSocketEvent = WSA_INVALID_EVENT;
        }
        ReleaseSocket();
        m_DisconnectPending = false;
      }

      bool Connecter::Send( const ByteBuffer& bbData )
      {
        if ( m_Socket == INVALID_SOCKET )
        {
          dh::Log( "Connecter::Send not connected" );
          return false;
        }
        size_t    iDataOffset = 0;
        while ( iDataOffset < bbData.Size() )
        {
          int   iSentBytes = send( m_Socket, (const char*)bbData.DataAt( iDataOffset ), (int)( bbData.Size() - iDataOffset ), 0 );
          if ( iSentBytes == SOCKET_ERROR )
          {
            DWORD dwLastError = WSAGetLastError();
            dh::Log( "send failed %d", dwLastError );
            return false;
          }
          iDataOffset += iSentBytes;
        }
        //dh::Log( "Connecter: Sent %d Bytes", bbData.Size() );
        return true;
      }

      void Connecter::OnDataReceived( ByteBuffer& bbIncoming )
      {
        SendEvent( tSocketEvent( tSocketEvent::SE_DATA_RECEIVED, m_Socket, bbIncoming.Data(), (GR::u32)bbIncoming.Size() ) );
      }

      void Connecter::OnConnected()
      {
        SendEvent( tSocketEvent( tSocketEvent::SE_CONNECTED, m_Socket, NULL, 0 ) );
      }

      void Connecter::OnDisconnected()
      {
        SendEvent( tSocketEvent( tSocketEvent::SE_DISCONNECTED, m_Socket, NULL, 0 ) );
      }

      bool Connecter::IsConnected()
      {
        return Socket() != INVALID_SOCKET;
      }

    };
  };
};