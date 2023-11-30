#ifndef GR_GAMEBASE_VALUESTORAGE_H
#define GR_GAMEBASE_VALUESTORAGE_H

#include <GR/GRTypes.h>

#include <Lang/Service.h>

#include <Interface/IValueStorage.h>



namespace GR
{
  namespace Gamebase
  {

    class ValueStorage : public GR::Service::ServiceImpl<IValueStorage>
    {
      private:

        GR::String        m_UserAppDataPath;
        GR::String        m_AllUsersAppDataPath;
        GR::String        m_AppDataPath;


      public:

        virtual const GR::Char*         UserAppDataPath( const GR::Char* Filename );
        virtual const GR::Char*         AllUsersAppDataPath( const GR::Char* Filename );
        virtual const GR::Char*         AppPath( const GR::Char* Filename );


        void SetAppPath( const GR::String& Path );
        void SetUserAppPath( const GR::String& Path );
        void SetAllUsersAppPath( const GR::String& Path );

    };

  }
}


#endif // GR_GAMEBASE_VALUESTORAGE_H