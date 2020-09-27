#include "XNullSound.h"



XNullSound::~XNullSound()
{
}



bool XNullSound::Initialize( GR::IEnvironment& Environment, GR::u32 Channels )
{
  return true;
}



bool XNullSound::Release()
{
  return true;
}



bool XNullSound::IsInitialized()
{
  return true;
}



bool XNullSound::SetMasterVolume( GR::u32 ucVolume, const eSoundType sType )
{
  return true;
}



GR::u32 XNullSound::LoadWave( const GR::String& Name, bool LoadAs3d, const eSoundType sType )
{
  return 0;
}



GR::u32 XNullSound::LoadWave( const char* szName, bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}



GR::u32 XNullSound::LoadWave( IIOStream& Stream, bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}



GR::u32 XNullSound::LoadWaveFromResource( const char* szResource, const char* szType, const eSoundType sType )
{
  return 0;
}



bool XNullSound::ReleaseWave( GR::u32 ID )
{
  return true;
}



bool XNullSound::Play( GR::u32 ID, GR::u32 Offset )
{
  return true;
}



bool XNullSound::Loop( GR::u32 ID )
{
  return true;
}



bool XNullSound::PlayDetail( GR::u32 ID, GR::i32 iVolume, GR::i32 iPan, GR::u32 iFrequency, bool bLooping )
{
  return true;
}



bool XNullSound::Stop( GR::u32 ID )
{
  return true;
}



bool XNullSound::StopAll()
{
  return true;
}



bool XNullSound::IsPlaying( GR::u32 ID )
{
  return true;
}



bool XNullSound::Resume( GR::u32 ID )
{
  return true;
}



bool XNullSound::Pause( GR::u32 ID )
{
  return true;
}



bool XNullSound::SetFrequency( GR::u32 ID, GR::u32 Frequency )
{
  return true;
}



GR::u32 XNullSound::GetOriginalFrequency( GR::u32 ID )
{
  return true;
}



GR::u32 XNullSound::GetCurrentFrequency( GR::u32 ID )
{
  return true;
}



bool XNullSound::SetPos( GR::u32 ID, GR::u32 Pos )
{
  return true;
}



GR::u32 XNullSound::GetPos( GR::u32 ID )
{
  return 0;
}




bool XNullSound::SetPan( GR::u32 ID, signed long slPan )
{
  return true;
}



bool XNullSound::SetVolume( GR::u32 ID, GR::u32 ucVolume )
{
  return true;
}



GR::u32 XNullSound::GetVolume( GR::u32 ID )
{
  return 0;
}



bool XNullSound::Create3dListener()
{
  return true;
}



bool XNullSound::Set3dListenerPosition( float fX, float fY, float fZ )
{
  return true;
}



bool XNullSound::Set3dListenerOrientation( float fFrontX, float fFrontY, float fFrontZ,
                                          float fUpX, float fUpY, float fUpZ )
{
  return true;
}



bool XNullSound::Set3dListenerParams( float fX, float fY, float fZ,
                                      float fFrontX, float fFrontY, float fFrontZ,
                                      float fUpX, float fUpY, float fUpZ,
                                      float fVelocityX, float fVelocityY, float fVelocityZ )
{
  return true;
}



bool XNullSound::Set3dDistanceFactor( float fDistance )
{
  return true;
}



bool XNullSound::Set3dDopplerFactor( float fDoppler )
{
  return true;
}



bool XNullSound::Set3dRollOffFactor( float fRollOff )
{
  return true;
}




bool XNullSound::Play3d( GR::u32 ID, float fX, float fY, float fZ, bool bLooping )
{
  return true;
}



bool XNullSound::Set3dPosition( GR::u32 ID, float fX, float fY, float fZ )
{
  return true;
}




void XNullSound::ProcessEvent( const tGlobalEvent& Event )
{
}



GR::u32 XNullSound::CreateBufferFromMemory( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                            int iBitsPerSample, GR::up* pData, GR::up dwSize,
                                            bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}




GR::u32 XNullSound::FindFreeChannel()
{
  return 0;
}



void XNullSound::ReserveChannel( GR::u32 Channel )
{
}



void XNullSound::UnreserveChannel( GR::u32 Channel )
{
}



bool XNullSound::PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset,
                              GR::u32 Volume, GR::i32 iPan, GR::u32 Frequency, bool bLooping )
{
  return true;
}



bool XNullSound::LoopChannel( GR::u32 ID, GR::u32 Channel )
{
  return true;
}



bool XNullSound::StopChannel( GR::u32 Channel )
{
  return true;
}



bool XNullSound::IsChannelPlaying( GR::u32 Channel )
{
  return false;
}



bool XNullSound::SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency )
{
  return true;
}



GR::u32 XNullSound::GetChannelOriginalFrequency( GR::u32 Channel )
{
  return 0;
}



GR::u32 XNullSound::GetChannelCurrentFrequency( GR::u32 Channel )
{
  return 0;
}



bool XNullSound::SetChannelPos( GR::u32 Channel, GR::u32 Pos )
{
  return true;
}



GR::u32 XNullSound::GetChannelPos( GR::u32 Channel )
{
  return 0;
}



bool XNullSound::SetChannelPan( GR::u32 Channel, signed long slPan )
{
  return true;
}



bool XNullSound::SetChannelVolume( GR::u32 Channel, GR::u32 ucVolume )
{
  return true;
}



GR::u32 XNullSound::GetChannelVolume( GR::u32 Channel )
{
  return 0;
}



GR::u32 XNullSound::CreateModifyableBuffer( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                            int iBitsPerSample, GR::up dwSize,
                                            bool bLoadAs3d, const eSoundType sType )
{
  return 0;
}



bool XNullSound::LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                       void** pBlock1, GR::u32* pBlock1Size,
                                       void** pBlock2, GR::u32* pBlock2Size )
{
  *pBlock1 = NULL;
  *pBlock1Size = 0;
  *pBlock2 = NULL;
  *pBlock2Size = 0;
  return true;
}



bool XNullSound::UnlockModifyableBuffer( GR::u32 ID )
{
  return true;
}



GR::u32 XNullSound::MasterVolume( const eSoundType sType )
{
  return 0;
}



void XNullSound::ReleaseAssets()
{
}