#ifndef SDLSOUND_H_INCLUDED
#define SDLSOUND_H_INCLUDED



#include <Xtreme/Audio/XSoundBase.h>

#include <Xtreme/XMusic.h>

#include <vector>


#ifdef EMSCRIPTEN
#include <SDL2/SDL_mixer.h>
#else
#include "SDL_mixer.h"
#endif



class IDebugService;

class SDLSound : public XSoundBase, 
                 public XMusic
{

  protected:

    struct ChannelInfo
    {
      int             Volume;
      int             SoundID;

      ChannelInfo() :
        Volume( 0 ),
        SoundID( -1 )
      {
      }
    };

    IDebugService*          m_pDebugger;

    std::vector<ChannelInfo>  m_Channels;

    std::vector<Mix_Chunk*> m_Sounds;

    int                     m_NumChannels;


    // music 
    virtual bool            Initialize( GR::IEnvironment& Environment );



  public:

    SDLSound();
    virtual ~SDLSound();


    virtual bool            Initialize( GR::IEnvironment& Environment, GR::u32 Channels = 16  );
    virtual bool            Release();

    virtual void            ReleaseAssets();

    virtual bool            IsInitialized();

    virtual bool            SetMasterVolume( GR::u32 ucVolume, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         MasterVolume( const eSoundType sType = ST_SOUND_EFFECT );

    virtual GR::u32         LoadWave( const char* Name, bool LoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         LoadWave( const GR::String& Name, bool LoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         LoadWave( IIOStream& Stream, bool LoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         LoadWaveFromResource( const char* Resource, const char* Type, const eSoundType sType = ST_SOUND_EFFECT );

    virtual GR::u32         CreateBufferFromMemory( int Channels, int SamplesPerSec, int BlockAlign,
                                                    int BitsPerSample, GR::up* pData, GR::up dwSize,
                                                    bool LoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         CreateModifyableBuffer( int Channels, int SamplesPerSec, int BlockAlign,
                                                    int BitsPerSample, GR::up dwSize,
                                                    bool LoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual bool            LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                                  void** pBlock1, GR::u32* dwBlock1Size,
                                                  void** pBlock2, GR::u32* dwBlock2Size );
    virtual bool            UnlockModifyableBuffer( GR::u32 ID );

    virtual bool            ReleaseWave( GR::u32 ID );

    virtual bool            Play( GR::u32 ID, GR::u32 Offset = 0 );
    virtual bool            Loop( GR::u32 ID );
    virtual bool            PlayDetail( GR::u32 ID, GR::i32 Volume, GR::i32 Pan = 0, GR::u32 Frequency = 0, bool Looping = false );
    virtual bool            Stop( GR::u32 ID );
    virtual bool            StopAll();
    virtual bool            IsPlaying( GR::u32 ID );

    virtual GR::u32         FindFreeChannel();
    virtual void            ReserveChannel( GR::u32 Channel );
    virtual void            UnreserveChannel( GR::u32 Channel );
    virtual bool            PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset = 0,
                                         GR::u32 Volume = 100, GR::i32 Pan = 0, GR::u32 Frequency = 0, bool Looping = false );
    virtual bool            LoopChannel( GR::u32 ID, GR::u32 Channel );
    virtual bool            StopChannel( GR::u32 Channel );
    virtual bool            IsChannelPlaying( GR::u32 Channel );

    virtual bool            Resume( GR::u32 ID );
    virtual bool            Pause( GR::u32 ID );

    virtual bool            SetFrequency( GR::u32 ID, GR::u32 Frequency );

    virtual GR::u32         GetOriginalFrequency( GR::u32 ID );
    virtual GR::u32         GetCurrentFrequency( GR::u32 ID );

    virtual bool            SetPos( GR::u32 ID, GR::u32 Pos );
    virtual GR::u32         GetPos( GR::u32 ID );

    virtual bool            SetPan( GR::u32 ID, signed long Pan );

    virtual bool            SetVolume( GR::u32 ID, GR::u32 ucVolume );
    virtual GR::u32         GetVolume( GR::u32 ID );

    virtual bool            SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency );

    virtual GR::u32         GetChannelOriginalFrequency( GR::u32 Channel );
    virtual GR::u32         GetChannelCurrentFrequency( GR::u32 Channel );

    virtual bool            SetChannelPos( GR::u32 Channel, GR::u32 Pos );
    virtual GR::u32         GetChannelPos( GR::u32 Channel );

    virtual bool            SetChannelPan( GR::u32 Channel, signed long Pan );

    virtual bool            SetChannelVolume( GR::u32 Channel, GR::u32 Volume );
    virtual GR::u32         GetChannelVolume( GR::u32 Channel );

    virtual bool            Create3dListener();
    virtual bool            Set3dListenerPosition( float X, float Y, float Z );
    virtual bool            Set3dListenerOrientation( float FrontX, float FrontY, float FrontZ,
                                                      float UpX, float UpY, float UpZ );
    virtual bool            Set3dListenerParams( float X, float Y, float Z,
                                                 float FrontX, float FrontY, float FrontZ,
                                                 float UpX, float UpY, float UpZ,
                                                 float VelocityX, float VelocityY, float VelocityZ );
    virtual bool            Set3dDistanceFactor( float Distance );
    virtual bool            Set3dDopplerFactor( float Doppler );
    virtual bool            Set3dRollOffFactor( float RollOff );

    virtual bool            Play3d( GR::u32 ID, float X, float Y, float Z, bool Looping = false );
    virtual bool            Set3dPosition( GR::u32 ID, float X, float Y, float Z );

    virtual void            ProcessEvent( const tGlobalEvent& Event );


    void                    Log( const GR::String& strSystem, const char* szFormat, ... );
    GR::String              AppPath( const GR::String& Path );


    // music
    Mix_Music*              m_pMusic;



    virtual bool            SetVolume( int Volume );
    virtual int             Volume();

    virtual bool            Play( bool Looped = true );
    virtual void            Stop();
    virtual bool            IsPlaying();

    virtual bool            Resume();
    virtual bool            Pause();

    virtual bool            LoadMusic( const GR::Char* FileName );
    virtual bool            LoadMusic( IIOStream& Stream );

};


#endif //__SOUND_H_INCLUDED__