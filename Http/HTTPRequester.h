#ifndef _INCLUDE_HTTPREQUESTER_H__
#define _INCLUDE_HTTPREQUESTER_H__


#include <winsock2.h>
#include <HTTP/HTTPConnection.h>

#include <Interface/IEventListener.h>

#include <Memory/ByteBuffer.h>



class CSSLConnection;

class CHTTPRequester : public EventListener<CHTTPBody>,
                       public EventProducer<CHTTPBody>
{

  protected:

    CHTTPConnection       m_Connection;

    CSSLConnection*       m_pSSLConnection;

    bool                  m_bSSLWithProxyWaitForConnection;



	public:


    CHTTPRequester();
    virtual ~CHTTPRequester();


    bool                  Request( const GR::String& strAddress, const WORD wPort = 80 );

    bool                  Post( const GR::String& strAddress, const WORD wPort = 80 );

    void                  ConnectionConfig( const GR::String& strProxy, const WORD wProxyPort,
                                            const GR::String& strProxyUser = "", const GR::String& strProxyPassword = "" );
    bool                  ConnectionConfigFromIERegistry();
    GR::String           ProxyServer() const;
    WORD                  ProxyPort() const;

    virtual bool          ProcessEvent( const CHTTPBody& Body );

    bool                  Abort();


};


#endif // _INCLUDE_HTTPREQUESTER_H__