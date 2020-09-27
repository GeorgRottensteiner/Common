#include "Listener.h"

#include <debug/debugclient.h>

#include <Memory/ByteBuffer.h>



#pragma comment( lib, "ws2_32.lib" )




namespace GR
{
  namespace Net
  {
    namespace Socket
    {



      Listener::Listener() :
        m_bInitialised( false ),
        m_hSocketEvent( WSA_INVALID_EVENT ),
        m_hShutdownEvent( WSA_INVALID_EVENT ),
        m_hThread( NULL )
      {

      }

      Listener::~Listener()
      {
        Shutdown();
      }

      bool Listener::Initialize()
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

      void Listener::Shutdown()
      {
        if ( m_bInitialised )
        {
          StopListening();
          ReleaseSocket();
          WSACleanup();
          m_bInitialised = false;
        }
      }

      bool Listener::Listen( const GR::u16 wPort )
      {
        if ( !m_bInitialised )
        {
          if ( !Initialize() )
          {
            dh::Log( "Listener: Initialise failed" );
            return false;
          }
        }
        if ( Socket() != INVALID_SOCKET )
        {
          // schon listening
          dh::Log( "Listener: already listening" );
          return false;
        }

        sockaddr_in     stServerAddress;

        SOCKET ListenSocket = socket( AF_INET, SOCK_STREAM, 0 );
        if ( ListenSocket == INVALID_SOCKET )
        {
          dh::Log( "Listener: Failed to create socket" );
          return false;
        }

        Socket( ListenSocket );

	      m_hSocketEvent = WSACreateEvent();
	      if ( m_hSocketEvent == WSA_INVALID_EVENT )
	      {
          dh::Log( "Listener: WSACreateEvent failed" );
          StopListening();
		      return false;
	      }

        m_hShutdownEvent = WSACreateEvent();
	      if ( m_hShutdownEvent == WSA_INVALID_EVENT )
	      {
          dh::Log( "Listener: WSACreateEvent failed" );
          StopListening();
		      return false;
	      }

        stServerAddress.sin_family      = AF_INET;
        stServerAddress.sin_addr.s_addr = htonl( INADDR_ANY );
        stServerAddress.sin_port        = htons( wPort );
        if ( bind( Socket(), (LPSOCKADDR)&stServerAddress, sizeof( stServerAddress ) ) == SOCKET_ERROR )
        {
          dh::Log( "Listener: Bind failed %d", WSAGetLastError() );
          StopListening();
          return false;
        }

        if ( listen( Socket(), SOMAXCONN ) == SOCKET_ERROR )
        {
          dh::Log( "Listener: Listen failed %d", WSAGetLastError() );
          StopListening();
          return false;
        }

        int nRet = WSAEventSelect( Socket(), m_hSocketEvent, FD_ACCEPT | FD_WRITE | FD_READ | FD_CLOSE );
	      if ( nRet == SOCKET_ERROR )
	      {
          dh::Log( "Listener: WSAEventSelect failed" );
          StopListening();
		      return false;
	      }

        DWORD dwThreadID = 0;
        m_hThread = CreateThread( NULL, 0, ListenerProcHelper, (LPVOID)this, 0, &dwThreadID );
        if ( m_hThread == NULL )
        {
          dh::Log( "Listener: CreateThread failed" );
          StopListening();
          return false;
        }
        //dh::Log( "Listener: Listening on Port %d", wPort );
        return true;
      }

      DWORD WINAPI Listener::ListenerProcHelper( LPVOID lpParam )
      {
        Listener*   pListener = (Listener*)lpParam;

        DWORD dwResult = pListener->ThreadProc();

        return dwResult;
      }

      DWORD Listener::ThreadProc()
      {
        std::vector<WSAEVENT>     VectEvents;

        VectEvents.resize( 2 + m_mapClients.size() );

        VectEvents[0] = m_hShutdownEvent;
        VectEvents[1] = m_hSocketEvent;

        size_t    iIndex = 2;

        std::map<SOCKET,tClientInfo>::iterator    itClient( m_mapClients.begin() );
        while ( itClient != m_mapClients.end() )
        {
          tClientInfo& Info( itClient->second );

          VectEvents[iIndex] = Info.m_SocketEvent;

          ++iIndex;
          ++itClient;
        }

        while ( true )
        {
          DWORD dwRet = WSAWaitForMultipleEvents( (DWORD)VectEvents.size(), &VectEvents[0], FALSE, INFINITE, FALSE );
		      if ( dwRet == WSA_WAIT_TIMEOUT )
          {
            dh::Log( "Listener: WSAWaitForMultipleEvents Wait timeout" );
            break;
          }
          else if ( dwRet == WSA_WAIT_EVENT_0 )
          {
            //dh::Log( "Listener: WSAWaitForMultipleEvents Shutdown event" );
            break;
          }
          else if ( dwRet == WSA_WAIT_EVENT_0 + 1 )
          {
            //dh::Log( "Listener: WSAWaitForMultipleEvents Socket Event received" );
            WSANETWORKEVENTS      events;

            dwRet = WSAEnumNetworkEvents( Socket(), m_hSocketEvent, &events );
		        if ( dwRet == SOCKET_ERROR )
		        {
              dh::Log( "Listener: WSAEnumNetworkEvents failed" );
			        break;
		        }

		        // Connect event?
		        if ( events.lNetworkEvents & FD_CONNECT )
		        {
              dh::Log( "Listener: Connect %d", events.iErrorCode[FD_CONNECT_BIT] );
              SendEvent( tSocketEvent( tSocketEvent::SE_CONNECTED, Socket(), NULL, 0 ) );
		        }
		        // Read event?
		        if ( events.lNetworkEvents & FD_READ )
		        {
              dh::Log( "Listener: FD_READ %d", events.iErrorCode[FD_READ_BIT] );
              /*
              ByteBuffer     bbBuffer;

              bbBuffer.Resize( 4096 );

              fd_set      socks;

              FD_ZERO( &socks );
              FD_SET( m_ListenSocket, &socks );

              select( 
              nRet = recv( m_ListenSocket, 
              */
			        // Read the data and write it to stdout
              /*
			        nRet = recv(Socket, szBuffer, sizeof(szBuffer), 0);
			        if (nRet == SOCKET_ERROR)
			        {
				        PRINTERROR("recv()");
				        break;
			        }
			        fprintf(stderr,"\nRead %d bytes", nRet);
              */
		        }
		        // Close event?
		        if ( events.lNetworkEvents & FD_CLOSE )
		        {
              dh::Log( "Listener: FD_CLOSE %d", events.iErrorCode[FD_CLOSE_BIT] );
              SendEvent( tSocketEvent( tSocketEvent::SE_DISCONNECTED, Socket(), NULL, 0 ) );
		        }

		        // Write event?
		        if ( events.lNetworkEvents & FD_WRITE )
		        {
              dh::Log( "Listener: FD_WRITE %d", events.iErrorCode[FD_WRITE_BIT] );
		        }
            // Accept
            if ( events.lNetworkEvents & FD_ACCEPT )
		        {
              //dh::Log( "Listener: FD_ACCEPT %d", events.iErrorCode[FD_ACCEPT_BIT] );
              if ( events.iErrorCode[FD_ACCEPT_BIT] == 0 )
              {
                int     iClientInfoSize = sizeof( sockaddr );

                sockaddr_in         addrClient;

                SOCKET sockClient = accept( Socket(), (sockaddr*)&addrClient, &iClientInfoSize );

                if ( sockClient != INVALID_SOCKET )
                {
                  AddClient( sockClient );

                  //dh::Log( "Listener: %d Clients connected", m_mapClients.size() );
                  OnClientConnected( sockClient );

                  SendEvent( tSocketEvent( tSocketEvent::SE_CLIENT_CONNECTED, sockClient, NULL, 0, &addrClient ) );

                  VectEvents.resize( 2 + m_mapClients.size() );

                  VectEvents[0] = m_hShutdownEvent;
                  VectEvents[1] = m_hSocketEvent;

                  size_t    iIndex = 2;

                  std::map<SOCKET,tClientInfo>::iterator    itClient( m_mapClients.begin() );
                  while ( itClient != m_mapClients.end() )
                  {
                    tClientInfo& Info( itClient->second );

                    VectEvents[iIndex] = Info.m_SocketEvent;

                    ++iIndex;
                    ++itClient;
                  }
                }
              }
		        }
          }
          else if ( dwRet >= WSA_WAIT_EVENT_0 + 2 )
          {
            //dh::Log( "Listener: WSAWaitForMultipleEvents Client Socket Event received" );
            WSAResetEvent( VectEvents[dwRet - WSA_WAIT_EVENT_0] );

            WSANETWORKEVENTS      events;

            std::map<SOCKET,tClientInfo>::iterator    it( m_mapClients.begin() );
            std::advance( it, dwRet - WSA_WAIT_EVENT_0 - 2 );

            dwRet = WSAEnumNetworkEvents( it->second.m_Socket.Socket(), m_hSocketEvent, &events );
		        if ( dwRet == SOCKET_ERROR )
		        {
              dh::Log( "Listener: WSAEnumNetworkEvents failed" );
			        break;
		        }

            //dh::Log( "Listener: Client Event %d", events.lNetworkEvents );

		        // Connect event?
		        if ( events.lNetworkEvents & FD_CONNECT )
		        {
              //dh::Log( "Listener: Client Connect %d", events.iErrorCode[FD_CONNECT_BIT] );
		        }
		        // Read event?
		        if ( events.lNetworkEvents & FD_READ )
		        {
              //dh::Log( "Listener: Client FD_READ %d", events.iErrorCode[FD_READ_BIT] );

              ByteBuffer     bbBuffer;

              bbBuffer.Resize( 4096 );

              int nRet = recv( it->second.m_Socket.Socket(), (char*)bbBuffer.Data(), 4096, 0 );
			        if ( nRet == SOCKET_ERROR )
			        {
                dh::Error( "Listener: Error while recv data" );
			        }
              else
              {
                bbBuffer.Resize( nRet );
                OnDataReceived( it->second, bbBuffer );
              }
		        }
		        // Close event?
		        if ( events.lNetworkEvents & FD_CLOSE )
		        {
              //dh::Log( "Listener: Client FD_CLOSE %d", events.iErrorCode[FD_CLOSE_BIT] );
              OnClientDisconnected( it->second.m_Socket.Socket() );

              SendEvent( GR::Net::Socket::tSocketEvent( tSocketEvent::SE_CLIENT_DISCONNECTED, it->second.m_Socket.Socket(), NULL, 0 ) );

              RemoveClient( it->second.m_Socket.Socket() );

              //dh::Log( "Listener: %d Clients connected now", m_mapClients.size() );

              VectEvents.resize( 2 + m_mapClients.size() );

              VectEvents[0] = m_hShutdownEvent;
              VectEvents[1] = m_hSocketEvent;

              size_t    iIndex = 2;

              std::map<SOCKET,tClientInfo>::iterator    itClient( m_mapClients.begin() );
              while ( itClient != m_mapClients.end() )
              {
                tClientInfo& Info( itClient->second );

                VectEvents[iIndex] = Info.m_SocketEvent;

                ++iIndex;
                ++itClient;
              }

		        }

		        // Write event?
		        if ( events.lNetworkEvents & FD_WRITE )
		        {
              //dh::Log( "Listener: Client FD_WRITE %d", events.iErrorCode[FD_WRITE_BIT] );
		        }
            // Accept
            if ( events.lNetworkEvents & FD_ACCEPT )
		        {
              dh::Log( "Listener: Client ?? FD_ACCEPT %d", events.iErrorCode[FD_ACCEPT_BIT] );
              if ( events.iErrorCode[FD_ACCEPT_BIT] == 0 )
              {
              }
		        }
          }
        }
        //dh::Log( "Listener: Leaving Threadproc" );
        CloseHandle( m_hThread );
        m_hThread = NULL;
        return 0;
      }

      void Listener::StopListening()
      {
        //dh::Log( "Listener: Stop listening" );

        WSASetEvent( m_hShutdownEvent );
        while ( m_hThread )
        {
          Sleep( 10 );
        }

        // Clients zumachen
        std::map<SOCKET,tClientInfo>::iterator    itClient( m_mapClients.begin() );
        while ( itClient != m_mapClients.end() )
        {
          tClientInfo& Info( itClient->second );

          WSACloseEvent( Info.m_SocketEvent );

          LINGER      ling;

          ling.l_onoff  = TRUE;
          ling.l_linger = 0;
          setsockopt( itClient->first, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof( ling ) );
          closesocket( itClient->first );

          ++itClient;
        }
        m_mapClients.clear();

        if ( m_hShutdownEvent != WSA_INVALID_EVENT )
        {
          WSACloseEvent( m_hShutdownEvent );
          m_hShutdownEvent = WSA_INVALID_EVENT;
        }
        if ( m_hSocketEvent != WSA_INVALID_EVENT )
        {
          WSACloseEvent( m_hSocketEvent );
          m_hSocketEvent = WSA_INVALID_EVENT;
        }
        ReleaseSocket();
      }

      GR::Net::Socket::Listener::tClientInfo* Listener::AddClient( SOCKET sockClient )
      {
        if ( m_mapClients.find( sockClient ) != m_mapClients.end() )
        {
          dh::Error( "Listener: Client Socket already registered!" );
        }

        tClientInfo&    Info( m_mapClients[sockClient] );

        Info.m_SocketEvent = WSACreateEvent();
        Info.m_Socket.Socket( sockClient );

        int nRet = WSAEventSelect( sockClient, Info.m_SocketEvent, FD_ACCEPT | FD_WRITE | FD_READ | FD_CLOSE );
	      if ( nRet == SOCKET_ERROR )
	      {
          dh::Log( "Listener::AddClient WSAEventSelect failed" );
	      }
        return &Info;
      }



      bool Listener::RemoveClient( SOCKET sockClient )
      {
        if ( m_mapClients.find( sockClient ) == m_mapClients.end() )
        {
          dh::Error( "Listener: Client Socket already removed!" );
          return false;
        }

        tClientInfo&    Info( m_mapClients[sockClient] );

        WSACloseEvent( Info.m_SocketEvent );

        m_mapClients.erase( m_mapClients.find( sockClient ) );
        return true;
      }



      bool Listener::SendToClient( SOCKET sockClient, const ByteBuffer& bbData )
      {
        if ( sockClient == INVALID_SOCKET )
        {
          return false;
        }
        return m_mapClients[sockClient].m_Socket.SocketSend( (const char*)bbData.Data(), (int)bbData.Size() );
      }

      bool Listener::SendToAllClients( const ByteBuffer& bbData )
      {
        bool    bResult = true;

        std::map<SOCKET,tClientInfo>::iterator    itClient( m_mapClients.begin() );
        while ( itClient != m_mapClients.end() )
        {
          if ( !SendToClient( itClient->first, bbData ) )
          {
            bResult = false;
          }
          ++itClient;
        }
        return bResult;
      }

      Listener::tClientInfo* Listener::GetClientBySocket( SOCKET sockClient )
      {
        std::map<SOCKET,tClientInfo>::iterator    it( m_mapClients.find( sockClient ) );
        if ( it == m_mapClients.end() )
        {
          return NULL;
        }
        return &it->second;
      }

      void Listener::OnClientConnected( SOCKET sockClient )
      {
      }

      void Listener::OnClientDisconnected( SOCKET sockClient )
      {
      }

      void Listener::OnDataReceived( tClientInfo& ClientInfo, const ByteBuffer& bbData )
      {
        SendEvent( GR::Net::Socket::tSocketEvent( tSocketEvent::SE_DATA_RECEIVED, ClientInfo.m_Socket.Socket(), bbData.Data(), (int)bbData.Size(), 0 ) );
      }

    }
  }
}