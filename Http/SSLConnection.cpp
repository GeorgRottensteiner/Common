#include <winsock2.h>
#include <debug/debugclient.h>

#include <HTTP/SSLConnection.h>



CSSLConnection::CSSLConnection() :
  m_SSLHandshakeStep( SSL_NONE )
{
}



CSSLConnection::~CSSLConnection()
{
}


void CSSLConnection::SSLHandShake( CHTTPConnection& Connection, const ByteBuffer& memReceived )
{

  if ( m_SSLHandshakeStep == SSL_NONE )
  {
    // Client-Hello
    dh::Log( "Send Client Hello\n" );
    m_SSLHandshakeStep = SSL_SEND_CLIENT_HELLO;

    ByteBuffer      sslBlock( 33 );

    BYTE*   pData = (BYTE*)sslBlock.Data();

    pData[0] = 0x80;      // Size
    pData[1] = 0x1f;      // Size Pt.2

    // Client says Hello!
    pData[2] = 0x01;

    // Client wishes to use Version 2.0 later
    pData[3] = 0x03;
    pData[4] = 0x00;

    // Cipher Specs Length = 6
    pData[5] = 0x00;
    pData[6] = 0x06;

    // Session ID = 0
    pData[7] = 0x00;
    pData[8] = 0x00;

    // Challenge Length = 16
    pData[9] = 0x00;
    pData[10] = 0x10;

    // Challenge Specs Data
    pData[11] = 0x00;
    pData[12] = 0x00;
    pData[13] = 0x03; 

    pData[14] = 0x00;
    pData[15] = 0x00;
    pData[16] = 0x06;

    pData[17] = 0x90;
    pData[18] = 0x06;
    pData[19] = 0x46;

    pData[20] = 0x69;
    pData[21] = 0x20;
    pData[22] = 0x81;

    pData[23] = 0x64;
    pData[24] = 0x08;
    pData[25] = 0xba;
    pData[26] = 0xb4;
    pData[27] = 0x3f;
    pData[28] = 0x9f;
    pData[29] = 0x81;
    pData[30] = 0xfa;
    pData[31] = 0x5b;
    pData[32] = 0x20;

    ByteBuffer    SHA1Hash;

    m_SHA1.CalcHash( sslBlock, SHA1Hash );
    /*
    for ( int i = 0; i < 5; ++i )
    {
      dh::Log( "SHA1 State (%d) = %x\n", i, m_SHA1.State( i ) );
    }*/

    m_MD5.update( pData + 2, 31 );
  
    
    for ( int i = 0; i < 4; ++i )
    {
      dh::Log( "MD5 State (%d) = %x\n", i, m_MD5.State( i ) );
    }


    Connection.SendBinary( pData, sslBlock.Size(), true );
  }
  else if ( m_SSLHandshakeStep == SSL_SEND_CLIENT_HELLO )
  {
    if ( ( m_sslBuffer.Size() )
    &&   ( memReceived.Size() ) )
    {
      ByteBuffer      memTemp( m_sslBuffer.Size() + memReceived.Size() );

      memcpy( (BYTE*)memTemp.Data(), m_sslBuffer.Data(), m_sslBuffer.Size() );
      memcpy( ( (BYTE*)memTemp.Data() ) + m_sslBuffer.Size(), memReceived.Data(), memReceived.Size() );

      m_sslBuffer = memTemp;
    }
    else
    {
      m_sslBuffer = ByteBuffer( memReceived );
    }

    dh::Log( "SSLBuffer Size %d\n", m_sslBuffer.Size() );

    if ( m_sslBuffer.Size() >= 5 )
    {
      BYTE*   pData = (BYTE*)m_sslBuffer.Data();

      // Prüfen, ob alle Pakete komplett sind
      size_t    iPacketLength = ( pData[2] << 16 ) + ( pData[3] << 8 ) + pData[4],
                iOffset = 0;

      bool  bComplete = false;

      while ( true )
      {
        if ( iOffset + 5 + iPacketLength == m_sslBuffer.Size() )
        {
          bComplete = true;
          break;
        }
        if ( iOffset + 5 + iPacketLength < m_sslBuffer.Size() )
        {
          if ( iOffset + 5 + iPacketLength < m_sslBuffer.Size() )
          {
            iOffset += 5 + iPacketLength;
            iPacketLength = ( pData[iOffset + 2] << 16 ) + ( pData[iOffset + 3] << 8 ) + pData[iOffset + 4];
          }
          else
          {
            break;
          }

        }
        else
        {
          break;
        }
      }

      if ( bComplete )
      {
        dh::Hex( pData, m_sslBuffer.Size() );

        ParseServerHello( pData, m_sslBuffer.Size() );
      }
    }
    dh::Log( "received server hello (%d Bytes)\n", memReceived.Size() );
  }

}



void CSSLConnection::ParseServerHello( BYTE* pData, size_t iLength )
{

  BYTE    bHeader[5];

  size_t  iOffset = 0;


  while ( iOffset < iLength )
  {
    memcpy( bHeader, pData + iOffset, 5 );

    size_t    iPacketLength = ( pData[2] << 16 ) + ( pData[3] << 8 ) + pData[4];

    iOffset += 5;

    pData += 5;

    switch ( *pData )
    {
      case 2:
        {
          dh::Log( "Server Hello\n" );

          /*
          pData++;

          size_t    iHelloLength = ( *pData++ << 16 ) + ( *pData++ << 8 ) + *pData++;

          dh::Log( "Server SSL Version %d.%d\n", *pData++, *pData++ );
          
          for ( int i = 0; i < 32; ++i )
          {
            dh::Log( "Server Random %d = %d\n", i, *pData++ );
          }

          dh::Log( "SessionID Length %d\n", *pData++ );

          for ( int i = 0; i < 32; ++i )
          {
            dh::Log( "SessionID %d = %d\n", i, *pData++ );
          }

          dh::Log( "Cipher Suite %d.%d\n", *pData++, *pData++ );
          dh::Log( "Compression %d\n", *pData++ );
          */
        }
        break;
      case 0x0b:
        dh::Log( "Server Certificate Handshake\n" );
        break;
      case 0x0c:
        dh::Log( "server_key_exchange\n" );
        break;
      case 0x0e:
        dh::Log( "Server Hello done\n" );
        break;
      default:
        dh::Log( "unknown %x\n", *pData );
        break;
    }
    dh::Log( "Size %d Bytes\n", iPacketLength );

    iOffset += iPacketLength;
    pData += iPacketLength;
  }

}

/*
enum {
        hello_request(0), client_hello(1), server_hello(2),
        certificate(11), server_key_exchange (12), certificate_request(13),
        server_done(14), certificate_verify(15), client_key_exchange(16),
        finished(20), (255)
} HandshakeType;

struct {
        HandshakeType msg_type;         // type of handshake message
        uint24 length;  // # bytes in handshake msg body
        select (HandshakeType) {
            case hello_request: HelloRequest;
            case client_hello: ClientHello;
            case server_hello: ServerHello;
            case certificate: Certificate;
            case server_key_exchange: ServerKeyExchange;
            case certificate_request: CertificateRequest;
            case server_done: ServerHelloDone;
            case certificate_verify: CertificateVerify;
            case client_key_exchange: ClientKeyExchange;
            case finished: Finished;
        } body;
} Handshake;
*/