#ifndef GR_GAMEBASE_FRAMEWORK_H
#define GR_GAMEBASE_FRAMEWORK_H

#include <GR/GRTypes.h>

#include <OS/OS.h>

#include <GR/LocalRegistry/LocalRegistry.h>
#include <GR/Gamebase/ValueStorage.h>
#include <Grafik/ContextDescriptor.h>

#include <map>

#include <Xtreme/XRenderer.h>
#include <Xtreme/XSound.h>
#include <Xtreme/XMusic.h>
#include <Xtreme/XInput.h>

#include <Interface/IRenderFrame.h>

#include <GR/Database/DatabaseTable.h>
#include <GR/Database/Database.h>

#include <Interface/IAnimationManager.h>

#include <Xtreme/Environment/EnvironmentConfig.h>
#include <Xtreme/Environment/EnvironmentInfo.h>

#include "XFrameEvent.h"

#if OPERATING_SYSTEM == OS_WINDOWS
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
#include <Xtreme/Environment/XWindowWin32.h>
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
#include <Xtreme/Environment/XWindowUniversalApp.h>
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
#include <Xtreme/Environment/XWindowSDL.h>
#endif
#elif OPERATING_SYSTEM == OS_ANDROID
#include <Xtreme/Environment/XWindowAndroid.h>
#elif OPERATING_SYSTEM == OS_WEB
#include <Xtreme/Environment/XWindowSDL.h>
#endif



namespace GR
{

  namespace Gamebase
  {

    class Framework : public EventProducer<tXFrameEvent>,
                      public LocalRegistry
    {

      private:

        ValueStorage            m_ValueStorage;

        typedef std::map<GR::u32,GR::u32>       tBoundsKeys;


        tBoundsKeys             m_BoundKeys;

        GR::String              m_CommandLine;



        void                    DetectEnvironmentInfo();

      protected:

        bool                    m_ApplicationActive;
        bool                    m_EnableCursor;
        bool                    m_CustomMouseCursorSet;
        bool                    m_ShutDown;
        GR::i32                 m_ExitCode;


#if OPERATING_SYSTEM == OS_WINDOWS
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
        RenderFrame             m_RenderFrame;

        HMODULE                 m_hinstCurrentSound,
                                m_hinstCurrentInput,
                                m_hinstCurrentMusic,
                                m_hinstCurrentRenderer;

        GR::String              m_CurrentSoundModule,
                                m_CurrentMusicModule,
                                m_CurrentInputModule,
                                m_CurrentRenderModule;

        Xtreme::Win32Window     m_Window;
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
      public:
        RenderFrame             m_RenderFrame;

        Xtreme::UniversalAppWindow  m_Window;
      protected:
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
        Xtreme::SDLWindow       m_Window;

        RenderFrame             m_RenderFrame;
#endif
#elif OPERATING_SYSTEM == OS_WEB
        Xtreme::SDLWindow       m_Window;

        RenderFrame             m_RenderFrame;


      public:

        static bool             s_CheckPersistanceFileSystemComplete;

      protected:
#else
#if OPERATING_SYSTEM == OS_ANDROID
      public:

        Xtreme::AndroidWindow   m_Window;

      protected:
#endif
        RenderFrame             m_RenderFrame;
#endif


        std::list<GR::String>   m_StartParameter;

        GR::Database::Table*    m_pTableGUIText;

        GR::String              m_AssetProjectToLoad;

        XMusic*                 m_pMusicPlayer;
        XSound*                 m_pSoundClass;
        Xtreme::XInput*         m_pInputClass;



        void                    ParseCommandLine();
        bool                    ParameterSwitch( const char* szSwitch );
        GR::String              Argument( const GR::String& ParamName );

        void                    OnVarEvent( LocalRegistryEvent Event, const GR::String& VarName );


      public:

        GR::u32                 m_MusicVolume;
        GR::u32                 m_FXVolume;
        bool                    m_VSyncEnabled;

        GR::Database::Database  m_TextDB;

        IAnimationManager<GR::String>       m_AnimationManager;

        Xtreme::EnvironmentConfig               m_EnvironmentConfig;
        Xtreme::EnvironmentInfo m_EnvironmentInfo;

        GR::tRect               m_EnvironmentDisplayRect;

        XRendererDisplayMode    m_StoredWindowedMode;



        Framework();
        virtual ~Framework();


        GR::u32                 BoundKey( GR::u32 KeyType );
        bool                    BoundKeyPushed( GR::u32 KeyType );
        bool                    ReleasedBoundKeyPushed( GR::u32 KeyType );

        void                    SetKeyBinding( GR::u32 KeyType, GR::u32 Key );
        void                    RemoveKeyBinding( GR::u32 KeyType, GR::u32 Key );


        void                    Clear();
        virtual bool            Save( IIOStream& ioOut );
        virtual bool            Load( IIOStream& ioIn );

        bool                    ConfigureApp( const GR::String& AssetFile, const GR::String& AppPath );
        GR::String              UserAppDataPath( const GR::String& AppPath = GR::String() );
        GR::String              AllUsersAppDataPath( const GR::String& AppPath = GR::String() );
        GR::String              AppPath( const GR::String& AppPath = GR::String() );

        GR::u32                 DetermineUserLanguage();
        bool                    IsLanguageSupported( GR::u32 LanguageID ) const;
        GR::u32                 FindNextAvailableLanguage( GR::u32 LanguageID ) const;
        GR::u32                 FindPreviousAvailableLanguage( GR::u32 LanguageID ) const;

        XSound*                 SoundClass();
        XMusic*                 MusicClass();
        Xtreme::XInput*         InputClass();

        XSound*                 SwitchSound( const char* FileName = NULL );
        XMusic*                 SwitchMusic( const char* FileName = NULL );
        Xtreme::XInput*         SwitchInput( const char* FileName = NULL );
        virtual bool            SwitchRenderer( const char* FileName = NULL ) = 0;

        bool                    InitialiseAssets();
        void                    ReleaseAssets();

        virtual void            Configure( Xtreme::EnvironmentConfig& Configuration ) = 0;

        virtual int             Run();
        virtual bool            RunDefaultModules();
        virtual void            ReleaseModules();

        void                    ShutDown( GR::i32 ExitCode = 0 );

        GR::String              ScriptFile( const GR::String& ScriptName ) const;
        GR::String              Script( const GR::String& ScriptName ) const;

        const GR::Graphic::ContextDescriptor& Image( const GR::String& Name ) const;
        const GR::Graphic::ContextDescriptor& ImageSection( const GR::String& Name ) const;

        GR::String            ParameterList( const GR::String& ValueListName, const GR::String& ValueKey ) const;
        int                   ParameterListCount( const GR::String& ValueListName ) const;
        GR::String            ParameterListParamName( const GR::String& ValueListName, int Index ) const;

        GR::String            ValueTable( const GR::String& ValueListName, size_t Index ) const;
        GR::i32               ValueTableI32( const GR::String& ValueListName, size_t Index ) const;
        int                   ValueTableCount( const GR::String& ValueListName ) const;

        GR::String            LayeredMapName( size_t Index ) const;
        GR::String            LayeredMapFile( size_t Index ) const;
        GR::String            LayeredMapFile( const GR::String& MapName ) const;
        int                   LayeredMapCount() const;

        virtual void          OnPauseApplication();
        virtual void          OnResumeApplication();

        void                  InvalidateWindow();

        bool                  IsActive() const;
        void                  SetActive( bool Active = true );

        void                  EnableCursor( bool bEnable = true );
        bool                  CursorEnabled() const;

        virtual void          OnCharEntered( char Key );
        virtual void          OnKeyDown( char Key );
        virtual void          OnKeyUp( char Key );

        virtual void          OnSizeChanged();
        virtual void          OnDisplayFrame();

        // web has no quit
        bool                  IsQuittable();

    };

  }
}


#endif // GR_GAMEBASE_FRAMEWORK_H