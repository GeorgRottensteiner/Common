#ifndef GR_GAMEBASE_IVALUESTORAGE_H
#define GR_GAMEBASE_IVALUESTORAGE_H

#include <GR/GRTypes.h>

#include <Lang/Service.h>



namespace GR
{

  namespace Gamebase
  {

    class IValueStorage : public GR::IService
    {

      public:

        virtual const GR::Char*   UserAppDataPath( const GR::Char* Filename ) = 0;
        virtual const GR::Char*   AppPath( const GR::Char* Filename ) = 0;

    };

  }
}


#endif // GR_GAMEBASE_IVALUESTORAGE_H