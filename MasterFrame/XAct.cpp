#include <Setup/Groups.h>

#include <Xtreme/XAsset/XAssetLoader.h>

#include <Interface/IService.h>

#include <WinSys/WinUtils.h>
#include <IO/FileUtil.h>

#include <String/Convert.h>
#include <String/Path.h>

#include <Misc/Misc.h>

#include "XAct.h"



namespace XAct
{

  DataStorage& Storage()
  {
    static DataStorage    s_Storage;

    return s_Storage;
  }



  bool ConfigureApp( const GR::String& AssetFile, const GR::String& AppPath )
  {
    Storage().m_AppPath = AppPath;
    if ( ( !AppPath.empty() )
    &&   ( !GR::IO::FileUtil::CreateSubDir( UserAppDataPath() ) ) )
    {
      return false;
    }
    return Xtreme::Asset::XAssetLoader::Instance().LoadAssets( CMisc::AppPath( AssetFile.c_str() ).c_str() );
  }



  GR::String UserAppDataPath( const GR::String& AppPath )
  {
    return Path::Append( Path::Append( CProgramGroups::GetShellFolder( "AppData", true ), GR::Convert::ToString( Storage().m_AppPath ) ), AppPath );
  }



  GR::String AllUsersAppDataPath( const GR::String& AppPath )
  {
    return Path::Append( Path::Append( CProgramGroups::GetShellFolder( "AppData", false ), GR::Convert::ToStringA( Storage().m_AppPath ) ), AppPath );
  }

}

