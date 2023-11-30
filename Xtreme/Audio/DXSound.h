#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED



#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include <dsound.h>

#include <Lang/ITask.h>

#include "XSoundBase.h"

#include <map>
#include <string>
#include <list>



struct IIOStream;
class EventQueue;

class DXSound : public XSoundBase
{
  protected:

    enum eFlags
    {
      FLAG_SOUND_POSITIONCHANGED = 1,
    };

    struct tSoundInfo
    {
      unsigned char           Volume;
      signed long             Pan;
      GR::u32                 Frequency,
                              OriginalFrequency,
                              Flags;
      GR::up                  Size;
      LPDIRECTSOUNDBUFFER     dsWaveBuffer;
      bool                    Looping,
                              Modifyable,
                              Locked;
      GR::u32                 SoundID;
      LPVOID                  pLockPointer1;
      LPVOID                  pLockPointer2;
      GR::u32                 LockSize1;
      GR::u32                 LockSize2;
      eSoundType              Type;

      typedef std::vector<DSBPOSITIONNOTIFY>    tNotifications;

      tNotifications          Notifications;


      tSoundInfo() :
        Volume( 100 ),
        Pan( 0 ),
        Size( 0 ),
        Frequency( 0 ),
        OriginalFrequency( 0 ),
        Flags( 0 ),
        dsWaveBuffer( 0 ),
        Looping( false ),
        Modifyable( false ),
        SoundID( 0 ),
        Locked( false ),
        Type( ST_INVALID )
      {
      }
    };


    bool                                  m_Initialized,
                                          m_SoundAlreadyBusy;

    LPDIRECTSOUND                         m_pDirectSound;

    HINSTANCE                             m_hinstDSoundHandle;

    GR::u32                               m_FXMasterVolume;
    GR::u32                               m_MusicMasterVolume;

    LPDIRECTSOUNDBUFFER                   m_p3dPrimaryBuffer;

    LPDIRECTSOUNDBUFFER                   m_pPrimaryBuffer;




  private:

    typedef std::vector<tSoundInfo>       tChannels;


    tChannels                             m_Channels;

    EventQueue*                           m_pEventQueue;

    DXSound& operator=( const DXSound& rhs );
    DXSound( const DXSound& rhs );
    DXSound();



  public:

    std::map<GR::u32,tSoundInfo*>             m_Sounds;

    std::map<GR::u32,std::list<tSoundInfo*> > m_Duplicates;

    std::set<GR::u32>                         m_ReservedChannels;

    GR::u32                                   m_ETPlaySound;
    GR::u32                                   m_ETLoopSound;
    GR::u32                                   m_ETStopSound;
    GR::u32                                   m_ETStopAll;
    GR::u32                                   m_ETSetMasterVolume;


    virtual ~DXSound();


    static DXSound&         Instance();


    bool                    Initialize( GR::IEnvironment& Environment, GR::u32 Channels = 16 );
    bool                    Release();
    bool                    IsInitialized( void );

    bool                    SetMasterVolume( GR::u32 Volume, const eSoundType Type = ST_SOUND_EFFECT );
    GR::u32                 MasterVolume( const eSoundType sType = ST_SOUND_EFFECT );

    GR::u32                 LoadWave( const char* Name, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
    GR::u32                 LoadWave( IIOStream& Stream, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
    GR::u32                 LoadWave( const GR::String& Name, bool LoadAs3d = false, const eSoundType Type = ST_SOUND_EFFECT );
    GR::u32                 LoadWaveFromResource( const char* Resource, const char* ResourceType, const eSoundType Type = ST_SOUND_EFFECT );
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
    virtual bool            AddNotification( GR::u32 ID, GR::u32 Position, HANDLE hEvent );
    virtual bool            RemoveNotification( GR::u32 ID, GR::u32 Position, HANDLE hEvent );

    bool                    ReleaseWave( GR::u32 ID );
    bool                    UnloadWave( GR::u32 ID );
    GR::u32                 GetFreeID();

    GR::u32                 FindFreeChannel();
    virtual void            ReserveChannel( GR::u32 Channel );
    virtual void            UnreserveChannel( GR::u32 Channel );

    bool                    PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset = 0,
                                 GR::u32 Volume = 100, GR::i32 Pan = 0, GR::u32 Frequency = 0, bool Looping = false );
    bool                    Play( GR::u32 ID, GR::u32 Offset = 0 );
    bool                    StartDuplicate( GR::u32 ID );

    bool                    Loop( GR::u32 ID );
    virtual bool            LoopChannel( GR::u32 ID, GR::u32 Channel );

    bool                    PlayDetail( GR::u32 ID, GR::i32 Volume, GR::i32 Pan, GR::u32 Frequency, bool Looping );

    bool                    Stop( GR::u32 ID );
    virtual bool            StopChannel( GR::u32 Channel );
    bool                    StopAll( void );
    virtual bool            IsPlaying( GR::u32 ID );
    virtual bool            IsChannelPlaying( GR::u32 ID );

    bool                    Resume( GR::u32 ID );
    bool                    Pause( GR::u32 ID );

    bool                    SetFrequency( GR::u32 ID, GR::u32 Frequency );
    GR::u32                 GetOriginalFrequency( GR::u32 ID );
    GR::u32                 GetCurrentFrequency( GR::u32 ID );
    bool                    SetPos( GR::u32 ID, GR::u32 Pos );
    GR::u32                 GetPos( GR::u32 ID );
    bool                    SetPan( GR::u32 ID, signed long Pan );
    bool                    SetVolume( GR::u32 ID, GR::u32 Volume );
    GR::u32                 GetVolume( GR::u32 ID );

    virtual bool            SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency );

    virtual GR::u32         GetChannelOriginalFrequency( GR::u32 Channel );
    virtual GR::u32         GetChannelCurrentFrequency( GR::u32 Channel );

    virtual bool            SetChannelPos( GR::u32 Channel, GR::u32 Pos );
    virtual GR::u32         GetChannelPos( GR::u32 Channel );

    virtual bool            SetChannelPan( GR::u32 Channel, signed long Pan );

    virtual bool            SetChannelVolume( GR::u32 Channel, GR::u32 Volume );
    virtual GR::u32         GetChannelVolume( GR::u32 Channel );


    bool                    Create3dListener();
    bool                    Set3dListenerPosition( float X, float Y, float Z );
    bool                    Set3dListenerOrientation( float FrontX, float FrontY, float FrontZ,
                                              float UpX, float UpY, float UpZ );
    bool                    Set3dListenerParams( float X, float Y, float Z,
                                         float FrontX, float FrontY, float FrontZ,
                                         float UpX, float UpY, float UpZ,
                                         float VelocityX, float VelocityY, float VelocityZ );
    bool                    Set3dDistanceFactor( float Distance );
    bool                    Set3dDopplerFactor( float Doppler );
    bool                    Set3dRollOffFactor( float RollOff );

    bool                    Play3d( GR::u32 ID, float X, float Y, float Z, bool Looping = false );
    bool                    Set3dPosition( GR::u32 ID, float X, float Y, float Z );

    virtual void            ProcessEvent( const tGlobalEvent& Event );

    LPDIRECTSOUND           GetDevice()
    {
      return m_pDirectSound;
    }

};


#endif // SOUND_H_INCLUDED