#pragma once

#include <XSound/XMusic.h>

#include <Interface/IService.h>

#include <Xtreme/Audio/XAudio2Sound.h>

#include <string>

#include <windows.h>
#include <mmsystem.h>

//#include <dsound.h>

#include <vorbis/codec.h>

#include <vorbis/vorbisfile.h>



namespace Xtreme
{
  class XOggXAudio2  : public XMusic
  {

    private:

      GR::String              m_Filename;

      bool                    m_FileLoaded;

      bool                    m_Looping;

      bool                    m_PlaybackDone;

      bool                    m_PlayThreadActive;

      XAudio2Sound            m_XAudio;

      int                     m_PlayBufferID;



    protected:

      bool                    GetPlayThreadActive();

      void                    SetPlayThreadActive( bool bActive );

      OggVorbis_File          m_VorbisFile;

      bool                    m_Paused;

      CRITICAL_SECTION        m_CriticalSection;

      HANDLE                  m_PlayThread;

      HANDLE                  m_StopPlaybackEvent;

      GR::u32                 m_Volume;


      bool                    Allocate();

      void                    Cleanup();

      bool                    Fill( const bool FirstHalf );

      static unsigned int WINAPI PlayingThread( LPVOID lpParam );



    public:

      XOggXAudio2();

      virtual bool            Initialize( GR::IEnvironment& Environment );

      virtual bool            Release();

      virtual bool            IsInitialized();

      virtual bool            SetVolume( int iVolume );
      virtual int             Volume();

      virtual bool            LoadMusic( const GR::Char* szFileName );
      virtual bool            LoadMusic( IIOStream& Stream );



      virtual ~XOggXAudio2();



      GR::String GetFileName();

      //!Returns true if the OGG is playing, otherwise false

      bool IsPlaying();

      //!Stops the OGG

      void Stop();

      //!Starts streaming the OGG

      virtual bool            Play( bool Looped = true );

      virtual bool            Resume();
      virtual bool            Pause();

  };

}