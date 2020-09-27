#pragma once

#include <GR/GRTypes.h>

#include "Socket.h"



namespace GR
{

struct tMessage;

class CMessageSocket : public CSocket
{

  public:

    CMessageSocket();
    virtual ~CMessageSocket();


    virtual bool        SendGeneralMessage( tMessage& Message );

};


};