#include <IO/FileChunk.h>
#include <IO/FileUtil.h>
#include <IO/FileLocator.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik\Image.h>

#include <String/Convert.h>
#include <Xtreme/XAsset/XAssetLoader.h>
#include <Misc/Misc.h>
#include <String/Path.h>
#include <String/XML.h>
#include <String/StringUtil.h>
#include <Setup/Groups.h>

#include <System/Language.h>

#include <Xtreme/NullSound/XNullSound.h>
#include <Xtreme/NullSound/XNullMusic.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetSound.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetScript.h>
#include <Xtreme/XAsset/XAssetSpline.h>
#include <Xtreme/XAsset/XAssetAnimation.h>
#include <Xtreme/XAsset/XAssetTileset.h>
#include <Xtreme/XAsset/XAssetValueList.h>
#include <Xtreme/XAsset/XAssetValueTable.h>
#include <Xtreme/XAsset/XAssetLayeredMap.h>

#include <IO/FileStream.h>

#include "Framework.h"



namespace GR
{

  namespace Gamebase
  {

    GR::u16 Framework::s_FrameworkSettingsChunk = 0xF100;

#if OPERATING_SYSTEM == OS_WEB
    bool    Framework::s_CheckPersistanceFileSystemComplete = false;
#endif



    Framework::Framework() :
      m_MusicVolume( 80 ),
      m_FXVolume( 100 ),
      m_ApplicationActive( false ),
      m_EnableCursor( true ),
      m_CustomMouseCursorSet( false ),
      m_ShutDown( false ),
      m_ExitCode( 0 ),
      m_pTableGUIText( NULL ),
      m_pSoundClass( NULL ),
      m_pInputClass( NULL ),
      m_pMusicPlayer( NULL )
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      ,
      m_hinstCurrentRenderer( NULL ),
      m_hinstCurrentSound( NULL ),
      m_hinstCurrentInput( NULL ),
      m_hinstCurrentMusic( NULL )
#endif
    {
#if OPERATING_SYSTEM == OS_WEB
      s_CheckPersistanceFileSystemComplete = false;
#endif

      ParseCommandLine();

      AddHandler( fastdelegate::MakeDelegate( this, &Framework::OnVarEvent ) );

      GR::Service::Environment::Instance().SetService( "ValueStorage", &m_ValueStorage );
      GR::Service::Environment::Instance().SetService( "Application", this );
      GR::Service::Environment::Instance().SetService( "FileLocator", &GR::IO::FileLocator::Instance() );

      DetectEnvironmentInfo();
    }



    Framework::~Framework()
    {
      GR::Service::Environment::Instance().RemoveService( "SubclassManager" );
      GR::Service::Environment::Instance().RemoveService( "ValueStorage" );
      GR::Service::Environment::Instance().RemoveService( "Application" );
      GR::Service::Environment::Instance().RemoveService( "FileLocator" );
    }



    void Framework::DetectEnvironmentInfo()
    {
#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
      auto versionInfo = Windows::System::Profile::AnalyticsInfo::VersionInfo;

      GR::WString     string = GR::WString( versionInfo->DeviceFamily->Begin(), versionInfo->DeviceFamily->End() );
      GR::String     deviceFamily = GR::Convert::ToUTF8( string );

      if ( deviceFamily == "Windows.Mobile" )
      {
        m_EnvironmentInfo.Type = Xtreme::EnvironmentType::PHONE;
      }
#else
      m_EnvironmentInfo.Type = Xtreme::EnvironmentType::DESKTOP;
#endif
    }



    void Framework::OnVarEvent( LocalRegistryEvent Event, const GR::String& VarName )
    {
      if ( VarName == "FX.Volume" )
      {
        m_FXVolume = GetVarI( "FX.Volume" );
        if ( m_pSoundClass )
        {
          m_pSoundClass->SetMasterVolume( m_FXVolume );
        }
      }
      else if ( VarName == "Music.Volume" )
      {
        m_MusicVolume = GetVarI( "Music.Volume" );
        if ( m_pMusicPlayer )
        {
          m_pMusicPlayer->SetVolume( m_MusicVolume );
        }
      }
      else if ( VarName == "GUI.Language" )
      {
        m_TextDB.SetLangID( GetVarI( "GUI.Language" ) );
      }
    }



    bool Framework::BoundKeyPushed( GR::u32 KeyType )
    {
      tBoundsKeys::iterator   it( m_BoundKeys.find( KeyType ) );
      if ( it == m_BoundKeys.end() )
      {
        return false;
      }
      return m_pInputClass->VKeyPressed( it->second );
    }



    bool Framework::ReleasedBoundKeyPushed( GR::u32 KeyType )
    {
      tBoundsKeys::iterator   it( m_BoundKeys.find( KeyType ) );
      if ( it == m_BoundKeys.end() )
      {
        return false;
      }
      return m_pInputClass->ReleasedVKeyPressed( it->second );
    }



    /// returns the bound key or 0 if not set
    GR::u32 Framework::BoundKey( GR::u32 KeyType )
    {
      tBoundsKeys::iterator   it( m_BoundKeys.find( KeyType ) );
      if ( it == m_BoundKeys.end() )
      {
        return 0;
      }
      return it->second;
    }



    void Framework::SetKeyBinding( GR::u32 KeyType, GR::u32 Key )
    {
      m_BoundKeys[KeyType] = Key;
    }



    void Framework::RemoveKeyBinding( GR::u32 KeyType, GR::u32 Key )
    {
      tBoundsKeys::iterator   it( m_BoundKeys.find( KeyType ) );
      if ( it != m_BoundKeys.end() )
      {
        m_BoundKeys.erase( it );
      }
    }



    void Framework::Clear()
    {
      m_BoundKeys.clear();
      m_MusicVolume = 80;
      m_FXVolume    = 100;
    }



    bool Framework::Save( IIOStream& ioOut )
    {
      GR::IO::FileChunk   Chunk( s_FrameworkSettingsChunk );

      Chunk.AppendU32( (GR::u32)m_BoundKeys.size() );
      tBoundsKeys::iterator   it( m_BoundKeys.begin() );
      while ( it != m_BoundKeys.end() )
      {
        Chunk.AppendU32( it->first );
        Chunk.AppendU32( it->second );

        ++it;
      }
      Chunk.AppendU32( m_FXVolume );
      Chunk.AppendU32( m_MusicVolume );

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      Chunk.AppendString( m_CurrentInputModule );
      Chunk.AppendString( m_CurrentSoundModule );
      Chunk.AppendString( m_CurrentMusicModule );
      Chunk.AppendString( m_CurrentRenderModule );
#else
      Chunk.AppendString( "" );
      Chunk.AppendString( "" );
      Chunk.AppendString( "" );
      Chunk.AppendString( "" );
#endif
      Chunk.AppendU32( m_RenderFrame.m_DisplayMode.Width );
      Chunk.AppendU32( m_RenderFrame.m_DisplayMode.Height );
      Chunk.AppendU32( m_RenderFrame.m_DisplayMode.ImageFormat );
      Chunk.AppendU32( m_RenderFrame.m_DisplayMode.FullScreen ? 1 : 0 );
      Chunk.AppendU32( m_VSyncEnabled ? 1 : 0 );

      Chunk.AppendI32( m_EnvironmentDisplayRect.Left );
      Chunk.AppendI32( m_EnvironmentDisplayRect.Top );
      Chunk.AppendI32( m_EnvironmentDisplayRect.width() );
      Chunk.AppendI32( m_EnvironmentDisplayRect.height() );

      Chunk.AppendI32( m_StoredWindowedMode.Width );
      Chunk.AppendI32( m_StoredWindowedMode.Height );

      return Chunk.Write( ioOut );
    }



    bool Framework::Load( IIOStream& ioIn )
    {
      Clear();

      GR::IO::FileChunk   Chunk;

      if ( !Chunk.Read( ioIn ) )
      {
        return false;
      }
      if ( Chunk.Type() != s_FrameworkSettingsChunk )
      {
        return false;
      }
      MemoryStream  MemIn( Chunk.GetMemoryStream() );

      GR::u32       BoundKeys = MemIn.ReadU32();
      for ( GR::u32 i = 0; i < BoundKeys; ++i )
      {
        GR::u32     KeyIndex = MemIn.ReadU32();
        GR::u32     KeyValue = MemIn.ReadU32();

        SetKeyBinding( KeyIndex, KeyValue );
      }

      m_FXVolume    = MemIn.ReadU32();
      m_MusicVolume = MemIn.ReadU32();

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      m_CurrentInputModule  = MemIn.ReadString();
      m_CurrentSoundModule  = MemIn.ReadString();
      m_CurrentMusicModule  = MemIn.ReadString();
      m_CurrentRenderModule = MemIn.ReadString();
#else
      MemIn.ReadString();
      MemIn.ReadString();
      MemIn.ReadString();
      MemIn.ReadString();
#endif
      m_RenderFrame.m_DisplayMode.Width       = MemIn.ReadU32();
      m_RenderFrame.m_DisplayMode.Height      = MemIn.ReadU32();
      m_RenderFrame.m_DisplayMode.ImageFormat = (GR::Graphic::eImageFormat)MemIn.ReadU32();
      m_RenderFrame.m_DisplayMode.FullScreen  = ( MemIn.ReadU32() == 1 );
      m_VSyncEnabled                          = ( MemIn.ReadU32() == 1 );

      m_EnvironmentDisplayRect.Left   = MemIn.ReadI32();
      m_EnvironmentDisplayRect.Top    = MemIn.ReadI32();
      m_EnvironmentDisplayRect.Right  = m_EnvironmentDisplayRect.Left + MemIn.ReadI32();
      m_EnvironmentDisplayRect.Bottom = m_EnvironmentDisplayRect.Top + MemIn.ReadI32();
      if ( m_EnvironmentDisplayRect.width() == 0 )
      {
        m_EnvironmentDisplayRect.set( 0, 0, m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );
      }

      m_StoredWindowedMode.Width      = MemIn.ReadI32();
      m_StoredWindowedMode.Height     = MemIn.ReadI32();

      if ( m_StoredWindowedMode.Width == 0 )
      {
        m_StoredWindowedMode            = m_RenderFrame.m_DisplayMode;
        m_StoredWindowedMode.FullScreen = false;
      }

      SetVarI( "FX.Volume", m_FXVolume );
      SetVarI( "Music.Volume", m_MusicVolume );
      return true;
    }



    bool Framework::ConfigureApp( const GR::String& AssetFile, const GR::String& AppPath )
    {
      m_AssetProjectToLoad = AssetFile;

      GR::String    userAppData = GR::IO::FileUtil::UserAppDataPath();
      GR::String    appPath = GR::IO::FileUtil::AppPath();
      if ( !Argument( "startuppath" ).empty() )
      {
        appPath = Argument( "startuppath" );
      }

      userAppData = Path::Append( userAppData, GR::Convert::ToStringA( AppPath ) );

#if OPERATING_SYSTEM == OS_WEB
      userAppData = "/app";
      appPath     = "";
#endif

      m_ValueStorage.SetUserAppPath( userAppData );
      m_ValueStorage.SetAppPath( appPath );

      GR::IO::FileLocator::Instance().AddSource( userAppData.c_str() );
      GR::IO::FileLocator::Instance().AddSource( appPath.c_str() );

      if ( ( !userAppData.empty() )
      &&   ( !GR::IO::FileUtil::CreateSubDir( UserAppDataPath() ) ) )
      {
        dh::Log( "CreateSubDir userAppData failed" );
        return false;
      }
      return true;
    }



    GR::String Framework::UserAppDataPath( const GR::String& AppPath )
    {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      return m_ValueStorage.UserAppDataPath( AppPath.c_str() );
#elif OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
      return Path::Append( m_ValueStorage.UserAppDataPath( "" ), AppPath );
#else
      return m_ValueStorage.UserAppDataPath( AppPath.c_str() );
#endif
    }



    GR::String Framework::AllUsersAppDataPath( const GR::String& AppPath )
    {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL )
      // TODO - that's the path for the current user!!
      return m_ValueStorage.AppPath( AppPath.c_str() );
      //return Path::Append( Path::Append( CProgramGroups::GetShellFolder( "AppData", false ), m_ValueStorage.m_AppPath ), strAppPath );
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
      // TODO
      using namespace Windows::Storage;

      // TODO - SharedLocalFolder?
      StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;

      Platform::String^   fullPath = localFolder->Path;

      GR::String    localFolderPath = GR::Convert::ToUTF8( fullPath->Data() );

      return Path::Append( Path::Append( localFolderPath, GR::Convert::ToStringA( m_ValueStorage.UserAppDataPath( "" ) ) ), AppPath );
#endif
    }



    GR::String Framework::AppPath( const GR::String& AppPathPostfix )
    {
      //return GR::IO::FileLocator::Instance().LocateFile( AppPathPostfix.c_str() );
#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
      // TODO - that's the path for the current user!!
      return m_ValueStorage.AppPath( AppPathPostfix.c_str() );
#elif OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
      return m_ValueStorage.AppPath( AppPathPostfix.c_str() );
#else
      // TODO - that's the path for the current user!!
      return m_ValueStorage.AppPath( AppPathPostfix.c_str() );
#endif
    }



    void Framework::ParseCommandLine()
    {
#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) )
      m_CommandLine = GR::Convert::ToUTF8( GetCommandLineW() );

      m_StartParameter.clear();

      unsigned int    pos = 0;

      GR::String      param = "";

      while ( pos < m_CommandLine.length() )
      {
        if ( m_CommandLine[pos] == '"' )
        {
          ++pos;
          while ( pos < m_CommandLine.length() )
          {
            if ( m_CommandLine[pos] == '"' )
            {
              m_StartParameter.push_back( param );
              param = "";
              break;
            }
            else
            {
              param += m_CommandLine[pos];
            }
            ++pos;
          }
        }
        else if ( m_CommandLine[pos] == ' ' )
        {
          if ( !param.empty() )
          {
            m_StartParameter.push_back( param );
          }
          param = "";
        }
        else
        {
          param += m_CommandLine[pos];
        }

        ++pos;
      }
      if ( !param.empty() )
      {
        m_StartParameter.push_back( param );
      }
#endif
    }



    bool Framework::ParameterSwitch( const char* szSwitch )
    {
      std::list<GR::String>::iterator    it( m_StartParameter.begin() );
      while ( it != m_StartParameter.end() )
      {
        GR::String&    param = *it;

        if ( param.empty() )
        {
          ++it;
          continue;
        }

        if ( ( param[0] == '-' )
        ||   ( param[0] == '/' ) )
        {
          if ( param.substr( 1 ) == szSwitch )
          {
            return true;
          }
        }

        ++it;
      }
      return false;
    }



	  GR::String Framework::Argument( const GR::String& ParamName )
	  {
		  std::list<GR::String>::iterator    it( m_StartParameter.begin() );
		  while ( it != m_StartParameter.end() )
		  {
			  GR::String&    param = *it;

			  if ( param.empty() )
			  {
				  ++it;
				  continue;
			  }

			  if ( ( param[0] == '-' )
				||   ( param[0] == '/' ) )
			  {
          if ( GR::Strings::ToUpper( param.substr( 1 ) ) == GR::Strings::ToUpper( ParamName ) )
				  {
            ++it;
            if ( it != m_StartParameter.end() )
            {
              return *it;
            }
            return "";
				  }
			  }
			  ++it;
		  }
		  return "";
	  }



	  GR::u32 Framework::DetermineUserLanguage()
    {
      GR::u32     primaryLangID = System::Globalisation::Language::ENGLISH;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      primaryLangID = GetUserDefaultLangID();
#elif OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
      //CultureInfo currentCulture = Thread::.CurrentThread.CurrentCulture;
      //Windows::Foundation::Collections::IVectorView<String^>^   languages = Windows::System::UserProfile::GlobalizationPreferences::Languages();
      auto pLanguages = Windows::System::UserProfile::GlobalizationPreferences::Languages;

      if ( pLanguages != nullptr )
      {
        Platform::String^   langID = pLanguages->GetAt( 0 );

        auto pLang = ref new Windows::Globalization::Language( langID );
        if ( pLang != nullptr )
        {
          // TODO - map pLang to language ID
        }
      }


#else
      // fallback, stay at english
#endif
      if ( ( m_pTableGUIText == NULL )
      ||   ( m_pTableGUIText->GetLangCount() == 0 ) )
      {
        return primaryLangID;
      }
      for ( GR::u32 i = 0; i < m_pTableGUIText->GetLangCount(); ++i )
      {
        if ( m_pTableGUIText->GetLangID( i ) == primaryLangID )
        {
          return m_pTableGUIText->GetLangID( i );
        }
      }
      // fallback language is english
      if ( IsLanguageSupported( System::Globalisation::Language::ENGLISH ) )
      {
        return System::Globalisation::Language::ENGLISH;
      }
      return primaryLangID;
    }



    bool Framework::IsLanguageSupported( GR::u32 LanguageID ) const
    {
      if ( m_pTableGUIText == NULL )
      {
        return false;
      }
      return m_pTableGUIText->HasLanguage( LanguageID );
    }



    GR::u32 Framework::FindNextAvailableLanguage( GR::u32 LanguageID ) const
    {
      if ( m_pTableGUIText == NULL )
      {
        return LanguageID;
      }
      for ( GR::u32 i = 0; i < m_pTableGUIText->GetLangCount(); ++i )
      {
        if ( m_pTableGUIText->GetLangID( i ) == LanguageID )
        {
          if ( i + 1 < m_pTableGUIText->GetLangCount() )
          {
            return m_pTableGUIText->GetLangID( i + 1 );
          }
          return m_pTableGUIText->GetLangID( 0 );
        }
      }
      return LanguageID;
    }



    GR::u32 Framework::FindPreviousAvailableLanguage( GR::u32 LanguageID ) const
    {
      if ( m_pTableGUIText == NULL )
      {
        return LanguageID;
      }
      for ( GR::u32 i = 0; i < m_pTableGUIText->GetLangCount(); ++i )
      {
        if ( m_pTableGUIText->GetLangID( i ) == LanguageID )
        {
          if ( i > 0 )
          {
            return m_pTableGUIText->GetLangID( i - 1 );
          }
          return m_pTableGUIText->GetLangID( m_pTableGUIText->GetLangCount() - 1 );
        }
      }
      return LanguageID;
    }



    bool Framework::InitialiseAssets()
    {
      GR::Service::Environment::Instance().SetService( "AssetLoader", &Xtreme::Asset::XAssetLoader::Instance() );
      Xtreme::Asset::XAssetLoader*    pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        return false;
      }

      if ( pLoader->AssetTypeCount( Xtreme::Asset::XA_TEXT ) )
      {
        GR::IO::FileStream    ioIn( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_TEXT, 0, "File" ) ).c_str() );

        if ( !ioIn.IsGood() )
        {
          dh::Log( "Failed to load database from %s", AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_TEXT, 0, "File" ) ).c_str() );
        }
        m_TextDB.Load( ioIn );
      }

      // es gibt Assets zu laden
      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_SPLINE ); ++i )
      {
        Xtreme::Asset::XAssetSpline* pSpline = new Xtreme::Asset::XAssetSpline();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_SPLINE, i );
        pSpline->m_Spline.FromXML( pXMLAsset );

        pLoader->SetAsset( Xtreme::Asset::XA_SPLINE, i, pSpline );
      }

      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_SCRIPT ); ++i )
      {
        Xtreme::Asset::XAssetScript* pScript = new Xtreme::Asset::XAssetScript();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_SCRIPT, i );
        pScript->m_Filename = pXMLAsset->Attribute( "File" );
        pLoader->SetAsset( Xtreme::Asset::XA_SCRIPT, i, pScript );
      }

      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_LAYERED_MAP ); ++i )
      {
        Xtreme::Asset::XAssetLayeredMap* pMap = new Xtreme::Asset::XAssetLayeredMap();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_LAYERED_MAP, i );
        pMap->Name = pXMLAsset->Attribute( "Name" );
        pMap->File = pXMLAsset->Attribute( "DataFile" );

        pLoader->SetAsset( Xtreme::Asset::XA_LAYERED_MAP, i, pMap );
      }

      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_ANIMATION ); ++i )
      {
        Xtreme::Asset::XAssetAnimation* pAnim = new Xtreme::Asset::XAssetAnimation();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_ANIMATION, i );

        GR::Strings::XML::iterator    it2( pXMLAsset->FirstChild() );
        while ( it2 != GR::Strings::XML::iterator() )
        {
          GR::Strings::XMLElement*    pXMLFrame( *it2 );

          if ( pXMLFrame->Type() == "Frame" )
          {
            pAnim->m_Anim.AddFrame( pXMLFrame->Attribute( "Section" ),
                          GR::Convert::ToF32( pXMLFrame->Attribute( "Length" ) ) );
          }

          it2 = it2.next_sibling();
        }
        pAnim->m_AnimID = m_AnimationManager.RegisterAnimation( pAnim->m_Anim );

        pLoader->SetAsset( Xtreme::Asset::XA_ANIMATION, i, pAnim );
      }

      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE ); ++i )
      {
        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_IMAGE, i );

        if ( pXMLAsset->Attribute( "AsImage" ) == "1" )
        {
          // load as image too, not only texture
          Xtreme::Asset::XAssetImage* pImage = ( Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );
          if ( pImage == NULL )
          {
            pImage = new Xtreme::Asset::XAssetImage( NULL );
            pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, pImage );
          }

          ImageFormatManager*   pImgFormatManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );

          auto pLoadedImage = pImgFormatManager->LoadData( AppPath( pXMLAsset->Attribute( "File" ) ).c_str() );
          if ( pLoadedImage != NULL )
          {
            pImage->m_Image = *pLoadedImage;
            delete pLoadedImage;
          }
        }
      }

      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION ); ++i )
      {
        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage*  pImage = ( Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, pXMLAsset->Attribute( "Image" ).c_str() );

        if ( ( pImage != NULL )
        &&   ( pImage->m_Image.Width() ) )
        {
          // load as image too, not only texture
          Xtreme::Asset::XAssetImageSection* pImageSection = ( Xtreme::Asset::XAssetImageSection* )pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );
          if ( pImageSection == NULL )
          {
            XTextureSection   tsEmpty;
            pImageSection = new Xtreme::Asset::XAssetImageSection( tsEmpty );
            pLoader->SetAsset( Xtreme::Asset::XA_IMAGE_SECTION, i, pImageSection );
          }

          //dh::Log( "For section %s", pXMLAsset->Attribute( "Name" ).c_str() );
          //dh::Log( "Orig data %x", pImage->m_Image.Data() );
          pImageSection->m_Image.Attach( GR::Convert::ToI32( pXMLAsset->Attribute( "W" ) ),
                                         GR::Convert::ToI32( pXMLAsset->Attribute( "H" ) ),
                                         pImage->m_Image.LineOffsetInBytes(),
                                         pImage->m_Image.ImageFormat(),
                                         pImage->m_Image.GetRowColumnData( GR::Convert::ToI32( pXMLAsset->Attribute( "X" ) ),
                                                                           GR::Convert::ToI32( pXMLAsset->Attribute( "Y" ) ) ) );

          /*
          dh::Log( "sub data %x, offset %d", pImageSection->m_Image.Data(), (int)pImageSection->m_Image.Data() - (int)pImage->m_Image.Data() );

          auto img = pImageSection->m_Image.CreateImage();

          img->Save( "test.igf" );*/
        }
      }

      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_TILESET ); ++i )
      {
        Xtreme::Asset::XAssetTileset* pTileset = new Xtreme::Asset::XAssetTileset();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_TILESET, i );

        GR::Strings::XML::iterator    it2( pXMLAsset->FirstChild() );
        while ( it2 != GR::Strings::XML::iterator() )
        {
          GR::Strings::XMLElement*    pXMLFrame( *it2 );

          if ( pXMLFrame->Type() == "Tile" )
          {
            // Tile Section="Tile.Empty
            if ( pXMLFrame->HasAttribute( "Animation" ) )
            {
              pTileset->AddTileAnimated( pXMLFrame->Attribute( "Animation" ),
                                         GR::Convert::ToU32( pXMLFrame->Attribute( "Type" ) ) );
            }
            else
            {
              pTileset->AddTile( pXMLFrame->Attribute( "Section" ),
                                 GR::Convert::ToU32( pXMLFrame->Attribute( "Type" ) ) );
            }
          }

          it2 = it2.next_sibling();
        }
        pLoader->SetAsset( Xtreme::Asset::XA_TILESET, i, pTileset );
      }

      // ValueList
      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_PARAMETER_TABLE ); ++i )
      {
        Xtreme::Asset::XAssetValueList* pValueList = new Xtreme::Asset::XAssetValueList();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_PARAMETER_TABLE, i );

        GR::Strings::XML::iterator    it2( pXMLAsset->FirstChild() );
        while ( it2 != GR::Strings::XML::iterator() )
        {
          GR::Strings::XMLElement*    pXMLFrame( *it2 );

          pValueList->SetValue( pXMLFrame->Attribute( "Name" ), pXMLFrame->Attribute( "Value" ) );
          it2 = it2.next_sibling();
        }
        /*
        for ( size_t j = 0; j < pXMLAsset->AttributeCount(); ++j )
        {
          GR::String     attrName = pXMLAsset->AttributeName( j );
          if ( attrName != _T( "Name" ) )
          {
            pValueList->SetValue( attrName, pXMLAsset->AttributeValue( j ) );
          }
        }*/
        pLoader->SetAsset( Xtreme::Asset::XA_PARAMETER_TABLE, i, pValueList );
      }

      // ValueTable
      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_VALUE_TABLE ); ++i )
      {
        Xtreme::Asset::XAssetValueTable* pValueList = new Xtreme::Asset::XAssetValueTable();

        GR::Strings::XMLElement* pXMLAsset = pLoader->AssetInfo( Xtreme::Asset::XA_VALUE_TABLE, i );

        size_t  valueIndex = 0;
        GR::Strings::XML::iterator    it2( pXMLAsset->FirstChild() );
        while ( it2 != GR::Strings::XML::iterator() )
        {
          GR::Strings::XMLElement*    pXMLFrame( *it2 );

          pValueList->SetValue( valueIndex, pXMLFrame->GetContent() );
          ++valueIndex;
          it2 = it2.next_sibling();
        }
        pLoader->SetAsset( Xtreme::Asset::XA_VALUE_TABLE, i, pValueList );
      }
      return true;
    }



    void Framework::ReleaseAssets()
    {
      // es gibt Assets zu entladen
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader )
      {
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_SPLINE ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_SPLINE, i );
          pLoader->SetAsset( Xtreme::Asset::XA_SPLINE, i, NULL );
        }
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_SCRIPT ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_SCRIPT, i );
          pLoader->SetAsset( Xtreme::Asset::XA_SCRIPT, i, NULL );
        }
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_LAYERED_MAP ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_LAYERED_MAP, i );
          pLoader->SetAsset( Xtreme::Asset::XA_LAYERED_MAP, i, NULL );

        }
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_ANIMATION ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_ANIMATION, i );
          pLoader->SetAsset( Xtreme::Asset::XA_ANIMATION, i, NULL );
        }
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_TILESET ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_TILESET, i );
          pLoader->SetAsset( Xtreme::Asset::XA_TILESET, i, NULL );
        }
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_PARAMETER_TABLE ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_PARAMETER_TABLE, i );
          pLoader->SetAsset( Xtreme::Asset::XA_PARAMETER_TABLE, i, NULL );
        }
        for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_VALUE_TABLE ); ++i )
        {
          delete pLoader->Asset( Xtreme::Asset::XA_VALUE_TABLE, i );
          pLoader->SetAsset( Xtreme::Asset::XA_VALUE_TABLE, i, NULL );
        }
        GR::Service::Environment::Instance().RemoveService( "AssetLoader" );
        pLoader->ReleaseAllAssets();
      }
    }



    GR::String Framework::ScriptFile( const GR::String& ScriptName ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = ( Xtreme::Asset::XAssetLoader* )GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::ScriptFile Missing AssetLoader Service" );
        return "";
      }

      Xtreme::Asset::XAssetScript* pScript = (Xtreme::Asset::XAssetScript*)pLoader->Asset( Xtreme::Asset::XA_SCRIPT, ScriptName.c_str() );
      if ( pScript == NULL )
      {
        dh::Log( "Framework::ScriptFile Script %s not found", ScriptName.c_str() );
        return "";
      }
      return pScript->Filename();
    }



    GR::String Framework::Script( const GR::String& ScriptName ) const
    {
      GR::String    file = ScriptFile( ScriptName );
      if ( file.empty() )
      {
        return file;
      }

      return GR::IO::FileUtil::ReadFileAsString( file );
    }



    GR::String Framework::ParameterList( const GR::String& ValueListName, const GR::String& ValueKey ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::ParameterList Missing AssetLoader Service" );
        return "";
      }

      Xtreme::Asset::XAssetValueList* pList = (Xtreme::Asset::XAssetValueList*)pLoader->Asset( Xtreme::Asset::XA_PARAMETER_TABLE, ValueListName.c_str() );
      if ( pList == NULL )
      {
        dh::Log( "Framework::ParameterList ValueList %s not found", ValueListName.c_str() );
        return "";
      }
      return pList->Value( ValueKey );
    }



    int Framework::ParameterListCount( const GR::String& ValueListName ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::ParameterListCount Missing AssetLoader Service" );
        return 0;
      }

      Xtreme::Asset::XAssetValueList* pList = (Xtreme::Asset::XAssetValueList*)pLoader->Asset( Xtreme::Asset::XA_PARAMETER_TABLE, ValueListName.c_str() );
      if ( pList == NULL )
      {
        dh::Log( "Framework::ParameterListCount ValueList %s not found", ValueListName.c_str() );
        return 0;
      }
      return pList->ParamCount();
    }

    
    
    GR::String Framework::ParameterListParamName( const GR::String& ValueListName, int Index ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::ParameterList Missing AssetLoader Service" );
        return "";
      }

      Xtreme::Asset::XAssetValueList* pList = (Xtreme::Asset::XAssetValueList*)pLoader->Asset( Xtreme::Asset::XA_PARAMETER_TABLE, ValueListName.c_str() );
      if ( pList == NULL )
      {
        dh::Log( "Framework::ParameterList ValueList %s not found", ValueListName.c_str() );
        return "";
      }
      return pList->ParamName( Index );
    }



    GR::String Framework::ValueTable( const GR::String& ValueListName, size_t Index ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::ValueList Missing AssetLoader Service" );
        return "";
      }

      Xtreme::Asset::XAssetValueTable* pList = (Xtreme::Asset::XAssetValueTable*)pLoader->Asset( Xtreme::Asset::XA_VALUE_TABLE, ValueListName.c_str() );
      if ( pList == NULL )
      {
        dh::Log( "Framework::ValueList ValueList %s not found", ValueListName.c_str() );
        return "";
      }
      return pList->Value( Index );
    }



    GR::i32 Framework::ValueTableI32( const GR::String& ValueListName, size_t Index ) const
    {
      return GR::Convert::ToI32( ValueTable( ValueListName, Index ) );
    }



    int Framework::ValueTableCount( const GR::String& ValueListName ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::ValueTableCount Missing AssetLoader Service" );
        return 0;
      }

      Xtreme::Asset::XAssetValueTable* pList = (Xtreme::Asset::XAssetValueTable*)pLoader->Asset( Xtreme::Asset::XA_VALUE_TABLE, ValueListName.c_str() );
      if ( pList == NULL )
      {
        dh::Log( "Framework::ValueTableCount ValueList %s not found", ValueListName.c_str() );
        return 0;
      }
      return (int)pList->ValueCount();
    }



    const GR::Graphic::ContextDescriptor& Framework::Image( const GR::String& strName ) const
    {
      static    GR::Graphic::ContextDescriptor    cdEmpty;


      Xtreme::Asset::XAssetLoader* pLoader = ( Xtreme::Asset::XAssetLoader* )GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "XFrameApp::Texture No AssetLoader found" );
        return cdEmpty;
      }

      Xtreme::Asset::XAssetImage* pImage = ( Xtreme::Asset::XAssetImage* )pLoader->Asset( Xtreme::Asset::XA_IMAGE, strName.c_str() );
      if ( pImage == NULL )
      {
        dh::Log( "XFrameApp::Image Image %s not found", strName.c_str() );
        return cdEmpty;
      }
      return pImage->CD();
    }



    const GR::Graphic::ContextDescriptor& Framework::ImageSection( const GR::String& strName ) const
    {
      static    GR::Graphic::ContextDescriptor    cdEmpty;


      Xtreme::Asset::XAssetLoader* pLoader = ( Xtreme::Asset::XAssetLoader* )GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "XFrameApp::Texture No AssetLoader found" );
        return cdEmpty;
      }

      Xtreme::Asset::XAssetImageSection* pImage = ( Xtreme::Asset::XAssetImageSection* )pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, strName.c_str() );
      if ( pImage == NULL )
      {
        dh::Log( "XFrameApp::ImageSection ImageSection %s not found", strName.c_str() );
        return cdEmpty;
      }
      return pImage->CD();
    }



    GR::String Framework::LayeredMapFile( size_t Index ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::LayeredMapFile Missing AssetLoader Service" );
        return GR::String();
      }
      Xtreme::Asset::XAssetLayeredMap*    pMap = (Xtreme::Asset::XAssetLayeredMap*)pLoader->Asset( Xtreme::Asset::XA_LAYERED_MAP, Index );
      if ( pMap == NULL )
      {
        dh::Log( "Framework::LayeredMapFile Map not found" );
        return GR::String();
      }
      return pMap->MapFile();
    }



    GR::String Framework::LayeredMapName( size_t Index ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::LayeredMapFile Missing AssetLoader Service" );
        return GR::String();
      }
      Xtreme::Asset::XAssetLayeredMap*    pMap = (Xtreme::Asset::XAssetLayeredMap*)pLoader->Asset( Xtreme::Asset::XA_LAYERED_MAP, Index );
      if ( pMap == NULL )
      {
        dh::Log( "Framework::LayeredMapFile Map not found" );
        return GR::String();
      }
      return pMap->MapName();
    }



    GR::String Framework::LayeredMapFile( const GR::String& MapName ) const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::LayeredMapFile Missing AssetLoader Service" );
        return GR::String();
      }
      int     numMaps = (int)pLoader->AssetTypeCount( Xtreme::Asset::XA_LAYERED_MAP );

      for ( int i = 0; i < numMaps; ++i )
      {
        Xtreme::Asset::XAssetLayeredMap*    pMap = (Xtreme::Asset::XAssetLayeredMap*)pLoader->Asset( Xtreme::Asset::XA_LAYERED_MAP, i );
        if ( pMap->MapName() == MapName )
        {
          return pMap->MapFile();
        }
      }
      return GR::String();
    }



    int Framework::LayeredMapCount() const
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader == NULL )
      {
        dh::Log( "Framework::LayeredMapCount Missing AssetLoader Service" );
        return 0;
      }
      return (int)pLoader->AssetTypeCount( Xtreme::Asset::XA_LAYERED_MAP );
    }


    void Framework::OnPauseApplication()
    {
    }



    void Framework::OnResumeApplication()
    {
    }



    void Framework::InvalidateWindow()
    {
#if OPERATING_SYSTEM == OS_WINDOWS
      m_Window.Invalidate();
#endif
    }



    XMusic* Framework::SwitchMusic( const char* szFileName )
    {
      if ( m_pMusicPlayer )
      {
        m_pMusicPlayer->Release();
      }
      GR::Service::Environment::Instance().RemoveService( "Music" );

      m_pMusicPlayer = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      if ( m_hinstCurrentMusic )
      {
        FreeLibrary( m_hinstCurrentMusic );
        m_hinstCurrentMusic = NULL;
      }
#endif

#if OPERATING_SYSTEM != OS_ANDROID
      if ( szFileName == NULL )
#endif
      {
        static XNullMusic     xNullMusic;

        m_pMusicPlayer = &xNullMusic;
        return NULL;
      }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      m_hinstCurrentMusic = LoadLibraryA( szFileName );
      if ( m_hinstCurrentMusic == NULL )
      {
        m_hinstCurrentMusic = LoadLibraryA( AppPath( szFileName ).c_str() );
      }
      if ( m_hinstCurrentMusic )
      {
        typedef XMusic* ( *tIFunction )( void );
        tIFunction    fGetInterface = (tIFunction)GetProcAddress( m_hinstCurrentMusic, "GetInterface" );

        if ( fGetInterface )
        {
          m_pMusicPlayer = fGetInterface();

          GR::Service::Environment::Instance().SetService( "Music", m_pMusicPlayer );

          if ( !m_pMusicPlayer->Initialize( GR::Service::Environment::Instance() ) )
          {
            return SwitchMusic();
          }
        }
      }
#endif
      EventProducer<tXFrameEvent>::SendEvent( tXFrameEvent( tXFrameEvent::ET_MUSIC_PLAYER_SWITCHED ) );
      return m_pMusicPlayer;
    }



    XSound* Framework::SwitchSound( const char* szFileName )
    {
      GR::Service::Environment::Instance().RemoveService( "Sound" );

      if ( m_pSoundClass )
      {
        m_pSoundClass->Release();
      }
      m_pSoundClass = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      if ( m_hinstCurrentSound )
      {
        FreeLibrary( m_hinstCurrentSound );
        m_hinstCurrentSound = NULL;
      }
#endif

      if ( szFileName == NULL )
      {
        // NULL-Device
        static XNullSound     xNullSound;

        m_pSoundClass = &xNullSound;

        InvalidateWindow();

        EventProducer<tXFrameEvent>::SendEvent( tXFrameEvent( tXFrameEvent::ET_SOUND_SWITCHED ) );
        return NULL;
      }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      m_hinstCurrentSound = LoadLibraryA( szFileName );
      if ( m_hinstCurrentSound == NULL )
      {
        m_hinstCurrentSound = LoadLibraryA( AppPath( szFileName ).c_str() );
      }
      if ( m_hinstCurrentSound )
      {
        typedef XSound* ( *tIFunction )( void );
        tIFunction    fGetInterface = (tIFunction)GetProcAddress( m_hinstCurrentSound, "GetInterface" );

        if ( fGetInterface )
        {
          m_pSoundClass = fGetInterface();
          if ( !m_pSoundClass->Initialize( GR::Service::Environment::Instance() ) )
          {
            return SwitchSound();
          }
        }
        else
        {
          return SwitchSound();
        }
      }
      else
      {
        return SwitchSound();
      }
#endif
      GR::Service::Environment::Instance().SetService( "Sound", m_pSoundClass );

      EventProducer<tXFrameEvent>::SendEvent( tXFrameEvent( tXFrameEvent::ET_SOUND_SWITCHED ) );
      return m_pSoundClass;
    }



    Xtreme::XInput* Framework::SwitchInput( const char* szFileName )
    {
      if ( m_pInputClass )
      {
        m_pInputClass->Release();
      }
      GR::Service::Environment::Instance().RemoveService( "Input" );
      m_pInputClass = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      if ( m_hinstCurrentInput )
      {
        FreeLibrary( m_hinstCurrentInput );
        m_hinstCurrentInput = NULL;
      }
      if ( szFileName == NULL )
      {
        return NULL;
      }
      m_hinstCurrentInput = LoadLibraryA( szFileName );
      if ( m_hinstCurrentInput == NULL )
      {
        m_hinstCurrentInput = LoadLibraryA( AppPath( szFileName ).c_str() );
      }
      if ( m_hinstCurrentInput )
      {
        typedef Xtreme::XInput* ( *tIFunction )( void );
        tIFunction    fGetInterface = (tIFunction)GetProcAddress( m_hinstCurrentInput, "GetInterface" );

        if ( fGetInterface )
        {
          m_pInputClass = fGetInterface();

          if ( !m_pInputClass->Initialize( GR::Service::Environment::Instance() ) )
          {
            return SwitchInput();
          }
        }
      }
#endif
      if ( m_pInputClass == NULL )
      {
        return NULL;
      }

      GR::Service::Environment::Instance().SetService( "Input", m_pInputClass );
      EventProducer<tXFrameEvent>::SendEvent( tXFrameEvent( tXFrameEvent::ET_INPUT_SWITCHED ) );

      return m_pInputClass;
    }



    Xtreme::XInput* Framework::InputClass()
    {
      return m_pInputClass;
    }



    XSound* Framework::SoundClass()
    {
      return m_pSoundClass;
    }



    XMusic* Framework::MusicClass()
    {
      return m_pMusicPlayer;
    }



    int Framework::Run()
    {
      dh::Log( "Override Framework::Run!" );
      return -1;
    }



    bool Framework::RunDefaultModules()
    {
      if ( ParameterSwitch( "f" ) )
      {
        m_RenderFrame.m_DisplayMode.FullScreen = true;
      }
      if ( ParameterSwitch( "w" ) )
      {
        m_RenderFrame.m_DisplayMode.FullScreen = false;
      }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      if ( m_CurrentSoundModule.empty() )
      {
        m_CurrentSoundModule = "DX8Sound.dll";
      }
      SwitchSound( m_CurrentSoundModule.c_str() );
      if ( m_CurrentInputModule.empty() )
      {
        m_CurrentInputModule = "DXInput.dll";
      }
      SwitchInput( m_CurrentInputModule.c_str() );
      if ( m_CurrentMusicModule.empty() )
      {
        m_CurrentMusicModule = "DX8OggPlayer.dll";
      }
      SwitchMusic( m_CurrentMusicModule.c_str() );
#endif
      if ( m_pSoundClass )
      {
        m_pSoundClass->SetMasterVolume( m_FXVolume );
      }
      if ( m_pMusicPlayer )
      {
        m_pMusicPlayer->SetVolume( m_MusicVolume );
      }
      return true;
    }



    void Framework::ReleaseModules()
    {
      if ( m_pSoundClass )
      {
        m_pSoundClass->Release();
        GR::Service::Environment::Instance().RemoveService( "Sound" );
        m_pSoundClass = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        if ( m_hinstCurrentSound )
        {
          FreeLibrary( m_hinstCurrentSound );
          m_hinstCurrentSound = NULL;
        }
#endif
      }

      if ( m_pMusicPlayer )
      {
        m_pMusicPlayer->Release();
        GR::Service::Environment::Instance().RemoveService( "Music" );
        m_pMusicPlayer = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        if ( m_hinstCurrentMusic )
        {
          FreeLibrary( m_hinstCurrentMusic );
          m_hinstCurrentMusic = NULL;
        }
#endif
      }

      if ( m_pInputClass )
      {
        m_pInputClass->Release();
        GR::Service::Environment::Instance().RemoveService( "Input" );
        m_pInputClass = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        if ( m_hinstCurrentInput )
        {
          FreeLibrary( m_hinstCurrentInput );
          m_hinstCurrentInput = NULL;
        }
#endif
      }
    }



    void Framework::OnCharEntered( char Key )
    {
    }



    void Framework::OnKeyDown( char Key )
    {
    }



    void Framework::OnKeyUp( char Key )
    {
    }



    bool Framework::IsActive() const
    {
      return m_ApplicationActive;
    }



    void Framework::SetActive( bool Active )
    {
      m_ApplicationActive = Active;
    }



    bool Framework::CursorEnabled() const
    {
      return m_EnableCursor;
    }



    void Framework::EnableCursor( bool bEnable )
    {
      if ( m_EnableCursor != bEnable )
      {
        m_EnableCursor = bEnable;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        if ( ( m_EnableCursor )
        &&   ( !m_CustomMouseCursorSet ) )
        {
          SetCursor( LoadCursor( NULL, IDC_ARROW ) );
        }
        else
        {
          SetCursor( NULL );
        }
#endif
      }
    }



    void Framework::ShutDown( GR::i32 ExitCode )
    {
      m_ExitCode = ExitCode;
      m_ShutDown = true;
    }



    void Framework::OnSizeChanged()
    {
    }



    void Framework::OnDisplayFrame()
    {
    }



    bool Framework::IsQuittable()
    {
#if OPERATING_SYSTEM == OS_WEB
      return false;
#else
      return true;
#endif
    }
  }
}


