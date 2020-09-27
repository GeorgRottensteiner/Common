#include "DXSound.h"
#include <IO/FileStream.h>

#include <Interface/IIOStream.h>
#include <Interface/IDebugService.h>
#include <Interface/IValueStorage.h>

#include <Misc/Misc.h>

#include <Xtreme/Environment/XWindow.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetSound.h>

#include <String/XML.h>

#include <debug\debugclient.h>






static HRESULT ( WINAPI *MyDirectSoundCreate )( HINSTANCE hinst, LPDIRECTSOUND * lplpDirectSound, LPUNKNOWN punkOuter );



DXSound::DXSound() :
  m_Initialized(       false ),
  m_SoundAlreadyBusy(  false ),
  m_pDirectSound(       NULL  ),
  m_hinstDSoundHandle(  NULL  ),
  m_FXMasterVolume(   100   ),
  m_MusicMasterVolume( 100 ),
  m_p3dPrimaryBuffer(   NULL ),
  m_pPrimaryBuffer(     NULL ),
  m_pEventQueue(        NULL )
{
}



DXSound::~DXSound()
{
  Release();
}



bool DXSound::Initialize( GR::IEnvironment& Environment, GR::u32 Channels )
{
  HRESULT     hResult;

  SetEnvironment( &Environment );

  if ( Channels == 0 )
  {
    Channels = 16;
  }
  m_Channels.clear();
  for ( GR::u32 i = 0; i < Channels; ++i )
  {
    m_Channels.push_back( tSoundInfo() );
  }

  m_Initialized       = false;
  m_SoundAlreadyBusy  = false;
  m_FXMasterVolume    = 100;
  m_MusicMasterVolume = 100;

  m_pEventQueue = (EventQueue*)Environment.Service( "GlobalQueue" );
  m_pDebugger = (IDebugService*)Environment.Service( "Logger" );

  Xtreme::IAppWindow* pWindowService = (Xtreme::IAppWindow*)Environment.Service( "Window" );
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }

  m_hinstDSoundHandle = LoadLibrary( "dsound.dll" );
  if ( m_hinstDSoundHandle == NULL )
  {
    // kein DSOUND installiert?
    Log( "Sound.General", CMisc::printf( "Could not open dsound.dll (%x)", GetLastError() ) );
    dh::Log( "kein DSOUND installiert?" );
    return false;
  }

  MyDirectSoundCreate = (HRESULT (__stdcall *)( HINSTANCE, LPDIRECTSOUND *, LPUNKNOWN ) )GetProcAddress( m_hinstDSoundHandle, "DirectSoundCreate" );

  if ( !MyDirectSoundCreate )
  {
    FreeLibrary( m_hinstDSoundHandle );

    Log( "Sound.General", "DirectSoundCreate not found in dsound.dll" );

    // DirectSoundCreate nicht in der DLL gefunden??
    return false;
  }

  hResult = MyDirectSoundCreate( NULL, &m_pDirectSound, NULL );
  if ( hResult == DSERR_NODRIVER )
  {
    // Kein Soundgerät, also tonlos weiter
    Log( "Sound.General", "DirectSoundCreate returned DSERR_NODRIVER" );

    m_Initialized = false;

    if ( m_pEventQueue )
    {
      m_ETPlaySound       = m_pEventQueue->RegisterEvent( "Sound.Play" );
      m_ETLoopSound       = m_pEventQueue->RegisterEvent( "Sound.Loop" );
      m_ETStopSound       = m_pEventQueue->RegisterEvent( "Sound.Stop" );
      m_ETStopAll         = m_pEventQueue->RegisterEvent( "Sound.StopAll" );
      m_ETSetMasterVolume = m_pEventQueue->RegisterEvent( "Sound.SetMasterVolume" );

      m_pEventQueue->AddListener( this );
    }
    return true;
  }
  else if ( hResult == DSERR_ALLOCATED )
  {
    // das Soundgerät ist bereits in Beschlag genommen
    Log( "Sound.General", "DirectSoundCreate returned DSERR_ALLOCATED" );

    m_SoundAlreadyBusy = true;
    m_Initialized = true;

    if ( m_pEventQueue )
    {
      m_ETPlaySound       = m_pEventQueue->RegisterEvent( "Sound.Play" );
      m_ETLoopSound       = m_pEventQueue->RegisterEvent( "Sound.Loop" );
      m_ETStopSound       = m_pEventQueue->RegisterEvent( "Sound.Stop" );
      m_ETStopAll         = m_pEventQueue->RegisterEvent( "Sound.StopAll" );
      m_ETSetMasterVolume = m_pEventQueue->RegisterEvent( "Sound.SetMasterVolume" );

      m_pEventQueue->AddListener( this );
    }
    return true;
  }
  if ( FAILED( hResult ) )
  {
    // ein unbekannter Fehler, wir gehen davon aus, daß es keinen Sound gibt
    Log( "Sound.General", CMisc::printf( "DirectSoundCreate returned unknown error (%x)", hResult ) );
    return false;
  }
  // DSound-Interface Teil 1 ok
  if ( DS_OK != m_pDirectSound->SetCooperativeLevel( hWnd, DSSCL_EXCLUSIVE ) )
  {
    Log( "Sound.General", "SetCooperativeLevel failed" );
    return false;
  }

  if ( m_pEventQueue )
  {
    m_ETPlaySound       = m_pEventQueue->RegisterEvent( "Sound.Play" );
    m_ETLoopSound       = m_pEventQueue->RegisterEvent( "Sound.Loop" );
    m_ETStopSound       = m_pEventQueue->RegisterEvent( "Sound.Stop" );
    m_ETStopAll         = m_pEventQueue->RegisterEvent( "Sound.StopAll" );
    m_ETSetMasterVolume = m_pEventQueue->RegisterEvent( "Sound.SetMasterVolume" );

    m_pEventQueue->AddListener( this );
  }

  m_Initialized = true;

  LoadAssets();
  Log( "Sound.General", "Initialize done" );
  return true;
}



bool DXSound::Release()
{
  if ( !m_Initialized )
  {
    // war gar nicht initialisiert
    return true;
  }

  ReleaseAssets();

  m_ReservedChannels.clear();

  if ( m_pEventQueue )
  {
    m_pEventQueue->UnregisterEvent( m_ETPlaySound );
    m_pEventQueue->UnregisterEvent( m_ETLoopSound );
    m_pEventQueue->UnregisterEvent( m_ETStopSound );
    m_pEventQueue->UnregisterEvent( m_ETStopAll );
    m_pEventQueue->UnregisterEvent( m_ETSetMasterVolume );

    m_pEventQueue->RemoveListener( this );
  }

  m_Initialized = false;
  m_SoundAlreadyBusy = false;

  if ( m_p3dPrimaryBuffer )
  {
    m_p3dPrimaryBuffer->Release();
    m_p3dPrimaryBuffer = NULL;
  }

  std::map<GR::u32, tSoundInfo*>::iterator it( m_Sounds.begin() );
  while ( it != m_Sounds.end() )
  {
    ReleaseWave( it->first );
    it = m_Sounds.begin();
  }

  std::map<GR::u32,std::list<tSoundInfo*> >::iterator   itDup( m_Duplicates.begin() );
  while ( itDup != m_Duplicates.end() )
  {
    std::list<tSoundInfo*>&   listInfos = itDup->second;

    std::list<tSoundInfo*>::iterator    itInfo( listInfos.begin() );
    while ( itInfo != listInfos.end() )
    {
      tSoundInfo*     pSound = *itInfo;

      GR::u32   Dummy;
      HRESULT hResult = pSound->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
      if ( hResult == DS_OK )
      {
        if ( ( Dummy && DSBSTATUS_PLAYING )
        ||   ( Dummy && DSBSTATUS_LOOPING ) )
        {
          // Wave läuft noch!
          pSound->dsWaveBuffer->Stop();
        }
        // Und Buffer entlassen
        pSound->dsWaveBuffer->Release();
        pSound->dsWaveBuffer = NULL;
      }
      else
      {
        pSound->dsWaveBuffer->Release();
        pSound->dsWaveBuffer = NULL;
      }
      delete pSound;

      ++itInfo;
    }

    ++itDup;
  }
  m_Duplicates.clear();

  if ( m_pPrimaryBuffer )
  {
    m_pPrimaryBuffer->Stop();
    m_pPrimaryBuffer->Release();
    m_pPrimaryBuffer = NULL;
  }

  if ( m_pDirectSound != NULL )
  {
    m_pDirectSound->Release();
    m_pDirectSound = NULL;
  }

  if ( m_hinstDSoundHandle != NULL )
  {
    FreeLibrary( m_hinstDSoundHandle );

    m_hinstDSoundHandle = NULL;
  }
  return true;
}



bool DXSound::Create3dListener()
{
  if ( m_pDirectSound == NULL )
  {
    return false;
  }

  DSBUFFERDESC1   dsBufferDesc;


  memset( &dsBufferDesc, 0, sizeof( DSBUFFERDESC1 ) );            // Zero it out.
  dsBufferDesc.dwSize = sizeof( DSBUFFERDESC1 );                  // Immer
  dsBufferDesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

  // DirectSoundBuffer anlegen
  HRESULT hResult = m_pDirectSound->CreateSoundBuffer( (LPDSBUFFERDESC)&dsBufferDesc,
                                                       &m_p3dPrimaryBuffer,
                                                       NULL );

  if ( FAILED( hResult ) )
  {
    return false;
  }
  return true;
}



bool DXSound::Set3dListenerPosition( float fX, float fY, float fZ )
{
  if ( m_p3dPrimaryBuffer == NULL )
  {
    return false;
  }
  LPDIRECTSOUND3DLISTENER lpDs3dListener;

  HRESULT hr = m_p3dPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener,
                                                   (void**)&lpDs3dListener );


  if ( FAILED( hr ) )
  {
    return false;
  }

  hr = lpDs3dListener->SetPosition( fX, fY, fZ, DS3D_IMMEDIATE );
  lpDs3dListener->Release();

  if ( FAILED( hr ) )
  {
    return false;
  }
  return true;
}



bool DXSound::Set3dListenerOrientation( float fFrontX, float fFrontY, float fFrontZ,
                                                float fUpX, float fUpY, float fUpZ )
{
  if ( m_p3dPrimaryBuffer == NULL )
  {
    return false;
  }
  LPDIRECTSOUND3DLISTENER lpDs3dListener;

  HRESULT hr = m_p3dPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener,
                                                   (void**)&lpDs3dListener );


  if ( FAILED( hr ) )
  {
    return false;
  }

  hr = lpDs3dListener->SetOrientation( fFrontX, fFrontY, fFrontZ,
                                       fUpX, fUpY, fUpZ,
                                       DS3D_IMMEDIATE );
  lpDs3dListener->Release();

  if ( FAILED( hr ) )
  {
    return false;
  }
  return true;
}



bool DXSound::Set3dListenerParams( float fX, float fY, float fZ,
                                         float fFrontX, float fFrontY, float fFrontZ,
                                         float fUpX, float fUpY, float fUpZ,
                                         float fVelocityX, float fVelocityY, float fVelocityZ )
{
  if ( m_p3dPrimaryBuffer == NULL )
  {
    return false;
  }
  LPDIRECTSOUND3DLISTENER lpDs3dListener;

  HRESULT hr = m_p3dPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener,
                                                   (void**)&lpDs3dListener );


  if ( FAILED( hr ) )
  {
    return false;
  }

  hr = lpDs3dListener->SetPosition( fX, fY, fZ, DS3D_DEFERRED );
  hr = lpDs3dListener->SetOrientation( fFrontX, fFrontY, fFrontZ,
                                       fUpX, fUpY, fUpZ,
                                       DS3D_DEFERRED );
  hr = lpDs3dListener->SetVelocity( fVelocityX, fVelocityY, fVelocityZ, DS3D_DEFERRED );
  lpDs3dListener->CommitDeferredSettings();
  lpDs3dListener->Release();

  if ( FAILED( hr ) )
  {
    return false;
  }
  return true;
}



bool DXSound::Set3dDistanceFactor( float fDistance )
{
  if ( m_p3dPrimaryBuffer == NULL )
  {
    return false;
  }
  LPDIRECTSOUND3DLISTENER lpDs3dListener;

  HRESULT hr = m_p3dPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener,
                                                   (void**)&lpDs3dListener );


  if ( FAILED( hr ) )
  {
    return false;
  }

  hr = lpDs3dListener->SetDistanceFactor( fDistance, DS3D_IMMEDIATE );
  lpDs3dListener->Release();

  if ( FAILED( hr ) )
  {
    return false;
  }
  return true;
}



bool DXSound::Set3dDopplerFactor( float fDoppler )
{
  if ( m_p3dPrimaryBuffer == NULL )
  {
    return false;
  }
  LPDIRECTSOUND3DLISTENER lpDs3dListener;

  HRESULT hr = m_p3dPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener,
                                                   (void**)&lpDs3dListener );


  if ( FAILED( hr ) )
  {
    return false;
  }

  hr = lpDs3dListener->SetDopplerFactor( fDoppler, DS3D_IMMEDIATE );
  lpDs3dListener->Release();

  if ( FAILED( hr ) )
  {
    return false;
  }
  return true;
}



bool DXSound::Set3dRollOffFactor( float fRollOff )
{
  if ( m_p3dPrimaryBuffer == NULL )
  {
    return false;
  }
  LPDIRECTSOUND3DLISTENER lpDs3dListener;

  HRESULT hr = m_p3dPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener,
                                                   (void**)&lpDs3dListener );


  if ( FAILED( hr ) )
  {
    return false;
  }

  hr = lpDs3dListener->SetRolloffFactor( fRollOff, DS3D_IMMEDIATE );
  lpDs3dListener->Release();

  if ( FAILED( hr ) )
  {
    return false;
  }
  return true;
}



bool DXSound::IsInitialized()
{
  return m_Initialized;
}



bool DXSound::SetMasterVolume( GR::u32 Volume, const eSoundType Type )
{
  if ( !m_Initialized )
  {
    // war gar nicht initialisiert
    return true;
  }
  if ( ( Volume < 0 )
  ||   ( Volume > 100 ) )
  {
    return false;
  }
  if ( ( Type == ST_ALL )
  ||   ( Type == ST_SOUND_EFFECT ) )
  {
    m_FXMasterVolume = Volume;
  }
  if ( ( Type == ST_ALL )
  ||   ( Type == ST_MUSIC ) )
  {
    m_MusicMasterVolume = Volume;
  }

  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( m_ReservedChannels.find( i ) != m_ReservedChannels.end() )
    {
      continue;
    }

    tSoundInfo&   SoundInfo = m_Channels[i];

    if ( SoundInfo.dsWaveBuffer != NULL )
    {
      if ( ( SoundInfo.Type == Type )
      ||   ( Type == ST_ALL ) )
      {
        if ( SoundInfo.Type == ST_MUSIC )
        {
          SoundInfo.dsWaveBuffer->SetVolume( m_VolumeRange[SoundInfo.Volume * m_MusicMasterVolume / 100] );
        }
        else if ( SoundInfo.Type == ST_SOUND_EFFECT )
        {
          SoundInfo.dsWaveBuffer->SetVolume( m_VolumeRange[SoundInfo.Volume * m_FXMasterVolume / 100] );
        }
      }
    }
  }
  return true;
}



GR::u32 DXSound::GetFreeID()
{
  GR::u32       Dummy;

  Dummy = 1;
  while ( m_Sounds.find( Dummy ) != m_Sounds.end() )
  {
    Dummy++;
  }
  return Dummy;
}



GR::u32 DXSound::LoadWave( const GR::String& Name, bool LoadAs3d, const eSoundType Type )
{
  GR::IO::FileStream  inFile;

  if ( !inFile.Open( Name.c_str() ) )
  {
    return 0;
  }
  return LoadWave( inFile, LoadAs3d, Type );
}



GR::u32 DXSound::LoadWave( const char* Name, bool LoadAs3d, const eSoundType Type )
{
  GR::IO::FileStream  inFile;

  if ( !inFile.Open( Name ) )
  {
    return 0;
  }
  return LoadWave( inFile, LoadAs3d, Type );
}



GR::u32 DXSound::LoadWave( IIOStream& Stream, bool LoadAs3d, const eSoundType Type )
{
  if ( !Stream.IsGood() )
  {
    return 0;
  }

  WAVEFORMATEX        wfDummy;


  DSBUFFERDESC1       dsBufferDesc;

  HRESULT             hResult;

  char*               pPointer1;
  char*               pPointer2;

  unsigned char       ucBuffer[16];

  GR::u32             ChunkSize,
                      Size1,
                      Size2,
                      ID;

  tSoundInfo*         pSoundInfo;


  if ( !m_Initialized )
  {
    // Sound ist nicht initialisiert
    Stream.Close();
    return 0;
  }

  if ( m_SoundAlreadyBusy )
  {
    Stream.Close();
    return 0;
  }

  ID = GetFreeID();

  // Wave einlesen
  Stream.ReadBlock( ucBuffer, 8 );
  if ( ( ucBuffer[0] != 82 )
  &&   ( ucBuffer[1] != 73 )
  &&   ( ucBuffer[2] != 70 )
  &&   ( ucBuffer[3] != 70 ) )
  {
    // kein RIFF
    Stream.Close();
    return 0;
  }
  Stream.ReadBlock( ucBuffer, 8 );
  if ( ( ucBuffer[0] != 87 )
  &&   ( ucBuffer[1] != 65 )
  &&   ( ucBuffer[2] != 86 )
  &&   ( ucBuffer[3] != 69 )
  &&   ( ucBuffer[4] != 102 )
  &&   ( ucBuffer[5] != 109 )
  &&   ( ucBuffer[6] != 116 )
  &&   ( ucBuffer[7] != 32 ) )
  {
    // kein WAVEfmt_
    Stream.Close();
    return 0;
  }
  ChunkSize = Stream.ReadU32();
  if ( ChunkSize != 16 )
  {
    // Nicht Standardgröße, ich bin verwörrt
    Stream.Close();
    return 0;
  }

  // Da ist jetzt das wichtige Zeugs drin
  Stream.ReadBlock( ucBuffer, 16 );

  Stream.SetPosition( ChunkSize - 16, IIOStream::PT_CURRENT );

  // Set up wave format structure.
  memset( &wfDummy, 0, sizeof( WAVEFORMATEX ) );
  wfDummy.wFormatTag = WAVE_FORMAT_PCM;    // Immer
  wfDummy.nChannels = ucBuffer[2] + 256 * ucBuffer[3];
  wfDummy.nSamplesPerSec = ucBuffer[4] + 256 * ucBuffer[5] + 65536 * ucBuffer[6] + 16777217 * ucBuffer[7];
  wfDummy.nBlockAlign = ucBuffer[12] + 256 * ucBuffer[13];
  wfDummy.nAvgBytesPerSec = wfDummy.nSamplesPerSec * wfDummy.nBlockAlign;
  wfDummy.wBitsPerSample = ucBuffer[14] + 256 * ucBuffer[15];

  pSoundInfo = new tSoundInfo();

  pSoundInfo->OriginalFrequency = wfDummy.nSamplesPerSec;
  pSoundInfo->Frequency         = wfDummy.nSamplesPerSec;
  pSoundInfo->Type              = Type;

  Stream.ReadBlock( ucBuffer, 4 );
  if ( ( ucBuffer[0] != 100 )
  &&   ( ucBuffer[1] != 97 )
  &&   ( ucBuffer[2] != 116 )
  &&   ( ucBuffer[3] != 97 ) )
  {
    // kein data
    delete pSoundInfo;
    Stream.Close();
    return 0;
  }
  ChunkSize = Stream.ReadU32();


  // DSBUFFERDESC1 füllen
  memset( &dsBufferDesc, 0, sizeof( DSBUFFERDESC1 ) );            // Zero it out.
  dsBufferDesc.dwSize = sizeof( DSBUFFERDESC1 );                  // Immer
  dsBufferDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
    //DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2;// | DSBCAPS_STICKYFOCUS;
  //  DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
  dsBufferDesc.dwBufferBytes = ChunkSize;

  if ( LoadAs3d )
  {
    dsBufferDesc.dwFlags |= DSBCAPS_CTRL3D;
  }

  dsBufferDesc.lpwfxFormat = (LPWAVEFORMATEX)&wfDummy;

  // DirectSoundBuffer anlegen
  pSoundInfo->dsWaveBuffer  = NULL;
  pSoundInfo->Size        = ChunkSize;
  hResult = m_pDirectSound->CreateSoundBuffer( (LPDSBUFFERDESC)&dsBufferDesc,
                                               &pSoundInfo->dsWaveBuffer,
                                               NULL );
  if ( hResult == DS_OK )
  {
    hResult = pSoundInfo->dsWaveBuffer->Lock( 0, ChunkSize, (void **)&pPointer1, (LPDWORD)&Size1, (void **)&pPointer2, (LPDWORD)&Size2, 0 );
    if ( hResult != DS_OK )
    {
      // Konnte Buffer nicht "locken"
      delete pSoundInfo;
      Stream.Close();
      return 0;
    }
    Stream.ReadBlock( (BYTE*)pPointer1, Size1 );
    if ( ( Size2 != 0 )
    &&   ( pPointer2 != NULL ) )
    {
      // Zweiten Teil nur bei Wrap einladen
      Stream.ReadBlock( (BYTE*)pPointer2, Size2 );
    }
    else
    {
      Size2 = 0;
    }
    Stream.Close();
    hResult = pSoundInfo->dsWaveBuffer->Unlock( pPointer1, Size1, pPointer2, Size2 );
    if ( hResult != DS_OK )
    {
      // Konnte Buffer nicht "unlocken"
      delete pSoundInfo;
      return 0;
    }
    m_Sounds[ID] = pSoundInfo;

    return ID;
  }
  // Failed
  Stream.Close();

  delete pSoundInfo;
  return 0;
}



GR::u32 DXSound::LoadWaveFromResource( const char* Resource, const char* ResourceType, const eSoundType Type )
{
  HRSRC               hrHandle;

  HGLOBAL             globHandle;

  WAVEFORMATEX        wfDummy;

  DSBUFFERDESC1       dsBufferDesc;

  HRESULT             hResult;

  BYTE*               pResource;

  char*               pPointer1;
  char*               pPointer2;

  GR::u32             ChunkSize,
                      Size1,
                      Size2,
                      ID,
                      ResourceSize;

  tSoundInfo*         pSoundInfo;


  if ( !m_Initialized )
  {
    // Sound ist nicht initialisiert
    return 0;
  }

  // Wave einlesen
  hrHandle = FindResource( NULL, Resource, ResourceType );
  if ( hrHandle == NULL )
  {
    // Resource nicht gefunden
    return 0;
  }
  globHandle = LoadResource( NULL, hrHandle );
  if ( globHandle == NULL )
  {
    // Resource konnte nicht geladen werden
    return 0;
  }
  pResource = (BYTE *)LockResource( globHandle );
  ResourceSize = SizeofResource( NULL, hrHandle );
  if ( ResourceSize == 0 )
  {
    // ResourceGröße konnte nicht geladen werden
    return 0;
  }

  if ( ( pResource[0] != 82 )
  &&   ( pResource[1] != 73 )
  &&   ( pResource[2] != 70 )
  &&   ( pResource[3] != 70 ) )
  {
    // kein RIFF
    FreeResource( hrHandle );
    return 0;
  }
  pResource += 8;
  if ( ( pResource[0] != 87 )
  &&   ( pResource[1] != 65 )
  &&   ( pResource[2] != 86 )
  &&   ( pResource[3] != 69 )
  &&   ( pResource[4] != 102 )
  &&   ( pResource[5] != 109 )
  &&   ( pResource[6] != 116 )
  &&   ( pResource[7] != 32 ) )
  {
    // kein WAVEfmt_
    FreeResource( hrHandle );
    return 0;
  }
  pResource += 8;

  ChunkSize = *(GR::u32*)pResource;//pFile->ReadU32();
  if ( ChunkSize != 16 )
  {
    // Nicht Standardgröße, ich bin verwörrt
    FreeResource( hrHandle );
    return 0;
  }

  // Da ist jetzt das wichtige Zeugs drin
  pResource += 4;
  //pFile->ReadBlock( ucBuffer, 16 );

  // Set up wave format structure.
  memset( &wfDummy, 0, sizeof( WAVEFORMATEX ) );
  wfDummy.wFormatTag = WAVE_FORMAT_PCM;    // Immer
  wfDummy.nChannels = pResource[2] + 256 * pResource[3];
  wfDummy.nSamplesPerSec = pResource[4] + 256 * pResource[5] + 65536 * pResource[6] + 16777217 * pResource[7];
  wfDummy.nBlockAlign = pResource[12] + 256 * pResource[13];
  wfDummy.nAvgBytesPerSec = wfDummy.nSamplesPerSec * wfDummy.nBlockAlign;
  wfDummy.wBitsPerSample = pResource[14] + 256 * pResource[15];

  pSoundInfo = new tSoundInfo();

  pSoundInfo->OriginalFrequency = wfDummy.nSamplesPerSec;
  pSoundInfo->Frequency         = wfDummy.nSamplesPerSec;
  pSoundInfo->Type              = Type;

  pResource += ChunkSize;

  if ( ( pResource[0] != 100 )
  &&   ( pResource[1] != 97 )
  &&   ( pResource[2] != 116 )
  &&   ( pResource[3] != 97 ) )
  {
    // kein data
    FreeResource( hrHandle );
    return 0;
  }
  pResource += 4;

  ChunkSize = *(GR::u32*)pResource;

  // DSBUFFERDESC füllen
  memset( &dsBufferDesc, 0, sizeof( DSBUFFERDESC1 ) );            // Zero it out.
  dsBufferDesc.dwSize = sizeof( DSBUFFERDESC1 );                  // Immer
  dsBufferDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
  //dsBufferDesc.Flags = DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
  dsBufferDesc.dwBufferBytes = ChunkSize;
  dsBufferDesc.lpwfxFormat = (LPWAVEFORMATEX)&wfDummy;

  // DirectSoundBuffer anlegen
  pSoundInfo->Size = ChunkSize;
  hResult = m_pDirectSound->CreateSoundBuffer( (LPDSBUFFERDESC)&dsBufferDesc,
                                                &pSoundInfo->dsWaveBuffer,
                                                NULL );
  if ( hResult == DS_OK )
  {
    hResult = pSoundInfo->dsWaveBuffer->Lock( 0, ChunkSize, (void **)&pPointer1, (LPDWORD)&Size1, (void **)&pPointer2, (LPDWORD)&Size2, 0 );
    if ( hResult != DS_OK )
    {
      // Konnte Buffer nicht "locken"
      FreeResource( hrHandle );
      return 0;
    }
    memcpy( (BYTE*)pPointer1, pResource, Size1 );
    //pFile->ReadBlock( (BYTE*)pPointer1, Size1 );
    if ( ( Size2 != 0 )
    &&   ( pPointer2 != NULL ) )
    {
      // Zweiten Teil nur bei Wrap einladen
      memcpy( (BYTE*)pPointer2, pResource + Size1, Size2 );
      //pFile->ReadBlock( (BYTE*)pPointer2, Size2 );
    }
    else
    {
      Size2 = 0;
    }
    hResult = pSoundInfo->dsWaveBuffer->Unlock( pPointer1, Size1, pPointer2, Size2 );
    if ( hResult != DS_OK )
    {
      // Konnte Buffer nicht "unlocken"
      FreeResource( hrHandle );
      return 0;
    }
    pSoundInfo->Flags = 0;
    FreeResource( hrHandle );
    ID = GetFreeID();
    m_Sounds[ID] = pSoundInfo;
    return ID;
  }
  // Failed
  FreeResource( hrHandle );
  return 0;
}



bool DXSound::ReleaseWave( GR::u32 ID )
{
  HRESULT           hResult;

  GR::u32           Dummy;

  tSoundInfo*       pSoundInfo;


  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  if ( pSoundInfo->dsWaveBuffer != NULL )
  {
    hResult = pSoundInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
  }
  else
  {
    hResult = DS_OK + 1;
  }
  if ( hResult == DS_OK )
  {
    if ( ( Dummy && DSBSTATUS_PLAYING )
    ||   ( Dummy && DSBSTATUS_LOOPING ) )
    {
      // Wave läuft noch!
      pSoundInfo->dsWaveBuffer->Stop();
    }
    // Und Buffer entlassen
    pSoundInfo->dsWaveBuffer->Release();
  }

  std::map<GR::u32,tSoundInfo*>::iterator itPos( m_Sounds.find( ID ) );
  if ( itPos != m_Sounds.end() )
  {
    itPos->second = NULL;
    m_Sounds.erase( itPos );
  }
  delete pSoundInfo;

  // Duplikate entfernen
  std::map<GR::u32,std::list<tSoundInfo*> >::iterator   itDup( m_Duplicates.find( ID ) );
  if ( itDup != m_Duplicates.end() )
  {
    std::list<tSoundInfo*>&   listInfos = itDup->second;

    std::list<tSoundInfo*>::iterator    itInfo( listInfos.begin() );
    while ( itInfo != listInfos.end() )
    {
      tSoundInfo*     pSound = *itInfo;

      GR::u32   Dummy;
      HRESULT hResult = pSound->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
      if ( hResult == DS_OK )
      {
        if ( ( Dummy && DSBSTATUS_PLAYING )
        ||   ( Dummy && DSBSTATUS_LOOPING ) )
        {
          // Wave läuft noch!
          pSound->dsWaveBuffer->Stop();
        }
        // Und Buffer entlassen
        pSound->dsWaveBuffer->Release();
        pSound->dsWaveBuffer = NULL;
      }
      else
      {
        pSound->dsWaveBuffer->Release();
        pSound->dsWaveBuffer = NULL;
      }
      delete pSound;

      ++itInfo;
    }

    m_Duplicates.erase( itDup );
  }
  return true;
}



bool DXSound::UnloadWave( GR::u32 ID )
{
  HRESULT             hResult;

  GR::u32             Dummy;


  if ( !m_Initialized )
  {
    // Sound ist nicht initialisiert
    return false;
  }

  tSoundInfo       *pSoundInfo;


  std::map<GR::u32,tSoundInfo*>::iterator itPos( m_Sounds.find( ID ) );
  if ( itPos == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  pSoundInfo = itPos->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  hResult = pSoundInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
  if ( hResult == DS_OK )
  {
    if ( ( Dummy && DSBSTATUS_PLAYING )
    ||   ( Dummy && DSBSTATUS_LOOPING ) )
    {
      // Wave läuft noch!
      pSoundInfo->dsWaveBuffer->Stop();
    }
    // Und Buffer entlassen
    pSoundInfo->dsWaveBuffer->Release();
    pSoundInfo->dsWaveBuffer = NULL;
    return true;
  }
  return false;
}



bool DXSound::StartDuplicate( GR::u32 ID )
{
  std::map<GR::u32,tSoundInfo*>::iterator   itOrig( m_Sounds.find( ID ) );
  if ( itOrig == m_Sounds.end() )
  {
    return false;
  }

  tSoundInfo*     pSoundOrig = itOrig->second;

  tSoundInfo*     pSoundInfo = NULL;

  std::map<GR::u32,std::list<tSoundInfo*> >::iterator   itDup( m_Duplicates.find( ID ) );
  if ( itDup != m_Duplicates.end() )
  {
    std::list<tSoundInfo*>&   listInfos = itDup->second;

    std::list<tSoundInfo*>::iterator    itInfo( listInfos.begin() );
    while ( itInfo != listInfos.end() )
    {
      tSoundInfo*   pSInfo = *itInfo;

      if ( pSInfo->dsWaveBuffer == NULL )
      {
        pSoundInfo = pSInfo;
        break;
      }
      GR::u32   Dummy = 0;
      HRESULT hResult = pSInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
      if ( hResult == DS_OK )
      {
        if ( !( Dummy & DSBSTATUS_PLAYING )
        &&   !( Dummy & DSBSTATUS_LOOPING ) )
        {
          // Wave ist fertig
          pSoundInfo = pSInfo;
          break;
        }
      }

      ++itInfo;
    }
  }

  if ( pSoundInfo == NULL )
  {
    // ein neues Duplikat anlegen
    pSoundInfo = new tSoundInfo();

    *pSoundInfo = *pSoundOrig;
    pSoundInfo->dsWaveBuffer = NULL;

    HRESULT   hRes = m_pDirectSound->DuplicateSoundBuffer( pSoundOrig->dsWaveBuffer, &pSoundInfo->dsWaveBuffer );
    if ( hRes != DS_OK )
    {
      pSoundInfo->dsWaveBuffer = NULL;
      delete pSoundInfo;
      return false;
    }
    m_Duplicates[ID].push_back( pSoundInfo );

    pSoundInfo->Type = pSoundOrig->Type;
  }

  pSoundInfo->Volume = 100;
  pSoundInfo->Frequency = pSoundInfo->OriginalFrequency;
  pSoundInfo->Pan = 0;

  HRESULT   hRes = S_OK;

  if ( !( pSoundInfo->Flags & FLAG_SOUND_POSITIONCHANGED ) )
  {
    if ( FAILED( hRes = pSoundInfo->dsWaveBuffer->SetCurrentPosition( 0 ) ) )
    {
      dh::Log( "DXSound::StartDuplicate SetCurrentPosition failed (%x)", hRes );
    }
  }

  // Wave-Lautstärke ist 100 -> wird rausgekürzt
  if ( FAILED( hRes = pSoundInfo->dsWaveBuffer->SetVolume( m_VolumeRange[MasterVolume( pSoundInfo->Type )] ) ) )
  {
    dh::Log( "DXSound::StartDuplicate SetVolume failed (%x)", hRes );
  }
  if ( FAILED( hRes = pSoundInfo->dsWaveBuffer->Play( 0, 0, 0 ) ) )
  {
    dh::Log( "DXSound::StartDuplicate Play failed (%x)", hRes );
  }
  return true;
}



bool DXSound::Play( GR::u32 ID, GR::u32 Offset )
{
  return PlayChannel( ID, -1, Offset );
}



bool DXSound::PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset,
                                GR::u32 Volume, GR::i32 iPan, GR::u32 Frequency, bool bLooping )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  std::map<GR::u32,tSoundInfo*>::iterator    it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  tSoundInfo* pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  if ( Channel == -1 )
  {
    Channel = FindFreeChannel();
    if ( Channel == -1 )
    {
      //dh::Log( "PlayInChannel No free channel found!" );
      return false;
    }
  }

  if ( Frequency == 0 )
  {
    Frequency = pSoundInfo->OriginalFrequency;
  }

  if ( m_Channels[Channel].dsWaveBuffer != NULL )
  {
    DWORD   Status = 0;
    m_Channels[Channel].dsWaveBuffer->GetStatus( &Status );
    if ( ( Status & DSBSTATUS_PLAYING )
    ||   ( Status & DSBSTATUS_LOOPING ) )
    {
      // läuft noch!
      m_Channels[Channel].dsWaveBuffer->Stop();
    }
    if ( m_Channels[Channel].dsWaveBuffer->Release() )
    {
      dh::Log( "DXSound::PlayChannel Release failed" );
    }
    m_Channels[Channel].dsWaveBuffer = NULL;
  }

  HRESULT   hRes = m_pDirectSound->DuplicateSoundBuffer( pSoundInfo->dsWaveBuffer, &m_Channels[Channel].dsWaveBuffer );
  if ( hRes != DS_OK )
  {
    dh::Log( "DXSound::PlayChannel DuplicateSoundBuffer failed (%x)", hRes );
    return false;
  }

  if ( ( Volume < 0 )
  ||   ( Volume > 100 ) )
  {
    Volume = 100;
  }

  m_Channels[Channel].Volume = (GR::u8)Volume;
  m_Channels[Channel].Frequency = pSoundInfo->OriginalFrequency;
  m_Channels[Channel].Pan = iPan;
  m_Channels[Channel].Looping = bLooping;
  m_Channels[Channel].SoundID = ID;

  m_Channels[Channel].dsWaveBuffer->SetCurrentPosition( Offset );

  // Wave-Lautstärke ist 100 -> wird rausgekürzt
  if ( FAILED( hRes = m_Channels[Channel].dsWaveBuffer->SetFrequency( Frequency ) ) )
  {
    dh::Log( "DXSound::PlayChannel SetFrequency failed (%x)", hRes );
  }
  if ( FAILED( hRes = m_Channels[Channel].dsWaveBuffer->SetPan( iPan * 100 ) ) )
  {
    dh::Log( "DXSound::PlayChannel SetPan failed (%x)", hRes );
  }
  if ( FAILED( hRes = m_Channels[Channel].dsWaveBuffer->SetVolume( m_VolumeRange[Volume * MasterVolume( pSoundInfo->Type ) / 100] ) ) )
  {
    dh::Log( "DXSound::PlayChannel SetVolume failed (%x)", hRes );
  }
  GR::u32   Flag = 0;
  if ( bLooping )
  {
    Flag = DSBPLAY_LOOPING;
  }
  if ( FAILED( hRes = m_Channels[Channel].dsWaveBuffer->Play( 0, 0, Flag ) ) )
  {
    dh::Log( "DXSound::PlayChannel Play failed (%x)", hRes );
  }
  return true;
}



GR::u32 DXSound::FindFreeChannel()
{
  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( m_ReservedChannels.find( i ) != m_ReservedChannels.end() )
    {
      // reserved channel
      continue;
    }

    if ( m_Channels[i].dsWaveBuffer == NULL )
    {
      return (GR::u32)i;
    }
    DWORD   Status = 0;
    if ( SUCCEEDED( m_Channels[i].dsWaveBuffer->GetStatus( &Status ) ) )
    {
      if ( ( !( Status & DSBSTATUS_PLAYING ) )
      &&   ( !( Status & DSBSTATUS_LOOPING ) ) )
      {
        return (GR::u32)i;
      }
    }
  }
  return -1;
}



void DXSound::ReserveChannel( GR::u32 Channel )
{
  m_ReservedChannels.insert( Channel );
}



void DXSound::UnreserveChannel( GR::u32 Channel )
{
  m_ReservedChannels.erase( Channel );
}



bool DXSound::Pause( GR::u32 ID )
{
  HRESULT             hResult;

  GR::u32               Dummy;


  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  std::map<GR::u32,tSoundInfo*>::iterator    it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  tSoundInfo *pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  hResult = pSoundInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
  if ( hResult == DS_OK )
  {
    if ( ( Dummy && DSBSTATUS_PLAYING )
    ||   ( Dummy && DSBSTATUS_LOOPING ) )
    {
      pSoundInfo->dsWaveBuffer->Stop();
      return true;
    }
  }
  return false;

}



bool DXSound::Resume( GR::u32 ID )
{
  HRESULT             hResult;

  GR::u32               Dummy;


  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  std::map<GR::u32,tSoundInfo*>::iterator    it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  tSoundInfo *pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  hResult = pSoundInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
  if ( hResult == DS_OK )
  {
    if ( !( Dummy && DSBSTATUS_PLAYING ) )
    {
      pSoundInfo->dsWaveBuffer->Play( 0, 0, pSoundInfo->Looping );
      return true;
    }
  }
  return false;
}



bool DXSound::Loop( GR::u32 ID )
{
  return LoopChannel( ID, -1 );
}



bool DXSound::LoopChannel( GR::u32 ID, GR::u32 Channel )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  if ( pSoundInfo->Modifyable )
  {
    // den Buffer direkt angehen!
    if ( pSoundInfo->dsWaveBuffer != NULL )
    {
      pSoundInfo->Frequency = pSoundInfo->OriginalFrequency;
      pSoundInfo->Pan = 0;
      //pSoundInfo->Volume = 100;
      pSoundInfo->dsWaveBuffer->SetCurrentPosition( 0 );
      //pSoundInfo->dsWaveBuffer->SetVolume( volumeRange[m_ucMasterVolume] );
      if ( FAILED( pSoundInfo->dsWaveBuffer->Play( 0, 0, DSBPLAY_LOOPING ) ) )
      {
        dh::Log( "Play failed!" );
      }
      pSoundInfo->Looping = true;
    }
    return true;
  }

  if ( Channel == -1 )
  {
    Channel = FindFreeChannel();
    if ( Channel == -1 )
    {
      //dh::Log( "PlayInChannel No free channel found!" );
      return false;
    }
  }

  if ( m_Channels[Channel].dsWaveBuffer != NULL )
  {
    DWORD   Status = 0;
    m_Channels[Channel].dsWaveBuffer->GetStatus( &Status );
    if ( ( Status & DSBSTATUS_PLAYING )
    ||   ( Status & DSBSTATUS_LOOPING ) )
    {
      // läuft noch!
      m_Channels[Channel].dsWaveBuffer->Stop();
    }
    m_Channels[Channel].dsWaveBuffer->Release();
    m_Channels[Channel].dsWaveBuffer = NULL;
  }

  HRESULT   hRes = m_pDirectSound->DuplicateSoundBuffer( pSoundInfo->dsWaveBuffer, &m_Channels[Channel].dsWaveBuffer );
  if ( hRes != DS_OK )
  {
    return false;
  }

  m_Channels[Channel].SoundID = ID;
  m_Channels[Channel].Frequency = m_Channels[Channel].OriginalFrequency;
  m_Channels[Channel].Pan = 0;
  m_Channels[Channel].Volume = (GR::u8)MasterVolume( pSoundInfo->Type );
  m_Channels[Channel].dsWaveBuffer->SetCurrentPosition( 0 );
  m_Channels[Channel].dsWaveBuffer->SetVolume( m_VolumeRange[MasterVolume( pSoundInfo->Type )] );
  m_Channels[Channel].dsWaveBuffer->Play( 0, 0, DSBPLAY_LOOPING );
  m_Channels[Channel].Looping = true;
  return true;
}



bool DXSound::PlayDetail( GR::u32 ID, GR::i32 slVolume, GR::i32 Pan, GR::u32 iFrequency, bool bLooping )
{
  return PlayChannel( ID, -1, 0, slVolume, Pan, iFrequency, bLooping );
}



bool DXSound::Play3d( GR::u32 ID, float fX, float fY, float fZ, bool bLooping )
{
  HRESULT             hResult;

  GR::u32             Dummy;


  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo*   pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  hResult = pSoundInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
  if ( hResult == DS_OK )
  {
    if ( ( Dummy && DSBSTATUS_PLAYING )
    ||   ( Dummy && DSBSTATUS_LOOPING ) )
    {
      // Wave läuft noch!
      pSoundInfo->dsWaveBuffer->Stop();
    }
    pSoundInfo->dsWaveBuffer->SetCurrentPosition( 0 );
    pSoundInfo->dsWaveBuffer->SetVolume( m_VolumeRange[MasterVolume( pSoundInfo->Type )] );
    pSoundInfo->Volume = 100;

    Set3dPosition( ID, fX, fY, fZ );

    if ( bLooping )
    {
      pSoundInfo->dsWaveBuffer->Play( 0, 0, DSBPLAY_LOOPING );
    }
    else
    {
      pSoundInfo->dsWaveBuffer->Play( 0, 0, 0 );
    }
    return true;
  }
  return false;
}



bool DXSound::StopChannel( GR::u32 Channel )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }

  if ( Channel >= m_Channels.size() )
  {
    return false;
  }

  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  if ( pSoundInfo->dsWaveBuffer != NULL )
  {
    pSoundInfo->dsWaveBuffer->Stop();
  }
  return true;
}



bool DXSound::Stop( GR::u32 ID )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( ( m_Channels[i].dsWaveBuffer )
    &&   ( m_Channels[i].SoundID == ID ) )
    {
      m_Channels[i].dsWaveBuffer->Stop();
    }
  }
  return true;
}



bool DXSound::StopAll()
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( m_Channels[i].dsWaveBuffer )
    {
      m_Channels[i].dsWaveBuffer->Stop();
    }
  }
  return true;
}



bool DXSound::SetFrequency( GR::u32 ID, GR::u32 Frequency )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( m_Channels[i].SoundID == ID )
    {
      m_Channels[i].dsWaveBuffer->SetFrequency( Frequency );
      m_Channels[i].Frequency = Frequency;
    }
  }
  return true;
}



bool DXSound::SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }
  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( pSoundInfo == NULL )
  {
    return true;
  }
  pSoundInfo->Frequency = Frequency;
  if ( pSoundInfo->dsWaveBuffer == NULL )
  {
    return true;
  }

  pSoundInfo->dsWaveBuffer->SetFrequency( Frequency );

  return true;
}



bool DXSound::SetPos( GR::u32 ID, GR::u32 Pos )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  pSoundInfo->dsWaveBuffer->SetCurrentPosition( Pos );
  pSoundInfo->Flags |= FLAG_SOUND_POSITIONCHANGED;
  return true;
}



bool DXSound::SetChannelPos( GR::u32 Channel, GR::u32 Pos )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    dh::Log( "DXSound::SetChannelPos Not initialized" );
    return false;
  }
  if ( Channel >= m_Channels.size() )
  {
    dh::Log( "DXSound::SetChannelPos Channel out of bounds %d >= %d", Channel, m_Channels.size() );
    return false;
  }
  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( ( pSoundInfo == NULL )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    dh::Log( "DXSound::SetChannelPos No SoundBuffer" );
    return true;
  }

  if ( FAILED( pSoundInfo->dsWaveBuffer->SetCurrentPosition( Pos ) ) )
  {
    dh::Log( "DXSound::SetCurrentPosition failed" );
  }
  pSoundInfo->Flags |= FLAG_SOUND_POSITIONCHANGED;
  return true;
}



bool DXSound::SetPan( GR::u32 ID, signed long Pan )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  pSoundInfo->dsWaveBuffer->SetPan( Pan * 100 );

  pSoundInfo->Pan = Pan;
  return true;
}



bool DXSound::SetChannelPan( GR::u32 Channel, signed long Pan )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }
  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( ( pSoundInfo == NULL )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    return true;
  }

  pSoundInfo->dsWaveBuffer->SetPan( Pan * 100 );

  pSoundInfo->Pan = Pan;
  return true;
}



bool DXSound::SetVolume( GR::u32 ID, GR::u32 Volume )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  pSoundInfo->dsWaveBuffer->SetVolume( m_VolumeRange[Volume * MasterVolume( pSoundInfo->Type ) / 100] );
  pSoundInfo->Volume = (GR::u8)Volume;
  return true;
}



bool DXSound::SetChannelVolume( GR::u32 Channel, GR::u32 Volume )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }
  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( ( pSoundInfo == NULL )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    return true;
  }

  pSoundInfo->dsWaveBuffer->SetVolume( m_VolumeRange[Volume * MasterVolume( pSoundInfo->Type ) / 100] );
  pSoundInfo->Volume = (GR::u8)Volume;
  return true;
}



bool DXSound::IsPlaying( GR::u32 ID )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( ( m_Channels[i].dsWaveBuffer )
    &&   ( m_Channels[i].SoundID == ID ) )
    {
      DWORD   Status = 0;
      if ( SUCCEEDED( m_Channels[i].dsWaveBuffer->GetStatus( &Status ) ) )
      {
        if ( ( Status & DSBSTATUS_PLAYING )
        ||   ( Status & DSBSTATUS_LOOPING ) )
        {
          // Wave läuft noch!
          return true;
        }
      }
    }
  }
  return false;
}



bool DXSound::IsChannelPlaying( GR::u32 Channel )
{
  HRESULT           hResult;

  GR::u32             Dummy;


  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }


  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( ( pSoundInfo == NULL )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  hResult = pSoundInfo->dsWaveBuffer->GetStatus( (LPDWORD)&Dummy );
  if ( hResult == DS_OK )
  {
    if ( ( Dummy && DSBSTATUS_PLAYING )
    ||   ( Dummy && DSBSTATUS_LOOPING ) )
    {
      // Wave läuft noch!
      return true;
    }
  }
  return false;
}



GR::u32 DXSound::GetCurrentFrequency( GR::u32 ID )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }

  for ( size_t i = 0; i < m_Channels.size(); ++i )
  {
    if ( ( m_Channels[i].dsWaveBuffer )
    &&   ( m_Channels[i].SoundID == ID ) )
    {
      return m_Channels[i].Frequency;
    }
  }
  return 0;
}



GR::u32 DXSound::GetChannelCurrentFrequency( GR::u32 Channel )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }

  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  return pSoundInfo->Frequency;
}



GR::u32 DXSound::GetOriginalFrequency( GR::u32 ID )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  return pSoundInfo->OriginalFrequency;
}



GR::u32 DXSound::GetChannelOriginalFrequency( GR::u32 Channel )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }

  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  return pSoundInfo->OriginalFrequency;
}



GR::u32 DXSound::GetVolume( GR::u32 ID )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  return pSoundInfo->Volume;
}



GR::u32 DXSound::GetChannelVolume( GR::u32 Channel )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }
  if ( Channel >= m_Channels.size() )
  {
    return false;
  }

  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }
  return pSoundInfo->Volume;
}



GR::u32 DXSound::GetPos( GR::u32 ID )
{
  GR::u32     Dummy,
              Dummy2;


  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 1;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  pSoundInfo->dsWaveBuffer->GetCurrentPosition( (LPDWORD)&Dummy, (LPDWORD)&Dummy2 );
  return Dummy;
}



GR::u32 DXSound::GetChannelPos( GR::u32 Channel )
{
  GR::u32     Dummy,
              Dummy2;


  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return 0;
  }
  if ( Channel >= m_Channels.size() )
  {
    return 0;
  }

  tSoundInfo*     pSoundInfo = &m_Channels[Channel];
  if ( ( pSoundInfo == NULL )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    // den Sound gibt es doch sowieso nicht
    return 0;
  }

  pSoundInfo->dsWaveBuffer->GetCurrentPosition( (LPDWORD)&Dummy, (LPDWORD)&Dummy2 );
  return Dummy;
}



bool DXSound::Set3dPosition( GR::u32 ID, float fX, float fY, float fZ )
{
  if ( ( !m_Initialized )
  ||   ( m_SoundAlreadyBusy ) )
  {
    // Sound ist nicht initialisiert
    return false;
  }
  tSoundInfo       *pSoundInfo;

  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  pSoundInfo = it->second;
  if ( pSoundInfo == NULL )
  {
    // den Sound gibt es doch sowieso nicht
    return true;
  }

  LPDIRECTSOUND3DBUFFER     lpDs3dBuffer;


  HRESULT hRes = pSoundInfo->dsWaveBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void**)&lpDs3dBuffer );
  if ( FAILED( hRes ) )
  {
    return false;
  }

  if ( FAILED( lpDs3dBuffer->SetPosition( fX, fY, fZ, DS3D_IMMEDIATE ) ) )
  {
    dh::Log( "Failed to set sound 3d position\n" );
  }
  lpDs3dBuffer->Release();

  return true;
}



DXSound& DXSound::Instance()
{
  static    DXSound   g_Instance;

  return g_Instance;
}



void DXSound::ProcessEvent( const tGlobalEvent& Event )
{
  if ( ( Event.m_Type == m_ETLoopSound )
  ||   ( Event.m_Type == m_ETPlaySound ) )
  {
    GR::u32     SoundHandle = (GR::u32)Event.m_Param1;

    if ( !Event.m_Param.empty() )
    {
      // Asset-Loader
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
      if ( pLoader )
      {
        Xtreme::Asset::XAssetSound* pAsset = (Xtreme::Asset::XAssetSound*)pLoader->Asset( Xtreme::Asset::XA_SOUND, Event.m_Param.c_str() );
        if ( pAsset )
        {
          SoundHandle = pAsset->Handle();
        }
        else
        {
          dh::Error( "XSound: Loop/Play Asset Sound %s not found", Event.m_Param.c_str() );
        }
      }
      else
      {
        dh::Error( "XSound: Loop/Play Asset Sound %s without Asset Loader!", Event.m_Param.c_str() );
      }
    }
    if ( Event.m_Type == m_ETLoopSound )
    {
      Loop( SoundHandle );
    }
    else if ( Event.m_Type == m_ETPlaySound )
    {
      Play( SoundHandle );
    }
  }
  else if ( Event.m_Type == m_ETStopAll )
  {
    StopAll();
  }
  else if ( Event.m_Type == m_ETStopSound )
  {
    GR::u32     SoundHandle = (GR::u32)Event.m_Param1;

    if ( !Event.m_Param.empty() )
    {
      // Asset-Loader
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
      if ( pLoader )
      {
        Xtreme::Asset::XAssetSound* pAsset = (Xtreme::Asset::XAssetSound*)pLoader->Asset( Xtreme::Asset::XA_SOUND, Event.m_Param.c_str() );
        if ( pAsset )
        {
          SoundHandle = pAsset->Handle();
        }
        else
        {
          dh::Error( "XSound: Stop Asset Sound %s not found", Event.m_Param.c_str() );
        }
      }
      else
      {
        dh::Error( "XSound: Stop Asset Sound %s without Asset Loader!", Event.m_Param.c_str() );
      }
    }
    Stop( SoundHandle );
  }
  else if ( Event.m_Type == m_ETSetMasterVolume )
  {
    SetMasterVolume( (unsigned char)Event.m_Param1 );
  }
}



GR::u32 DXSound::CreateBufferFromMemory( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                              int iBitsPerSample, GR::up* pData, GR::up Size,
                                              bool bLoadAs3d, const eSoundType sType )
{
  if ( m_pDirectSound == NULL )
  {
    return 0;
  }

  WAVEFORMATEX        wfDummy;

  // Set up wave format structure.
  memset( &wfDummy, 0, sizeof( WAVEFORMATEX ) );
  wfDummy.wFormatTag      = WAVE_FORMAT_PCM;    // Immer
  wfDummy.nChannels       = iChannels;
  wfDummy.nSamplesPerSec  = iSamplesPerSec;
  wfDummy.nBlockAlign     = iBlockAlign;
  wfDummy.nAvgBytesPerSec = wfDummy.nSamplesPerSec * wfDummy.nBlockAlign;
  wfDummy.wBitsPerSample  = iBitsPerSample;

  tSoundInfo* pSoundInfo = new tSoundInfo();

  pSoundInfo->OriginalFrequency = wfDummy.nSamplesPerSec;
  pSoundInfo->Frequency         = wfDummy.nSamplesPerSec;
  pSoundInfo->Type              = sType;

  DSBUFFERDESC1     dsBufferDesc;

  memset( &dsBufferDesc, 0, sizeof( DSBUFFERDESC1 ) );
  dsBufferDesc.dwSize         = sizeof( DSBUFFERDESC1 );
  dsBufferDesc.dwFlags        = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS;
  dsBufferDesc.dwBufferBytes  = (GR::u32)Size;

  if ( bLoadAs3d )
  {
    dsBufferDesc.dwFlags |= DSBCAPS_CTRL3D;
  }

  dsBufferDesc.lpwfxFormat = (LPWAVEFORMATEX)&wfDummy;

  pSoundInfo->dsWaveBuffer = NULL;
  HRESULT hResult = m_pDirectSound->CreateSoundBuffer( (LPDSBUFFERDESC)&dsBufferDesc,
                                               &pSoundInfo->dsWaveBuffer,
                                               NULL );
  if ( hResult != DS_OK )
  {
    delete pSoundInfo;
    return 0;
  }

  GR::u32     Size1 = 0,
            Size2 = 0;
  GR::u8*   pPointer1 = NULL;
  GR::u8*   pPointer2 = NULL;

  pSoundInfo->Size = Size;

  hResult = pSoundInfo->dsWaveBuffer->Lock( 0, (GR::u32)Size, (void **)&pPointer1, (LPDWORD)&Size1, (void **)&pPointer2, (LPDWORD)&Size2, 0 );
  if ( hResult != DS_OK )
  {
    // Konnte Buffer nicht "locken"
    delete pSoundInfo;
    return 0;
  }
  memcpy( pPointer1, pData, Size1 );
  if ( ( Size2 != 0 )
  &&   ( pPointer2 != NULL ) )
  {
    // Zweiten Teil nur bei Wrap einladen
    memcpy( pPointer2, ( (BYTE*)pData ) + Size1, Size2 );
  }
  else
  {
    Size2 = 0;
  }
  hResult = pSoundInfo->dsWaveBuffer->Unlock( pPointer1, Size1, pPointer2, Size2 );
  if ( hResult != DS_OK )
  {
    // Konnte Buffer nicht "unlocken"
    delete pSoundInfo;
    return 0;
  }

  GR::u32 ID = GetFreeID();

  m_Sounds[ID] = pSoundInfo;

  return ID;
}



GR::u32 DXSound::CreateModifyableBuffer( int iChannels, int iSamplesPerSec, int iBlockAlign,
                                              int iBitsPerSample, GR::up Size,
                                              bool bLoadAs3d, const eSoundType sType )
{
  if ( m_pDirectSound == NULL )
  {
    return 0;
  }

  WAVEFORMATEX        wfDummy;

  // Set up wave format structure.
  memset( &wfDummy, 0, sizeof( WAVEFORMATEX ) );
  wfDummy.wFormatTag      = WAVE_FORMAT_PCM;    // Immer
  wfDummy.nChannels       = iChannels;
  wfDummy.nSamplesPerSec  = iSamplesPerSec;
  wfDummy.nBlockAlign     = iBlockAlign;
  wfDummy.nAvgBytesPerSec = wfDummy.nSamplesPerSec * wfDummy.nBlockAlign;
  wfDummy.wBitsPerSample  = iBitsPerSample;

  tSoundInfo* pSoundInfo = new tSoundInfo();

  pSoundInfo->OriginalFrequency = wfDummy.nSamplesPerSec;
  pSoundInfo->Frequency         = wfDummy.nSamplesPerSec;
  pSoundInfo->Type              = sType;

  DSBUFFERDESC1     dsBufferDesc;

  memset( &dsBufferDesc, 0, sizeof( DSBUFFERDESC1 ) );
  dsBufferDesc.dwSize         = sizeof( DSBUFFERDESC1 );
  //dsBufferDesc.Flags        = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
  dsBufferDesc.dwFlags        = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STICKYFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
  dsBufferDesc.dwBufferBytes  = (GR::u32)Size;

  if ( bLoadAs3d )
  {
    dsBufferDesc.dwFlags |= DSBCAPS_CTRL3D;
  }

  dsBufferDesc.lpwfxFormat = (LPWAVEFORMATEX)&wfDummy;

  pSoundInfo->dsWaveBuffer = NULL;
  HRESULT hResult = m_pDirectSound->CreateSoundBuffer( (LPDSBUFFERDESC)&dsBufferDesc,
                                               &pSoundInfo->dsWaveBuffer,
                                               NULL );
  if ( hResult != DS_OK )
  {
    delete pSoundInfo;
    return 0;
  }

  GR::u32     Size1 = 0,
            Size2 = 0;
  GR::u8*   pPointer1 = NULL;
  GR::u8*   pPointer2 = NULL;

  pSoundInfo->Size = Size;
  pSoundInfo->Modifyable = true;

  hResult = pSoundInfo->dsWaveBuffer->Lock( 0, (GR::u32)Size, (void **)&pPointer1, (LPDWORD)&Size1, (void **)&pPointer2, (LPDWORD)&Size2, 0 );
  if ( hResult != DS_OK )
  {
    // Konnte Buffer nicht "locken"
    delete pSoundInfo;
    return 0;
  }
  memset( pPointer1, 0, Size1 );
  if ( ( Size2 != 0 )
  &&   ( pPointer2 != NULL ) )
  {
    // Zweiten Teil nur bei Wrap einladen
    memset( pPointer2, 0, Size2 );
  }
  hResult = pSoundInfo->dsWaveBuffer->Unlock( pPointer1, Size1, pPointer2, Size2 );
  if ( hResult != DS_OK )
  {
    // Konnte Buffer nicht "unlocken"
    delete pSoundInfo;
    return 0;
  }

  GR::u32 ID = GetFreeID();

  m_Sounds[ID] = pSoundInfo;

  return ID;
}



bool DXSound::LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                         LPVOID* pBlock1, GR::u32* Block1Size,
                                         LPVOID* pBlock2, GR::u32* Block2Size )
{
  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    dh::Log( "No lock1" );
    return false;
  }
  tSoundInfo*   pSoundInfo( it->second );

  if ( ( !pSoundInfo->Modifyable )
  ||   ( pSoundInfo->Locked )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    dh::Log( "No lock2" );
    return false;
  }
  if ( FAILED( pSoundInfo->dsWaveBuffer->Lock( Start, LockSize, pBlock1, (LPDWORD)Block1Size,
                                                                             pBlock2, (LPDWORD)Block2Size, 0 ) ) )
  {
    dh::Log( "Lock failed" );
    return false;
  }
  if ( pBlock1 )
  {
    pSoundInfo->pLockPointer1 = *pBlock1;
    pSoundInfo->LockSize1   = *Block1Size;
  }
  else
  {
    pSoundInfo->pLockPointer1 = NULL;
    pSoundInfo->LockSize1   = 0;
  }
  if ( pBlock2 )
  {
    pSoundInfo->pLockPointer2 = *pBlock2;
    pSoundInfo->LockSize2   = *Block2Size;
  }
  else
  {
    pSoundInfo->pLockPointer2 = NULL;
    pSoundInfo->LockSize2   = 0;
  }
  /*
  dh::Log( "Locked %x (%d) and %x (%d)",
           pSoundInfo->m_pLockPointer1, pSoundInfo->m_LockSize1,
           pSoundInfo->m_pLockPointer2, pSoundInfo->m_LockSize2 )
           ;
           */
  pSoundInfo->Locked = true;
  return true;
}



bool DXSound::UnlockModifyableBuffer( GR::u32 ID )
{
  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  tSoundInfo*   pSoundInfo( it->second );

  if ( ( !pSoundInfo->Modifyable )
  ||   ( !pSoundInfo->Locked )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    return false;
  }

  /*
  dh::Log( "unLocked %x (%d) and %x (%d)",
           pSoundInfo->m_pLockPointer1, pSoundInfo->m_LockSize1,
           pSoundInfo->m_pLockPointer2, pSoundInfo->m_LockSize2 );
           */

  HRESULT   hRes = DS_OK;

  if ( FAILED( hRes = pSoundInfo->dsWaveBuffer->Unlock( pSoundInfo->pLockPointer1,
                                                 pSoundInfo->LockSize1,
                                                 pSoundInfo->pLockPointer2,
                                                 pSoundInfo->LockSize2 ) ) )
  {
    dh::Log( "Unlock failed (%x)", hRes );
    return false;
  }
  pSoundInfo->Locked = false;
  return true;
}



bool DXSound::AddNotification( GR::u32 ID, GR::u32 Position, HANDLE hEvent )
{
  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  tSoundInfo*   pSoundInfo( it->second );

  if ( ( !pSoundInfo->Modifyable )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    return false;
  }
  // TODO - prüfen, ob gestoppt? (laut doku muss das sein)
  LPDIRECTSOUNDNOTIFY   pSoundBufferNotify = NULL;

  if ( FAILED( pSoundInfo->dsWaveBuffer->QueryInterface( IID_IDirectSoundNotify,(LPVOID*)&pSoundBufferNotify ) ) )
  {
    return false;
  }

  DSBPOSITIONNOTIFY   dsbNotify;

  dsbNotify.dwOffset      = Position;
  dsbNotify.hEventNotify  = hEvent;

  pSoundInfo->Notifications.push_back( dsbNotify );

  bool bResult = SUCCEEDED( pSoundBufferNotify->SetNotificationPositions( (DWORD)pSoundInfo->Notifications.size(),
                                                                          &pSoundInfo->Notifications[0] ) );

  if ( !bResult )
  {
    //dh::Log( "AddNotification failed" );
  }
  pSoundBufferNotify->Release();

  return bResult;
}



bool DXSound::RemoveNotification( GR::u32 ID, GR::u32 Position, HANDLE hEvent )
{
  std::map<GR::u32,tSoundInfo*>::iterator it( m_Sounds.find( ID ) );
  if ( it == m_Sounds.end() )
  {
    // den Sound gibt es doch sowieso nicht
    return false;
  }
  tSoundInfo*   pSoundInfo( it->second );

  if ( ( !pSoundInfo->Modifyable )
  ||   ( pSoundInfo->dsWaveBuffer == NULL ) )
  {
    return false;
  }
  tSoundInfo::tNotifications::iterator    itNot( pSoundInfo->Notifications.begin() );
  while ( itNot != pSoundInfo->Notifications.end() )
  {
    DSBPOSITIONNOTIFY&    dsbNotify( *itNot );

    if ( ( dsbNotify.dwOffset == Position )
    &&   ( dsbNotify.hEventNotify == hEvent ) )
    {
      itNot = pSoundInfo->Notifications.erase( itNot );
      continue;
    }
    ++itNot;
  }
  // TODO - prüfen, ob gestoppt? (laut doku muss das sein)
  LPDIRECTSOUNDNOTIFY   pSoundBufferNotify = NULL;

  if ( FAILED( pSoundInfo->dsWaveBuffer->QueryInterface( IID_IDirectSoundNotify,(LPVOID*)&pSoundBufferNotify ) ) )
  {
    return false;
  }

  DSBPOSITIONNOTIFY   dsbNotify;

  dsbNotify.dwOffset      = Position;
  dsbNotify.hEventNotify  = hEvent;

  bool bResult = SUCCEEDED( pSoundBufferNotify->SetNotificationPositions( (DWORD)pSoundInfo->Notifications.size(),
                                                                          &pSoundInfo->Notifications[0] ) );

  pSoundBufferNotify->Release();

  return bResult;
}



GR::u32 DXSound::MasterVolume( const eSoundType sType )
{
  if ( sType == ST_SOUND_EFFECT )
  {
    return m_FXMasterVolume;
  }
  else if ( sType == ST_MUSIC )
  {
    return m_MusicMasterVolume;
  }
  else if ( sType == ST_ALL )
  {
    return m_FXMasterVolume;
  }
  return 0;
}





