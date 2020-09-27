#ifndef XAUDIO2_SOUND_H
#define XAUDIO2_SOUND_H



#include "XSoundBase.h"

#include <xaudio2.h>

#include <Memory/ByteBuffer.h>



namespace Xtreme
{
  class XAudio2Sound : public XSoundBase
  {
    private:

      struct tX2AudioSound
      {
        IXAudio2SourceVoice*    pSourceVoice;
        XAUDIO2_BUFFER          XA2Buffer;

        tX2AudioSound() :
          pSourceVoice( NULL )
        {
          memset( &XA2Buffer, 0, sizeof( XA2Buffer ) );
        }
      };

      struct tSoundInfo
      {
        unsigned char           Volume;
        signed long             Pan;
        GR::u32                 Frequency,
                                OriginalFrequency,
                                Flags;
        GR::up                  Size;
        std::list<tX2AudioSound>  SoundVoices;
        ByteBuffer              SoundData;
        WAVEFORMATEXTENSIBLE    WaveFormat;
        bool                    Looping,
                                Modifyable,
                                Locked;
        GR::u32                 ID;
        eSoundType              Type;


        tSoundInfo() :
          Volume( 100 ),
          Pan( 0 ),
          Size( 0 ),
          Frequency( 0 ),
          OriginalFrequency( 0 ),
          Flags( 0 ),
          Looping( false ),
          Modifyable( false ),
          ID( 0 ),
          Locked( false ),
          Type( ST_INVALID )
        {
          memset( &WaveFormat, 0, sizeof( WaveFormat ) );
        }
      };


      IXAudio2*               m_pX2Audio;

      IXAudio2MasteringVoice* m_pMasteringVoice;

      std::map<GR::u32,tSoundInfo>    m_Sounds;

      std::vector<tSoundInfo*>        m_Channels;

      bool                    m_Initialised;

      GR::u32                 m_FXMasterVolume;
      GR::u32                 m_MusicMasterVolume;



      GR::u32                 GetFreeID();
      bool                    CreateSoundVoice( tSoundInfo& SoundInfo );
      void                    SetupVoiceBuffer( const tSoundInfo& SoundInfo, tX2AudioSound& SourceVoice, GR::u32 Offset, GR::u32 Volume, GR::i32 Pan, GR::u32 Frequency, bool Looping );

      void                    ApplyPan( const tSoundInfo& SoundInfo, tX2AudioSound& SourceVoice, GR::i32 Pan );
      void                    ApplyVolume( const tSoundInfo& SoundInfo, tX2AudioSound& SourceVoice, GR::u32 Volume );



    public:


      XAudio2Sound();
      virtual ~XAudio2Sound();

      virtual bool            Initialize( GR::IEnvironment& Environment, GR::u32 Channels = 16 );
      virtual bool            Release();

      virtual bool            IsInitialized();

      virtual bool            SetMasterVolume( GR::u32 ucVolume, const eSoundType sType = ST_SOUND_EFFECT );
      virtual GR::u32         MasterVolume( const eSoundType sType = ST_SOUND_EFFECT );

      virtual GR::u32         LoadWave( const char* Name, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
      virtual GR::u32         LoadWave( IIOStream& Stream, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
      virtual GR::u32         LoadWave( const GR::String& Name, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
      virtual GR::u32         LoadWaveFromResource( const char* Resource, const char* ResourceType, const eSoundType Type = ST_SOUND_EFFECT );
      virtual GR::u32         CreateBufferFromMemory( int Channels, int SamplesPerSec, int BlockAlign,
                                                      int BitsPerSample, GR::up* pData, GR::up Size,
                                                      bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
      virtual GR::u32         CreateModifyableBuffer( int Channels, int SamplesPerSec, int BlockAlign,
                                                      int BitsPerSample, GR::up Size,
                                                      bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
      virtual bool            LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                                    LPVOID* pBlock1, GR::u32* Block1Size,
                                                    LPVOID* pBlock2, GR::u32* Block2Size );
      virtual bool            UnlockModifyableBuffer( GR::u32 ID );

      virtual bool            ReleaseWave( GR::u32 ID );

      virtual GR::u32         FindFreeChannel();
      virtual bool            Play( GR::u32 ID, GR::u32 Offset  );
      virtual bool            PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset ,
                                           GR::u32 Volume = 100, GR::i32 Pan = 0, GR::u32 Frequency = 0, bool Looping = false );
      virtual bool            Loop( GR::u32 ID );
      virtual bool            LoopChannel( GR::u32 ID, GR::u32 Channel );
      virtual bool            PlayDetail( GR::u32 ID, GR::i32 Volume, GR::i32 Pan, GR::u32 Frequency, bool Looping );
      virtual bool            Stop( GR::u32 ID );
      virtual bool            StopChannel( GR::u32 Channel );
      virtual bool            StopAll();
      virtual bool            IsPlaying( GR::u32 ID );
      virtual bool            IsChannelPlaying( GR::u32 Channel );

      virtual void            ReserveChannel( GR::u32 Channel );
      virtual void            UnreserveChannel( GR::u32 Channel );

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

      virtual bool            SetChannelVolume( GR::u32 Channel, GR::u32 ucVolume );
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


      static XAudio2Sound&    Instance();

  };
}






#endif // XAUDIO2_SOUND_H
