#ifndef CURL_INTERFACE_H
#define CURL_INTERFACE_H

#include <string>

#include <Memory/ByteBuffer.h>



typedef void CURL;

namespace CurlInterface
{
  class CurlImpl
  {
    public:

      bool            Post( const GR::String& URL, const ByteBuffer& PostParams, ByteBuffer& Result, GR::u32& CurlResultCode, bool TrySystemProxy = true );


    private:

      ByteBuffer      m_ReceivedData;


      static size_t   WriteFunction( void* buf, size_t size, size_t num, void* data );
      static int      DebugFunction( CURL*, int, char* Text, size_t Length, void* );

      bool            SetProxyFromSystem( CURL* curl, const GR::String& URL );
      bool            SetProxyFromIE( CURL* curl, const GR::String& URL );

  };

  bool            Post( const GR::String& URL, const ByteBuffer& PostParams, ByteBuffer& Result, GR::u32& CurlResultCode );
};

#endif // CURL_INTERFACE_H