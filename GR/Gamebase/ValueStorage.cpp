#include ".\ValueStorage.h"

#include <String/Path.h>



namespace GR
{

  namespace Gamebase
  {

    const GR::Char* ValueStorage::UserAppDataPath( const GR::Char* Filename )
    {
      static GR::String     reply;

      reply = Path::Append( m_UserAppDataPath, Filename );
      return reply.c_str();
    }



    const GR::Char* ValueStorage::AppPath( const GR::Char* Filename )
    {
      static GR::String     reply;

      reply = Path::Append( m_AppDataPath, Filename );
      return reply.c_str();
    }


    void ValueStorage::SetAppPath( const GR::String& Path )
    {
      m_AppDataPath = Path;
    }



    void ValueStorage::SetUserAppPath( const GR::String& Path )
    {
      m_UserAppDataPath = Path;
    }

  }
}