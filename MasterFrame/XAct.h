#ifndef XACT_H
#define XACT_H



#include <GR/GRTypes.h>



namespace XAct
{

  struct DataStorage
  {
    GR::String          m_AppPath;
  };

  bool                  ConfigureApp( const GR::String& AssetFile, const GR::String& AppPath );

  GR::String            UserAppDataPath( const GR::String& AppPath = GR::String() );
  GR::String            AllUsersAppDataPath( const GR::String& AppPath = GR::String() );

  static DataStorage&   Storage();

};



#endif // XACT_H