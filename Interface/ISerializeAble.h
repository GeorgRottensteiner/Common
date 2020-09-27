#ifndef ISERIALIZE_ABLE_H
#define ISERIALIZE_ABLE_H



#include <Interface/IIOStream.h>



struct ISerializeAble
{


  public:


    virtual bool          Save( IIOStream& Stream ) const = 0;
    virtual bool          Load( IIOStream& Stream ) = 0;

};

#endif// ISERIALIZE_ABLE_H

