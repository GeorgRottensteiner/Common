#ifndef INCLUDE_HTTPCONNECTION_H
#define INCLUDE_HTTPCONNECTION_H



#pragma comment( lib, "wsock32" )

#include <winsock2.h>
#include "HTTPBody.h"


#include <string>
#include <list>

#include <Interface/IEventProducer.h>

#include <Memory/ByteBuffer.h>

#include <Net/Connecter.h>



class CHTTPConnection : public EventProducer<CHTTPBody>,
                        public EventListener<GR::Net::Socket::tSocketEvent>
{

  protected:

    enum eResult
    {
      RES_READY,
      RES_WAITING,
      RES_FAILED,
      RES_TIMEOUT,
      RES_SUCCESS,
    };

    GR::String         m_strProxyServer,
                        m_strProxyUser,
                        m_strProxyUserPassword,
                        m_strCurrentProtocol,
                        m_strCurrentHost,
                        m_strCurrentURL,
                        m_strCurrentURLPath;

    WORD                m_wProxyPort,
                        m_wCurrentPort;

    GR::u32             m_AllowedTimeOuts;
    GR::u32             m_CurrentTimeOuts;

    bool                m_bUseProxy,
                        m_bAutoCloseConnection,
                        m_bSSLConnection,
                        m_bBinaryDataConnection,
                        m_bCancel;

    size_t              m_dwContentLength;

    ByteBuffer         m_bbIncomingData;

    CHTTPBody           m_SentBody,
                        m_ReceivedBody;

    GR::Net::Socket::Connecter      m_Connecter;


    bool                Connect();


	public:

		WSADATA             WSAData;

    /*
		struct sockaddr_in  server_addr;

		struct hostent*     hostent;
    */

		BOOL                m_bHeaderRead;

    GR::String         strURL;

		DWORD               ThreadID;

    int                 m_iAnswerCode,
                        m_iResult;

    char                m_bTempBuffer[16384 + 1];

    GR::String         m_strTemp;


		CHTTPConnection();
		virtual ~CHTTPConnection();


		bool                Request( CHTTPBody& Body, const GR::String& strURL, const WORD wPort = 80 );
    bool                SendBinary( const void* pData, const size_t iSize, bool bDoNotParseAnswer = false );

    void                Ready();
		void                Cancel();

    void                AutoCloseConnection( bool bClose = true );

    void                ConnectionConfig( const GR::String& strProxy = "",
                                          const WORD wProxyPort = 0,
                                          const GR::String& strProxyUser = "",
                                          const GR::String& strProxyPassword = "" );

    void                AddContent( const void* pData, DWORD dwBytes );

    const CHTTPBody&    SentBody() const;

    int                 Result();
    int                 AnswerCode() const;

    bool                UsingSSL() const;
    bool                UsingProxy() const;

    GR::String         ProxyServer() const;
    WORD                ProxyPort() const;

    virtual bool        ProcessEvent( const GR::Net::Socket::tSocketEvent& SocketEvent );

    void                ProcessData();

    bool                IsBodyComplete( const ByteBuffer& bbIncoming );

};


#endif // INCLUDE_HTTPCONNECTION_H