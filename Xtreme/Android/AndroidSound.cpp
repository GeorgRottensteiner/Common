#include "AndroidSound.h"



AndroidSound::~AndroidSound()
{
}



bool AndroidSound::Initialize( GR::IEnvironment& Environment, GR::u32 dwChannels )
{
  return true;
}



bool AndroidSound::Release()
{

  return true;

}



bool AndroidSound::IsInitialized()
{

  return true;

}



bool AndroidSound::SetMasterVolume( GR::u32 ucVolume, const eSoundType sType )
{

  return true;

}



GR::u32 AndroidSound::LoadWave( const GR::String& Name, bool LoadAs3d, const eSoundType sType )
{
  return 0;
}



GR::u32 AndroidSound::LoadWave( const char* szName, bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}



GR::u32 AndroidSound::LoadWave( IIOStream& Stream, bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}



GR::u32 AndroidSound::LoadWaveFromResource( const char* szResource, const char* szType, const eSoundType sType )
{
  return 0;
}



bool AndroidSound::ReleaseWave( GR::u32 dwID )
{
  return true;
}



bool AndroidSound::Play( GR::u32 dwID, GR::u32 dwOffset )
{
  return true;
}



bool AndroidSound::Loop( GR::u32 dwID )
{

  return true;

}



bool AndroidSound::PlayDetail( GR::u32 dwID, GR::i32 iVolume, GR::i32 iPan, GR::u32 iFrequency, bool bLooping )
{

  return true;

}



bool AndroidSound::Stop( GR::u32 dwID )
{

  return true;

}



bool AndroidSound::StopAll()
{

  return true;

}



bool AndroidSound::IsPlaying( GR::u32 dwID )
{

  return true;

}



bool AndroidSound::Resume( GR::u32 dwID )
{

  return true;

}



bool AndroidSound::Pause( GR::u32 dwID )
{

  return true;

}



bool AndroidSound::SetFrequency( GR::u32 dwID, GR::u32 dwFrequency )
{

  return true;

}



GR::u32 AndroidSound::GetOriginalFrequency( GR::u32 dwID )
{

  return true;

}



GR::u32 AndroidSound::GetCurrentFrequency( GR::u32 dwID )
{

  return true;

}



bool AndroidSound::SetPos( GR::u32 dwID, GR::u32 dwPos )
{

  return true;

}



GR::u32 AndroidSound::GetPos( GR::u32 dwID )
{

  return 0;

}




bool AndroidSound::SetPan( GR::u32 dwID, signed long slPan )
{

  return true;

}



bool AndroidSound::SetVolume( GR::u32 dwID, GR::u32 ucVolume )
{

  return true;

}



GR::u32 AndroidSound::GetVolume( GR::u32 dwID )
{

  return 0;

}



bool AndroidSound::Create3dListener()
{

  return true;

}



bool AndroidSound::Set3dListenerPosition( float fX, float fY, float fZ )
{

  return true;

}



bool AndroidSound::Set3dListenerOrientation( float fFrontX, float fFrontY, float fFrontZ,
                                          float fUpX, float fUpY, float fUpZ )
{

  return true;

}



bool AndroidSound::Set3dListenerParams( float fX, float fY, float fZ,
                                      float fFrontX, float fFrontY, float fFrontZ,
                                      float fUpX, float fUpY, float fUpZ,
                                      float fVelocityX, float fVelocityY, float fVelocityZ )
{

  return true;

}



bool AndroidSound::Set3dDistanceFactor( float fDistance )
{

  return true;

}



bool AndroidSound::Set3dDopplerFactor( float fDoppler )
{

  return true;

}



bool AndroidSound::Set3dRollOffFactor( float fRollOff )
{

  return true;

}




bool AndroidSound::Play3d( GR::u32 dwID, float fX, float fY, float fZ, bool bLooping )
{

  return true;

}



bool AndroidSound::Set3dPosition( GR::u32 dwID, float fX, float fY, float fZ )
{

  return true;

}




void AndroidSound::ProcessEvent( const tGlobalEvent& Event )
{

}



GR::u32 AndroidSound::CreateBufferFromMemory( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                            int iBitsPerSample, GR::up* pData, GR::up dwSize,
                                            bool bLoadAs3d, const eSoundType sType )
{

  return 0;

}




GR::u32 AndroidSound::FindFreeChannel()
{

  return 0;

}



void AndroidSound::ReserveChannel( GR::u32 dwChannel )
{
}



void AndroidSound::UnreserveChannel( GR::u32 dwChannel )
{
}



bool AndroidSound::PlayChannel( GR::u32 dwID, GR::u32 dwChannel, GR::u32 dwOffset,
                              GR::u32 dwVolume, GR::i32 iPan, GR::u32 dwFrequency, bool bLooping )
{

  return true;

}



bool AndroidSound::LoopChannel( GR::u32 dwID, GR::u32 dwChannel )
{

  return true;

}



bool AndroidSound::StopChannel( GR::u32 dwChannel )
{

  return true;

}



bool AndroidSound::IsChannelPlaying( GR::u32 dwChannel )
{

  return false;

}



bool AndroidSound::SetChannelFrequency( GR::u32 dwChannel, GR::u32 dwFrequency )
{

  return true;

}



GR::u32 AndroidSound::GetChannelOriginalFrequency( GR::u32 dwChannel )
{

  return 0;

}



GR::u32 AndroidSound::GetChannelCurrentFrequency( GR::u32 dwChannel )
{

  return 0;

}



bool AndroidSound::SetChannelPos( GR::u32 dwChannel, GR::u32 dwPos )
{

  return true;

}



GR::u32 AndroidSound::GetChannelPos( GR::u32 dwChannel )
{

  return 0;

}



bool AndroidSound::SetChannelPan( GR::u32 dwChannel, signed long slPan )
{

  return true;

}



bool AndroidSound::SetChannelVolume( GR::u32 dwChannel, GR::u32 ucVolume )
{

  return true;

}



GR::u32 AndroidSound::GetChannelVolume( GR::u32 dwChannel )
{

  return 0;

}



GR::u32 AndroidSound::CreateModifyableBuffer( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                            int iBitsPerSample, GR::up dwSize,
                                            bool bLoadAs3d, const eSoundType sType )
{

  return 0;

}



bool AndroidSound::LockModifyableBuffer( GR::u32 dwID, GR::u32 dwStart, GR::u32 dwLockSize,
                                       void** pBlock1, GR::u32* dwBlock1Size,
                                       void** pBlock2, GR::u32* dwBlock2Size )
{
  *pBlock1 = NULL;
  *dwBlock1Size = 0;
  *pBlock2 = NULL;
  *dwBlock2Size = 0;
  return true;
}



bool AndroidSound::UnlockModifyableBuffer( GR::u32 dwID )
{
  return true;
}



GR::u32 AndroidSound::MasterVolume( const eSoundType sType )
{
  return 0;
}



void AndroidSound::ReleaseAssets()
{
}