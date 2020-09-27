#include "XNullMusic.h"



XNullMusic::~XNullMusic()
{
}


bool XNullMusic::Initialize( GR::IEnvironment& Environment )
{
  return true;
}



bool XNullMusic::Release()
{
  return true;
}



bool XNullMusic::IsInitialized()
{
  return true;
}



int XNullMusic::Volume()
{
  return 0;
}



bool XNullMusic::SetVolume( int ucVolume )
{
  return true;
}



bool XNullMusic::LoadMusic( const GR::Char* szName )
{
  return false;
}



bool XNullMusic::LoadMusic( IIOStream& Stream )
{
  return false;
}



bool XNullMusic::Play( bool bLooped )
{
  return true;
}



void XNullMusic::Stop()
{
}



bool XNullMusic::IsPlaying()
{
  return true;
}



bool XNullMusic::Resume()
{
  return true;
}



bool XNullMusic::Pause()
{
  return true;
}



