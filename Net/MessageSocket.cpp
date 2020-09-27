#include "MessageSocket.h"
#include "Message.h"

#include <map>

#include <debug/debugclient.h>


namespace GR
{

  CMessageSocket::CMessageSocket() :
  CSocket()
  {


  }



  CMessageSocket::~CMessageSocket()
  {

  }



  bool CMessageSocket::SendGeneralMessage( tMessage& Message )
  {
  ByteBuffer     Buffer;

  Message.ToByteBuffer( Buffer );

  int   iSize = (int)Buffer.Size();

  //dh::Log( "Send Msg %d", iSize );

  if ( !SocketSend( (BYTE*)&iSize, 4 ) )
  {
    dh::Log( "Send Size failed" );
    return false;
  }
  return SocketSend( (BYTE*)Buffer.Data(), (DWORD)Buffer.Size() );
  }



};  // namespace GR