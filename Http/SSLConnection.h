#ifndef _SSL_CONNECTION_H_
#define _SSL_CONNECTION_H_

// HTTPClient.h: interface for the HTTPClient class.
//
//////////////////////////////////////////////////////////////////////

#include <Memory/ByteBuffer.h>
#include <HTTP/HTTPConnection.h>

#include <Codec/MD5.h>
#include <Codec/SHA-1.h>


#include <string>
#include <list>


class CSSLConnection
{

  protected:

    enum eSSLHandshakeStep
    {
      SSL_NONE,
      SSL_CONNECTING,
      SSL_SEND_CLIENT_HELLO,
    };


    GR::Codec::MD5        m_MD5;

    GR::Codec::SHA1       m_SHA1;

    eSSLHandshakeStep     m_SSLHandshakeStep;

    ByteBuffer            m_sslBuffer;


    void                  ParseServerHello( BYTE* pData, size_t iLength );


  public:


    CSSLConnection();
    ~CSSLConnection();


    void                  SSLHandShake( CHTTPConnection& Connection, const ByteBuffer& memReceived );

};


#endif // _SSL_CONNECTION_H_