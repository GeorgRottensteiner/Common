#ifndef XSOUND_H_INCLUDED
#define XSOUND_H_INCLUDED



#include <GR/GRTypes.h>

#include <Lang/EventQueue.h>
#include <Lang/Service.h>



struct IIOStream;

class XSound : public IGlobalEventListener, 
               public GR::Service::Service
{

  public:

    enum eSoundType
    {
      ST_INVALID = 0,
      ST_SOUND_EFFECT,
      ST_MUSIC,
      ST_ALL,
    };

    virtual ~XSound()
    {
    }


    virtual bool            Initialize( GR::IEnvironment& Environment, GR::u32 NumChannels = 16 ) = 0;
    virtual bool            Release() = 0;

    virtual void            ReleaseAssets() = 0;

    virtual bool            IsInitialized() = 0;

    virtual bool            SetMasterVolume( GR::u32 Volume, const eSoundType Type = ST_SOUND_EFFECT ) = 0;
    virtual GR::u32         MasterVolume( const eSoundType Type = ST_SOUND_EFFECT ) = 0;

    virtual GR::u32         LoadWave( const char* Name, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT ) = 0;
    virtual GR::u32         LoadWave( const GR::String& Filename, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT ) = 0;
    virtual GR::u32         LoadWave( IIOStream& Stream, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT ) = 0;
    virtual GR::u32         LoadWaveFromResource( const char* Resource, const char* Type, const eSoundType SoundType = ST_SOUND_EFFECT ) = 0;
    virtual GR::u32         CreateBufferFromMemory( int Channels, int SamplesPerSec, int BlockAlign,
                                                    int BitsPerSample, GR::up* pData, GR::up Size,
                                                    bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT ) = 0;
    virtual GR::u32         CreateModifyableBuffer( int Channels, int SamplesPerSec, int BlockAlign,
                                                    int BitsPerSample, GR::up Size,
                                                    bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT ) = 0;
    virtual bool            LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                                  void** pBlock1, GR::u32* pBlock1Size,
                                                  void** pBlock2, GR::u32* pBlock2Size ) = 0;
    virtual bool            UnlockModifyableBuffer( GR::u32 ID ) = 0;

    virtual bool            ReleaseWave( GR::u32 ID ) = 0;

    virtual GR::u32         FindFreeChannel() = 0;
    virtual bool            Play( GR::u32 ID, GR::u32 Offset = 0 ) = 0;
    virtual bool            PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset = 0,
                                         GR::u32 Volume = 100, GR::i32 Pan = 0, GR::u32 Frequency = 0, bool Looping = false ) = 0;
    virtual bool            Loop( GR::u32 ID ) = 0;
    virtual bool            LoopChannel( GR::u32 ID, GR::u32 Channel ) = 0;
    virtual bool            PlayDetail( GR::u32 ID, GR::i32 Volume, GR::i32 Pan = 0, GR::u32 Frequency = 0, bool Looping = false ) = 0;
    virtual bool            Stop( GR::u32 ID ) = 0;
    virtual bool            StopChannel( GR::u32 Channel ) = 0;
    virtual bool            StopAll() = 0;
    virtual bool            IsPlaying( GR::u32 ID ) = 0;
    virtual bool            IsChannelPlaying( GR::u32 Channel ) = 0;

    virtual void            ReserveChannel( GR::u32 Channel ) = 0;
    virtual void            UnreserveChannel( GR::u32 Channel ) = 0;

    virtual bool            Resume( GR::u32 ID ) = 0;
    virtual bool            Pause( GR::u32 ID ) = 0;

    virtual bool            SetFrequency( GR::u32 ID, GR::u32 Frequency ) = 0;

    virtual GR::u32         GetOriginalFrequency( GR::u32 ID ) = 0;
    virtual GR::u32         GetCurrentFrequency( GR::u32 ID ) = 0;

    virtual bool            SetPos( GR::u32 ID, GR::u32 Pos ) = 0;
    virtual GR::u32         GetPos( GR::u32 ID ) = 0;

    virtual bool            SetPan( GR::u32 ID, signed long slPan ) = 0;

    virtual bool            SetVolume( GR::u32 ID, GR::u32 ucVolume ) = 0;
    virtual GR::u32         GetVolume( GR::u32 ID ) = 0;

    virtual bool            SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency ) = 0;

    virtual GR::u32         GetChannelOriginalFrequency( GR::u32 Channel ) = 0;
    virtual GR::u32         GetChannelCurrentFrequency( GR::u32 Channel ) = 0;

    virtual bool            SetChannelPos( GR::u32 Channel, GR::u32 Pos ) = 0;
    virtual GR::u32         GetChannelPos( GR::u32 Channel ) = 0;

    virtual bool            SetChannelPan( GR::u32 Channel, signed long Pan ) = 0;

    virtual bool            SetChannelVolume( GR::u32 Channel, GR::u32 Volume ) = 0;
    virtual GR::u32         GetChannelVolume( GR::u32 Channel ) = 0;

    virtual bool            Create3dListener() = 0;
    virtual bool            Set3dListenerPosition( float X, float Y, float Z ) = 0;
    virtual bool            Set3dListenerOrientation( float FrontX, float FrontY, float FrontZ,
                                                      float UpX, float UpY, float UpZ ) = 0;
    virtual bool            Set3dListenerParams( float X, float Y, float Z,
                                                 float FrontX, float FrontY, float FrontZ,
                                                 float UpX, float UpY, float UpZ,
                                                 float VelocityX, float VelocityY, float VelocityZ ) = 0;
    virtual bool            Set3dDistanceFactor( float Distance ) = 0;
    virtual bool            Set3dDopplerFactor( float Doppler ) = 0;
    virtual bool            Set3dRollOffFactor( float RollOff ) = 0;

    virtual bool            Play3d( GR::u32 ID, float X, float Y, float Z, bool Looping = false ) = 0;
    virtual bool            Set3dPosition( GR::u32 ID, float X, float Y, float Z ) = 0;

    virtual void            ProcessEvent( const tGlobalEvent& Event ) = 0;

};


#endif //XSOUND_H_INCLUDED__