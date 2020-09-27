#include "SDLSound.h"

#ifdef EMSCRIPTEN
#include <SDL2/SDL_mixer.h>
#else
#include "SDL_mixer.h"
#endif

#include <debug/debugclient.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetSound.h>

#include <Interface/IDebugService.h>
#include <Interface/IValueStorage.h>



/* set this to any of 512,1024,2048,4096              */
/* the lower it is, the more FPS shown and CPU needed */
#define BUFFER 1024
#define W 640 /* NEVER make this be less than BUFFER! */
#define H 480
#define H2 (H/2)
#define H4 (H/4)
#define Y(sample) (((sample)*H)/4/0x7fff)



SDLSound::SDLSound() :
  m_pDebugger( NULL ),
  m_NumChannels( 8 ),

  m_pMusic( NULL )
{
}



SDLSound::~SDLSound()
{
}



bool SDLSound::Initialize( GR::IEnvironment& Environment, GR::u32 Channels )
{
  m_pDebugger = (IDebugService*)Environment.Service( "Logger" );

  if ( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, BUFFER ) < 0 )
  {
    dh::Log( "Mix_OpenAudio failed" );
    return false;
  }
  if ( Mix_AllocateChannels( Channels ) != Channels )
  {
    dh::Log( "Mix_AllocateChannels failed to allocate %d channels", Channels );
  }
  else
  {
    m_NumChannels = Channels;

    m_Channels.resize( m_NumChannels );
  }

  LoadAssets();

  return true;
}



bool SDLSound::Release()
{
  StopAll();

  ReleaseAssets();

  Mix_CloseAudio();
  return true;
}



bool SDLSound::IsInitialized()
{
  return true;
}



bool SDLSound::SetMasterVolume( GR::u32 ucVolume, const eSoundType sType )
{
  return true;
}



GR::u32 SDLSound::LoadWave( const GR::String& Name, bool LoadAs3d, const eSoundType sType )
{
  Mix_Chunk* pSound = Mix_LoadWAV( Name.c_str() );
  if ( pSound != NULL )
  {
    m_Sounds.push_back( pSound );

    return ( GR::u32 )m_Sounds.size();
  }
  return 0;
}



GR::u32 SDLSound::LoadWave( const char* szName, bool bLoadAs3d, const eSoundType sType )
{
  Mix_Chunk* pSound = Mix_LoadWAV( szName );
  if ( pSound != NULL )
  {
    m_Sounds.push_back( pSound );

    return (GR::u32)m_Sounds.size();
  }
  return 0;
}



GR::u32 SDLSound::LoadWave( IIOStream& Stream, bool bLoadAs3d, const eSoundType sType )
{
  ByteBuffer    waveFile( (size_t)Stream.GetSize() );

  Stream.ReadBlock( waveFile.Data(), waveFile.Size() );

  Mix_Chunk* pSound = Mix_LoadWAV_RW( SDL_RWFromConstMem( waveFile.Data(), waveFile.Size() ), 1 );
  if ( pSound != NULL )
  {
    m_Sounds.push_back( pSound );

    return ( GR::u32 )m_Sounds.size();
  }
  return 0;
}



GR::u32 SDLSound::LoadWaveFromResource( const char* szResource, const char* szType, const eSoundType sType )
{
  return 0;
}



bool SDLSound::ReleaseWave( GR::u32 ID )
{
  return true;
}



bool SDLSound::Play( GR::u32 ID, GR::u32 Offset )
{
  int channelUsed = Mix_PlayChannel( -1, m_Sounds[ID - 1], 0 );
  if ( channelUsed != -1 )
  {
    m_Channels[channelUsed].SoundID = ID;
    
    // TODO?
    m_Channels[channelUsed].Volume  = 100;
  }
  return channelUsed != -1;
}



bool SDLSound::Loop( GR::u32 ID )
{
  int channelUsed = Mix_PlayChannel( -1, m_Sounds[ID - 1], -1 );

  if ( channelUsed != -1 )
  {
    m_Channels[channelUsed].SoundID = ID;

    // TODO?
    m_Channels[channelUsed].Volume = 100;
  }
  return channelUsed != -1;
}



bool SDLSound::PlayDetail( GR::u32 ID, GR::i32 iVolume, GR::i32 iPan, GR::u32 iFrequency, bool bLooping )
{
  int channelUsed = Mix_PlayChannel( -1, m_Sounds[ID - 1], 0 );

  if ( channelUsed != -1 )
  {
    m_Channels[channelUsed].SoundID = ID;

    // TODO - Master Volume?
    m_Channels[channelUsed].Volume = iVolume * 128 / 100;

    Mix_Volume( channelUsed, iVolume * 128 / 100 );
  }
  return channelUsed != -1;
}



bool SDLSound::Stop( GR::u32 ID )
{
  if ( (GR::u32)ID >= m_Sounds.size() )
  {
    dh::Log( "Stop invalid sound %ld", ID );
    return false;
  }
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_Chunk* pChunk = Mix_GetChunk( i );
    if ( pChunk == m_Sounds[ID] )
    {
      Mix_HaltChannel( i );
      m_Channels[i].SoundID = -1;
    }
  }
  return true;
}



bool SDLSound::StopAll()
{
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_HaltChannel( i );
    m_Channels[i].SoundID = -1;
  }
  return true;
}



bool SDLSound::IsPlaying( GR::u32 ID )
{
  if ( ( GR::u32 )ID >= m_Sounds.size() )
  {
    dh::Log( "Stop invalid sound %ld", ID );
    return false;
  }
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_Chunk* pChunk = Mix_GetChunk( i );
    if ( pChunk == m_Sounds[ID] )
    {
      if ( Mix_Playing( i ) != 0 )
      {
        return true;
      }
      m_Channels[i].SoundID = -1;
    }
  }
  return false;
}



bool SDLSound::Resume( GR::u32 ID )
{
  if ( ( GR::u32 )ID >= m_Sounds.size() )
  {
    dh::Log( "Resume invalid sound %ld", ID );
    return false;
  }
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_Chunk* pChunk = Mix_GetChunk( i );
    if ( pChunk == m_Sounds[ID] )
    {
      Mix_Resume( i );
    }
  }
  return true;
}



bool SDLSound::Pause( GR::u32 ID )
{
  if ( ( GR::u32 )ID >= m_Sounds.size() )
  {
    dh::Log( "Pause invalid sound %ld", ID );
    return false;
  }
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_Chunk* pChunk = Mix_GetChunk( i );
    if ( pChunk == m_Sounds[ID] )
    {
      Mix_Pause( i );
    }
  }
  return true;
}



bool SDLSound::SetFrequency( GR::u32 ID, GR::u32 Frequency )
{
  return true;
}



GR::u32 SDLSound::GetOriginalFrequency( GR::u32 ID )
{
  return true;
}



GR::u32 SDLSound::GetCurrentFrequency( GR::u32 ID )
{
  return true;
}



bool SDLSound::SetPos( GR::u32 ID, GR::u32 Pos )
{
  return true;
}



GR::u32 SDLSound::GetPos( GR::u32 ID )
{
  return 0;
}




bool SDLSound::SetPan( GR::u32 ID, signed long slPan )
{
  if ( ( GR::u32 )ID >= m_Sounds.size() )
  {
    dh::Log( "SetPan invalid sound %ld", ID );
    return false;
  }
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_Chunk* pChunk = Mix_GetChunk( i );
    if ( pChunk == m_Sounds[ID] )
    {
      Mix_SetPanning( i, (GR::u8)( ( slPan * 255 ) / 100) , (GR::u8)( 255 - ( slPan * 255 ) / 100 ) );
    }
  }
  return true;
}



bool SDLSound::SetVolume( GR::u32 ID, GR::u32 ucVolume )
{
  if ( ( GR::u32 )ID >= m_Sounds.size() )
  {
    dh::Log( "SetVolume invalid sound %ld", ID );
    return false;
  }
  for ( int i = 0; i < m_NumChannels; ++i )
  {
    Mix_Chunk* pChunk = Mix_GetChunk( i );
    if ( pChunk == m_Sounds[ID] )
    {
      Mix_Volume( i, ucVolume * 128 / 100 );
    }
  }
  return true;
}



GR::u32 SDLSound::GetVolume( GR::u32 ID )
{
  return 0;
}



bool SDLSound::Create3dListener()
{
  return true;
}



bool SDLSound::Set3dListenerPosition( float fX, float fY, float fZ )
{
  return true;
}



bool SDLSound::Set3dListenerOrientation( float fFrontX, float fFrontY, float fFrontZ,
                                          float fUpX, float fUpY, float fUpZ )
{
  return true;
}



bool SDLSound::Set3dListenerParams( float fX, float fY, float fZ,
                                      float fFrontX, float fFrontY, float fFrontZ,
                                      float fUpX, float fUpY, float fUpZ,
                                      float fVelocityX, float fVelocityY, float fVelocityZ )
{
  return true;
}



bool SDLSound::Set3dDistanceFactor( float fDistance )
{
  return true;
}



bool SDLSound::Set3dDopplerFactor( float fDoppler )
{
  return true;
}



bool SDLSound::Set3dRollOffFactor( float fRollOff )
{
  return true;
}




bool SDLSound::Play3d( GR::u32 ID, float fX, float fY, float fZ, bool bLooping )
{
  return true;
}



bool SDLSound::Set3dPosition( GR::u32 ID, float fX, float fY, float fZ )
{
  return true;
}




void SDLSound::ProcessEvent( const tGlobalEvent& Event )
{
}



GR::u32 SDLSound::CreateBufferFromMemory( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                            int iBitsPerSample, GR::up* pData, GR::up dwSize,
                                            bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}




GR::u32 SDLSound::FindFreeChannel()
{
  return 0;
}



void SDLSound::ReserveChannel( GR::u32 Channel )
{
}



void SDLSound::UnreserveChannel( GR::u32 Channel )
{
}



bool SDLSound::PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset,
                              GR::u32 Volume, GR::i32 iPan, GR::u32 Frequency, bool bLooping )
{
  return true;
}



bool SDLSound::LoopChannel( GR::u32 ID, GR::u32 Channel )
{
  return true;
}



bool SDLSound::StopChannel( GR::u32 Channel )
{
  return true;
}



bool SDLSound::IsChannelPlaying( GR::u32 Channel )
{
  return false;
}



bool SDLSound::SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency )
{
  return true;
}



GR::u32 SDLSound::GetChannelOriginalFrequency( GR::u32 Channel )
{
  return 0;
}



GR::u32 SDLSound::GetChannelCurrentFrequency( GR::u32 Channel )
{
  return 0;
}



bool SDLSound::SetChannelPos( GR::u32 Channel, GR::u32 Pos )
{
  return true;
}



GR::u32 SDLSound::GetChannelPos( GR::u32 Channel )
{
  return 0;
}



bool SDLSound::SetChannelPan( GR::u32 Channel, signed long slPan )
{
  return true;
}



bool SDLSound::SetChannelVolume( GR::u32 Channel, GR::u32 ucVolume )
{
  return true;
}



GR::u32 SDLSound::GetChannelVolume( GR::u32 Channel )
{
  return 0;
}



GR::u32 SDLSound::CreateModifyableBuffer( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                            int iBitsPerSample, GR::up dwSize,
                                            bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}



bool SDLSound::LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                       void** pBlock1, GR::u32* pBlock1Size,
                                       void** pBlock2, GR::u32* pBlock2Size )
{
  *pBlock1 = NULL;
  *pBlock1Size = 0;
  *pBlock2 = NULL;
  *pBlock2Size = 0;
  return true;
}



bool SDLSound::UnlockModifyableBuffer( GR::u32 ID )
{
  return true;
}



GR::u32 SDLSound::MasterVolume( const eSoundType sType )
{
  return 0;
}



void SDLSound::ReleaseAssets()
{
}



void SDLSound::Log( const GR::String& strSystem, const char* szFormat, ... )
{
  if ( m_pDebugger )
  {
    static char    szMiscBuffer[5000];
    vsprintf( szMiscBuffer, szFormat, (char*)( &szFormat + 1 ) );

    m_pDebugger->LogDirect( strSystem.c_str(), szMiscBuffer );
  }
}



GR::String SDLSound::AppPath( const GR::String& Path )
{
  if ( XSoundBase::m_pEnvironment == NULL )
  {
    dh::Log( "CSoundSystem::AppPath no environment" );
    return CMisc::AppPath( Path.c_str() );
  }
  GR::Gamebase::IValueStorage* pStorage = ( GR::Gamebase::IValueStorage* )XSoundBase::m_pEnvironment->Service( "ValueStorage" );
  if ( pStorage == NULL )
  {
    dh::Log( "CSoundSystem::AppPath no storage" );
    return CMisc::AppPath( Path.c_str() );
  }
  return pStorage->AppPath( Path.c_str() );
}



bool SDLSound::SetVolume( int Volume )
{
  Mix_VolumeMusic( Volume * 128 / 100 );
  return true;
}



int SDLSound::Volume()
{
  return ( Mix_VolumeMusic( -1 ) * 100 ) / 128;
}



bool SDLSound::Play( bool Looped )
{
  if ( m_pMusic == NULL )
  {
    dh::Log( "SDLSound::Play no music loaded" );
    return false;
  }
  return !Mix_PlayMusic( m_pMusic, Looped ? -1 : 1 );
}



void SDLSound::Stop()
{
  Mix_HaltMusic();
}



bool SDLSound::IsPlaying()
{
  return !!Mix_PlayingMusic();
}



bool SDLSound::Resume()
{
  Mix_ResumeMusic();
  return true;
}



bool SDLSound::Pause()
{
  Mix_PauseMusic();
  return true;
}



bool SDLSound::LoadMusic( const GR::Char* FileName )
{
  if ( m_pMusic != NULL )
  {
    Stop();
    Mix_FreeMusic( m_pMusic );
    m_pMusic = NULL;
  }

  m_pMusic = Mix_LoadMUS( FileName );
  if ( m_pMusic == NULL )
  {
    dh::Log( "SDLSound::LoadMusic Mix_LoadMUS failed (%s)", Mix_GetError() );
  }
  return m_pMusic != NULL;
}



bool SDLSound::LoadMusic( IIOStream& Stream )
{
  if ( m_pMusic != NULL )
  {
    Stop();
    Mix_FreeMusic( m_pMusic );
    m_pMusic = NULL;
  }
  dh::Log( "SDLSound::LoadMusic from stream not supported" );
  return false;
}



bool SDLSound::Initialize( GR::IEnvironment& Environment )
{
  // all handled via sound player
  return true;
}