#ifndef ANDROID_SOUND_H
#define ANDROID_SOUND_H



#include <Xtreme/XSound.h>



class AndroidSound : public XSound
{

  protected:

  public:

    virtual ~AndroidSound();


    virtual bool            Initialize( GR::IEnvironment& Environment, GR::u32 dwChannels = 16  );
    virtual bool            Release();

    virtual void            ReleaseAssets();

    virtual bool            IsInitialized();

    virtual bool            SetMasterVolume( GR::u32 ucVolume, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         MasterVolume( const eSoundType sType = ST_SOUND_EFFECT );

    virtual GR::u32         LoadWave( const char* szName, bool bLoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         LoadWave( const GR::String& Name, bool LoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         LoadWave( IIOStream& Stream, bool bLoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         LoadWaveFromResource( const char* szResource, const char* szType, const eSoundType sType = ST_SOUND_EFFECT );

    virtual GR::u32         CreateBufferFromMemory( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                                    int iBitsPerSample, GR::up* pData, GR::up dwSize,
                                                    bool bLoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual GR::u32         CreateModifyableBuffer( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                                    int iBitsPerSample, GR::up dwSize,
                                                    bool bLoadAs3d = false, const eSoundType sType = ST_SOUND_EFFECT );
    virtual bool            LockModifyableBuffer( GR::u32 dwID, GR::u32 dwStart, GR::u32 dwLockSize,
                                                  void** pBlock1, GR::u32* dwBlock1Size,
                                                  void** pBlock2, GR::u32* dwBlock2Size );
    virtual bool            UnlockModifyableBuffer( GR::u32 dwID );

    virtual bool            ReleaseWave( GR::u32 dwID );

    virtual bool            Play( GR::u32 dwID, GR::u32 dwOffset = 0 );
    virtual bool            Loop( GR::u32 dwID );
    virtual bool            PlayDetail( GR::u32 dwID, GR::i32 iVolume, GR::i32 iPan, GR::u32 iFrequency, bool bLooping );
    virtual bool            Stop( GR::u32 dwID );
    virtual bool            StopAll();
    virtual bool            IsPlaying( GR::u32 dwID );

    virtual GR::u32         FindFreeChannel();
    virtual void            ReserveChannel( GR::u32 dwChannel );
    virtual void            UnreserveChannel( GR::u32 dwChannel );
    virtual bool            PlayChannel( GR::u32 dwID, GR::u32 dwChannel, GR::u32 dwOffset = 0,
                                         GR::u32 dwVolume = 100, GR::i32 iPan = 0, GR::u32 dwFrequency = 0, bool bLooping = false );
    virtual bool            LoopChannel( GR::u32 dwID, GR::u32 dwChannel );
    virtual bool            StopChannel( GR::u32 dwChannel );
    virtual bool            IsChannelPlaying( GR::u32 dwChannel );

    virtual bool            Resume( GR::u32 dwID );
    virtual bool            Pause( GR::u32 dwID );

    virtual bool            SetFrequency( GR::u32 dwID, GR::u32 dwFrequency );

    virtual GR::u32         GetOriginalFrequency( GR::u32 dwID );
    virtual GR::u32         GetCurrentFrequency( GR::u32 dwID );

    virtual bool            SetPos( GR::u32 dwID, GR::u32 dwPos );
    virtual GR::u32         GetPos( GR::u32 dwID );

    virtual bool            SetPan( GR::u32 dwID, signed long slPan );

    virtual bool            SetVolume( GR::u32 dwID, GR::u32 ucVolume );
    virtual GR::u32         GetVolume( GR::u32 dwID );

    virtual bool            SetChannelFrequency( GR::u32 dwChannel, GR::u32 dwFrequency );

    virtual GR::u32         GetChannelOriginalFrequency( GR::u32 dwChannel );
    virtual GR::u32         GetChannelCurrentFrequency( GR::u32 dwChannel );

    virtual bool            SetChannelPos( GR::u32 dwChannel, GR::u32 dwPos );
    virtual GR::u32         GetChannelPos( GR::u32 dwChannel );

    virtual bool            SetChannelPan( GR::u32 dwChannel, signed long slPan );

    virtual bool            SetChannelVolume( GR::u32 dwChannel, GR::u32 ucVolume );
    virtual GR::u32         GetChannelVolume( GR::u32 dwChannel );

    virtual bool            Create3dListener();
    virtual bool            Set3dListenerPosition( float fX, float fY, float fZ );
    virtual bool            Set3dListenerOrientation( float fFrontX, float fFrontY, float fFrontZ,
                                                      float fUpX, float fUpY, float fUpZ );
    virtual bool            Set3dListenerParams( float fX, float fY, float fZ,
                                                 float fFrontX, float fFrontY, float fFrontZ,
                                                 float fUpX, float fUpY, float fUpZ,
                                                 float fVelocityX, float fVelocityY, float fVelocityZ );
    virtual bool            Set3dDistanceFactor( float fDistance );
    virtual bool            Set3dDopplerFactor( float fDoppler );
    virtual bool            Set3dRollOffFactor( float fRollOff );

    virtual bool            Play3d( GR::u32 dwID, float fX, float fY, float fZ, bool bLooping = false );
    virtual bool            Set3dPosition( GR::u32 dwID, float fX, float fY, float fZ );

    virtual void            ProcessEvent( const tGlobalEvent& Event );

};


#endif //ANDROID_SOUND_H