#include "XSoundBase.h"

#include <debug/DebugService.h>

#include <Interface/IValueStorage.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetSound.h>



XSoundBase::XSoundBase() :
  m_pDebugger( NULL )
{
}



XSoundBase::~XSoundBase()
{
  //Release();
}



GR::String XSoundBase::AppPath( const GR::String& Path )
{
  if ( m_pEnvironment == NULL )
  {
    Log( "DXSound::AppPath no environment" );
    return CMisc::AppPath( Path.c_str() );
  }
  GR::Gamebase::IValueStorage* pStorage = ( GR::Gamebase::IValueStorage* )m_pEnvironment->Service( "ValueStorage" );
  if ( pStorage == NULL )
  {
    Log( "DXSound::AppPath no storage" );
    return CMisc::AppPath( Path.c_str() );
  }
  return pStorage->AppPath( Path.c_str() );
}



void XSoundBase::Log( const char* szFormat, ... )
{
  if ( m_pDebugger )
  {
#if OPERATING_SYSTEM == OS_WEB
    // TODO
#else
    static char    szMiscBuffer[5000];
    vsprintf_s( szMiscBuffer, 5000, szFormat, (char*)( &szFormat + 1 ) );

    m_pDebugger->LogDirect( "XSoundBase", szMiscBuffer );
#endif
  }
}



void XSoundBase::LoadAssets()
{
  Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader )
  {
    // es gibt Assets zu laden
    GR::up    imageCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_SOUND );
    for ( GR::up i = 0; i < imageCount; ++i )
    {
      GR::Strings::XMLElement* pXmlAsset( pLoader->AssetInfo( Xtreme::Asset::XA_SOUND, i ) );

      Xtreme::Asset::XAsset* pAsset = pLoader->Asset( Xtreme::Asset::XA_SOUND, i );

      GR::u32     handle = LoadWave( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_SOUND, i, "File" ) ).c_str() );
      if ( handle == 0 )
      {
        dh::Log( "XSound: Failed to load wave asset %s", AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_SOUND, i, "File" ) ).c_str() );
      }
      else
      {
        delete pAsset;

        Log( "Sound.General", CMisc::printf( "Loaded %s", AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_SOUND, i, "File" ) ).c_str() ) );

        pAsset = new Xtreme::Asset::XAssetSound( handle );
        pLoader->SetAsset( Xtreme::Asset::XA_SOUND, i, pAsset );
      }
    }
  }
}



void XSoundBase::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      GR::up    assetCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_SOUND );
      for ( GR::up i = 0; i < assetCount; ++i )
      {
        Xtreme::Asset::XAsset* pAsset = pLoader->Asset( Xtreme::Asset::XA_SOUND, i );

        delete pAsset;
        pLoader->SetAsset( Xtreme::Asset::XA_SOUND, i, NULL );
      }
    }
  }
}



