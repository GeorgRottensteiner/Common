#define INITGUID

#include <dmusicc.h>
#include <dmusici.h>
#include <dsound.h>

#include <debug/debugclient.h>

#include <MasterFrame/XWindow.h>

#include "DXMidi.h"

#pragma comment( lib, "dxguid.lib" )

const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };



CDXMidi::CDXMidi() :
  m_pLoader( NULL ),
  m_pPerformance( NULL ),
  m_pDSListener( NULL ),
  m_pSegment( NULL ),
  m_bCleanupCOM( FALSE ),
  m_bPaused( false )
{
  
  // Initialize COM
  m_bCleanupCOM = SUCCEEDED( CoInitialize( NULL ) );

}



CDXMidi::~CDXMidi()
{

  Release();

}



bool CDXMidi::Release()
{

  if ( m_pLoader )
  {
    m_pLoader->Release();
    m_pLoader = NULL;
  }
  if ( m_pSegment )
  {
    delete m_pSegment;
    m_pSegment = NULL;
  }
  if ( m_pDSListener )
  {
    m_pDSListener->Release();
    m_pDSListener = NULL;
  }

  if ( m_pPerformance )
  {
    // If there is any music playing, stop it.
    m_pPerformance->Stop( NULL, NULL, 0, 0 );
    m_pPerformance->CloseDown();

    if ( m_pPerformance )
    {
      m_pPerformance->Release();
      m_pPerformance = NULL;
    }
  }

  if ( m_bCleanupCOM )
  {
    CoUninitialize();
  }

  return true;

}



bool CDXMidi::Initialize( GR::IEnvironment& Environment )
{
  DWORD dwPChannels = 128;
  DWORD dwDefaultPathType = DMUS_APATH_DYNAMIC_STEREO;
  LPDIRECTSOUND pDS = NULL;

  HRESULT         hr;

  IDirectSound**  ppDirectSound = NULL;

  Xtreme::IAppWindow* pWindowService = ( Xtreme::IAppWindow* )Environment.Service( "Window" );
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }


  if ( pDS )
  {
    ppDirectSound = &pDS;
  }

  if ( FAILED( hr = CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
                                      IID_IDirectMusicLoader8, (void**)&m_pLoader ) ) )
  {
    return false;
  }

  if ( FAILED( hr = CoCreateInstance( CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, 
                                      IID_IDirectMusicPerformance8, (void**)&m_pPerformance ) ) )
  {
    return false;
  }

  // Initialize the performance with the standard audio path.
  // This initializes both DirectMusic and DirectSound and 
  // sets up the synthesizer. Typcially its easist to use an 
  // audio path for playing music and sound effects.
  if ( FAILED( hr = m_pPerformance->InitAudio( NULL, ppDirectSound, hWnd, dwDefaultPathType,
                                               dwPChannels, DMUS_AUDIOF_ALL, NULL ) ) )
  {
    if ( hr == DSERR_NODRIVER )
    {
      //DXTRACE( TEXT("Warning: No sound card found\n") );
      return false;
    }
    return false;
  }

  // Get the listener from the in the default audio path.
  IDirectMusicAudioPath8*   pAudioPath = GetDefaultAudioPath();
  if ( pAudioPath ) // might be NULL if dwDefaultPathType == 0
  {
    if ( SUCCEEDED( hr = pAudioPath->GetObjectInPath( 0, DMUS_PATH_PRIMARY_BUFFER, 0,
                                                      GUID_NULL, 0, IID_IDirectSound3DListener, 
                                                      (LPVOID*)&m_pDSListener ) ) )
    {
      // Get listener parameters
      m_dsListenerParams.dwSize = sizeof( DS3DLISTENER );
      m_pDSListener->GetAllParameters( &m_dsListenerParams );
    }
  }

  return true;

}



IDirectMusicAudioPath8* CDXMidi::GetDefaultAudioPath()
{

  IDirectMusicAudioPath8*     pAudioPath = NULL;

  if ( NULL == m_pPerformance )
  {
    return NULL;
  }

  m_pPerformance->GetDefaultAudioPath( &pAudioPath );

  return pAudioPath;

}



void CDXMidi::CollectGarbage()
{

  if ( m_pLoader )
  {
    m_pLoader->CollectGarbage();
  }

}



void CDXMidi::Stop()
{

  if ( m_pPerformance )
  {
    m_pPerformance->Stop( NULL, NULL, 0, 0 );
  }
  m_bPaused = false;

}



HRESULT CDXMidi::CreateSegmentFromFile( CMusicSegment** ppSegment, 
                                        const char* strFileName, 
                                        BOOL bDownloadNow,
                                        BOOL bIsMidiFile )
{

  HRESULT               hr;
  IDirectMusicSegment8* pSegment = NULL;


  // DMusic only takes wide strings
  WCHAR wstrFileName[MAX_PATH];

  MultiByteToWideChar( CP_ACP, 0, strFileName, -1, wstrFileName, 256 );

  if ( FAILED( hr = m_pLoader->LoadObjectFromFile( CLSID_DirectMusicSegment,
                                                   IID_IDirectMusicSegment8,
                                                   wstrFileName,
                                                   (LPVOID*)&pSegment ) ) )
  {
    if ( hr == DMUS_E_LOADER_FAILEDOPEN )
    {
      return hr;
    }
    return hr;
  }

  *ppSegment = new CMusicSegment( m_pPerformance, m_pLoader, pSegment );

  if ( !*ppSegment )
  {
    return E_OUTOFMEMORY;
  }

  if ( bIsMidiFile )
  {
    if ( FAILED( hr = pSegment->SetParam( GUID_StandardMIDIFile, 
                                          0xFFFFFFFF, 0, 0, NULL ) ) )
    {
      return hr;
    }
  }

  if ( bDownloadNow )
  {
    if ( FAILED( hr = (*ppSegment)->Download() ) )
    {
      return hr;
    }
  }

  return S_OK;

}



HRESULT CDXMidi::CreateSegmentFromResource( CMusicSegment** ppSegment, 
                                            TCHAR* strResource,
                                            TCHAR* strResourceType,
                                            BOOL bDownloadNow,
                                            BOOL bIsMidiFile )
{

  HRESULT               hr;
  IDirectMusicSegment8* pSegment      = NULL;
  HRSRC                 hres          = NULL;
  void*                 pMem          = NULL;
  DWORD                 dwSize        = 0;
  DMUS_OBJECTDESC       objdesc;


  // Find the resource
  hres = FindResource( NULL, strResource, strResourceType );
  if ( NULL == hres ) 
  {
    return E_FAIL;
  }

  // Load the resource
  pMem = (void*)LoadResource( NULL, hres );
  if ( NULL == pMem ) 
  {
    return E_FAIL;
  }

  // Store the size of the resource
  dwSize = SizeofResource( NULL, hres ); 
  
  // Set up our object description 
  ZeroMemory( &objdesc, sizeof( DMUS_OBJECTDESC ) );
  objdesc.dwSize      = sizeof( DMUS_OBJECTDESC );
  objdesc.dwValidData = DMUS_OBJ_MEMORY | DMUS_OBJ_CLASS;
  objdesc.guidClass   = CLSID_DirectMusicSegment;
  objdesc.llMemLength = (LONGLONG)dwSize;
  objdesc.pbMemData   = (BYTE*)pMem;
  
  if ( FAILED ( hr = m_pLoader->GetObject( &objdesc,
                                           IID_IDirectMusicSegment8,
                                           (void**)&pSegment ) ) )
  {
    if ( hr == DMUS_E_LOADER_FAILEDOPEN )
    {
      return hr;
    }
    return hr;
  }

  *ppSegment = new CMusicSegment( m_pPerformance, m_pLoader, pSegment );
  if ( NULL == *ppSegment )
  {
    return E_OUTOFMEMORY;
  }

  if ( bIsMidiFile )
  {
    // Do this to make sure that the default General MIDI set 
    // is connected appropriately to the MIDI file and 
    // all instruments sound correct.                  
    if ( FAILED( hr = pSegment->SetParam( GUID_StandardMIDIFile, 
                                         0xFFFFFFFF, 0, 0, NULL ) ) )
    {
      return hr;
    }
  }

  if ( bDownloadNow )
  {
    // The segment needs to be download first before playing.  
    // However, some apps may want to wait before calling this 
    // to because the download allocates memory for the 
    // instruments. The more instruments currently downloaded, 
    // the more memory is in use by the synthesizer.
    if ( FAILED( hr = (*ppSegment)->Download() ) )
    {
      return hr;
    }
  }

  return S_OK;

}



HRESULT CDXMidi::CreateChordMapFromFile( IDirectMusicChordMap8** ppChordMap, 
                                         TCHAR* strFileName )
{

  // DMusic only takes wide strings
  WCHAR wstrFileName[MAX_PATH];

  MultiByteToWideChar( CP_ACP, 0, strFileName, -1, wstrFileName, 256 );

  return m_pLoader->LoadObjectFromFile( CLSID_DirectMusicChordMap,
                                        IID_IDirectMusicChordMap8,
                                        wstrFileName, (LPVOID*)ppChordMap );

}



HRESULT CDXMidi::CreateStyleFromFile( IDirectMusicStyle8** ppStyle, 
                                      TCHAR* strFileName )
{

  // DMusic only takes wide strings
  WCHAR wstrFileName[MAX_PATH];

  MultiByteToWideChar( CP_ACP, 0, strFileName, -1, wstrFileName, 256 );

  return m_pLoader->LoadObjectFromFile( CLSID_DirectMusicStyle,
                                        IID_IDirectMusicStyle8,
                                        wstrFileName, (LPVOID*)ppStyle );

}



HRESULT CDXMidi::GetMotifFromStyle( IDirectMusicSegment8** ppMotif8, 
                                    TCHAR* strStyle, TCHAR* strMotif )
{       

  UNREFERENCED_PARAMETER( strMotif );

  HRESULT              hr;
  IDirectMusicStyle8*  pStyle = NULL;
  IDirectMusicSegment* pMotif = NULL;

  if ( FAILED( hr = CreateStyleFromFile( &pStyle, strStyle ) ) )
  {
    return hr;
  }

  if ( pStyle )
  {
    // DMusic only takes wide strings
    WCHAR wstrMotif[MAX_PATH];

    MultiByteToWideChar( CP_ACP, 0, strStyle, -1, wstrMotif, 256 );

    hr = pStyle->GetMotif( wstrMotif, &pMotif );

    if ( pStyle )
    {
      pStyle->Release();
      pStyle = NULL;
    }

    if ( FAILED( hr ) )
    {
      return hr;
    }
    pMotif->QueryInterface( IID_IDirectMusicSegment8, (LPVOID*)ppMotif8 );
  }

  return S_OK;

}



void CDXMidi::Set3DParameters( float fDistanceFactor, float fDopplerFactor, float fRolloffFactor )
{

  m_dsListenerParams.flDistanceFactor = fDistanceFactor;
  m_dsListenerParams.flDopplerFactor = fDopplerFactor;
  m_dsListenerParams.flRolloffFactor = fRolloffFactor;

  if ( m_pDSListener )
  {
    m_pDSListener->SetAllParameters( &m_dsListenerParams, DS3D_IMMEDIATE );
  }

}



CMusicSegment::CMusicSegment( IDirectMusicPerformance8* pPerformance, 
                              IDirectMusicLoader8*      pLoader,
                              IDirectMusicSegment8*     pSegment )
{

  m_pPerformance          = pPerformance;
  m_pLoader               = pLoader;
  m_pSegment              = pSegment;
  m_pEmbeddedAudioPath    = NULL;
  m_bDownloaded           = FALSE;
  
  // Try to pull out an audio path from the segment itself if there is one.
  // This embedded audio path will be used instead of the default
  // audio path if the app doesn't wish to use an overriding audio path.
  IUnknown* pConfig = NULL;
  if ( SUCCEEDED( m_pSegment->GetAudioPathConfig( &pConfig ) ) )
  {
    m_pPerformance->CreateAudioPath( pConfig, TRUE, &m_pEmbeddedAudioPath );
    if ( pConfig )
    {
      pConfig->Release();
      pConfig = NULL;
    }
  } 

}



CMusicSegment::~CMusicSegment()
{

  if ( m_pSegment )
  {
    // Tell the loader that this object should now be released
    if ( m_pLoader )
    {
      m_pLoader->ReleaseObjectByUnknown( m_pSegment );
    }

    if ( m_bDownloaded )
    {
      if ( m_pEmbeddedAudioPath )
      {
        m_pSegment->Unload( m_pEmbeddedAudioPath );
      }
      else
      {
        m_pSegment->Unload( m_pPerformance );
      }
    }

    if ( m_pEmbeddedAudioPath )
    {
      m_pEmbeddedAudioPath->Release();
      m_pEmbeddedAudioPath = NULL;
    }
    if ( m_pSegment )
    {
      m_pSegment->Release();
      m_pSegment = NULL;
    }
  }

  m_pPerformance = NULL;

}



HRESULT CMusicSegment::Play( DWORD dwFlags, IDirectMusicAudioPath8* pAudioPath )
{

  if ( ( m_pSegment == NULL )
  ||   ( m_pPerformance == NULL ) )
  {
    return CO_E_NOTINITIALIZED;
  }

  if ( !m_bDownloaded )
  {
    return E_FAIL;
  }

  // If an audio path was passed in then use it, otherwise
  // use the embedded audio path if there was one.
  if ( ( pAudioPath == NULL )
  &&   ( m_pEmbeddedAudioPath != NULL ) )
  {
    pAudioPath = m_pEmbeddedAudioPath;
  }
      
  // If pAudioPath is NULL then this plays on the default audio path.
  return m_pPerformance->PlaySegmentEx( m_pSegment, 0, NULL, dwFlags, 
                                        0, 0, NULL, pAudioPath );

}



HRESULT CMusicSegment::Resume( MUSIC_TIME& musTime )
{

  if ( ( m_pSegment == NULL )
  ||   ( m_pPerformance == NULL ) )
  {
    return CO_E_NOTINITIALIZED;
  }

  if ( !m_bDownloaded )
  {
    return E_FAIL;
  }

  // If an audio path was passed in then use it, otherwise
  // use the embedded audio path if there was one.
  IDirectMusicAudioPath*    pAudioPath = NULL;

  if ( m_pEmbeddedAudioPath != NULL )
  {
    pAudioPath = m_pEmbeddedAudioPath;
  }

  m_pSegment->SetStartPoint( musTime );
      
  // If pAudioPath is NULL then this plays on the default audio path.
  return m_pPerformance->PlaySegment( m_pSegment, 0, 0, NULL );

}



HRESULT CMusicSegment::Download( IDirectMusicAudioPath8* pAudioPath )
{

  HRESULT hr;
  
  if ( m_pSegment == NULL )
  {
    return CO_E_NOTINITIALIZED;
  }

  // If no audio path was passed in, then download
  // to the embedded audio path if it exists 
  // else download to the performance
  if ( pAudioPath == NULL )
  {
    if ( m_pEmbeddedAudioPath )
    {
      hr = m_pSegment->Download( m_pEmbeddedAudioPath );
    }
    else    
    {
      hr = m_pSegment->Download( m_pPerformance );
    }
  }
  else
  {
    hr = m_pSegment->Download( pAudioPath );
  }
  
  if ( SUCCEEDED( hr ) )
  {
    m_bDownloaded = TRUE;
  }
      
  return hr;

}



HRESULT CMusicSegment::Unload( IDirectMusicAudioPath8* pAudioPath )
{

  HRESULT hr;
  
  if ( m_pSegment == NULL )
  {
    return CO_E_NOTINITIALIZED;
  }

  // If no audio path was passed in, then unload 
  // from the embedded audio path if it exists 
  // else unload from the performance
  if ( pAudioPath == NULL )
  {
    if ( m_pEmbeddedAudioPath )
    {
      hr = m_pSegment->Unload( m_pEmbeddedAudioPath );
    }
    else    
    {
      hr = m_pSegment->Unload( m_pPerformance );
    }
  }
  else
  {
    hr = m_pSegment->Unload( pAudioPath );
  }
      
  if ( SUCCEEDED( hr ) )
  {
    m_bDownloaded = FALSE;
  }

  return hr;

}



bool CDXMidi::IsPlaying()
{

  if ( ( m_pSegment == NULL )
  ||   ( m_pPerformance == NULL ) )
  {
    return false;
  }

  return ( m_pPerformance->IsPlaying( m_pSegment->GetSegment(), NULL ) == S_OK );

}



HRESULT CMusicSegment::Stop( DWORD dwFlags )
{

  if ( ( m_pSegment == NULL )
  ||   ( m_pPerformance == NULL ) )
  {
    return CO_E_NOTINITIALIZED;
  }

  return m_pPerformance->Stop( m_pSegment, NULL, 0, dwFlags );;

}



HRESULT CMusicSegment::SetRepeats( DWORD dwRepeats )
{

  if ( m_pSegment == NULL )
  {
    return CO_E_NOTINITIALIZED;
  }

  return m_pSegment->SetRepeats( dwRepeats );

}



HRESULT CMusicSegment::GetStyle( IDirectMusicStyle8** ppStyle, DWORD dwStyleIndex )
{

  // Get the Style from the Segment by calling the Segment's GetData() with
  // the data type GUID_StyleTrackStyle. 0xffffffff indicates to look at
  // tracks in all TrackGroups in the segment. The first 0 indicates to
  // retrieve the Style from the first Track  in the indicated TrackGroup.
  // The second 0 indicates to retrieve the Style from the beginning of the
  // segment, i.e. time 0 in Segment time. If this Segment was loaded from a
  // section file, there is only one Style and it is at time 0.
  return m_pSegment->GetParam( GUID_IDirectMusicStyle, 0xffffffff, dwStyleIndex, 
                                0, NULL, (VOID*)ppStyle );

}



bool CDXMidi::LoadMusic( const char* strFileName )
{

  // Free any previous segment, and make a new one
  if ( m_pSegment )
  {
    delete m_pSegment;
    m_pSegment = NULL;
  }

  // Have the loader collect any garbage now that the old 
  // segment has been released
  CollectGarbage();

  // For DirectMusic must know if the file is a standard MIDI file or not
  // in order to load the correct instruments.
  BOOL bMidiFile = FALSE;
  if ( ( strstr( strFileName, ".mid" ) != NULL )
  ||   ( strstr( strFileName, ".rmi" ) != NULL ) )
  {
    bMidiFile = TRUE;
  }

  BOOL bWavFile = FALSE;
  if ( strstr( strFileName, ".wav" ) != NULL )
  {
    bWavFile = TRUE;
  }        

  // Load the file into a DirectMusic segment 
  if ( FAILED( CreateSegmentFromFile( &m_pSegment, strFileName, TRUE, bMidiFile ) ) )
  {
    // Not a critical failure, so just update the status
    return false; 
  }

  return true;

}



bool CDXMidi::LoadMusic( IIOStream& Stream )
{

  return false;

}



bool CDXMidi::Play( bool bLooped )
{

  if ( m_pSegment == NULL )
  {
    return false;
  }

  HRESULT   hr;

  if ( bLooped )
  {
    // Set the segment to repeat many times
    if ( FAILED( hr = m_pSegment->SetRepeats( DMUS_SEG_REPEAT_INFINITE ) ) )
    {
      return false;
    }
  }
  else
  {
    // Set the segment to not repeat
    if ( FAILED( hr = m_pSegment->SetRepeats( 0 ) ) )
    {
      return false;
    }
  }

  // Play the segment and wait. The DMUS_SEGF_BEAT indicates to play on the 
  // next beat if there is a segment currently playing. 
  if ( FAILED( hr = m_pSegment->Play( DMUS_SEGF_BEAT ) ) )
  {
    return false;
  }

  return true;

}



bool CDXMidi::SetVolume( int nVolume )
{

  static int                   iVolumeRange[101] = { -10000, -6644, -5644, -5059, -4644,
                                                      -4322, -4059, -3837, -3644, -3474,
                                                      -3322, -3184, -3059, -2943, -2837,
                                                      -2737, -2644, -2556, -2474, -2396,
                                                      -2322, -2252, -2184, -2120, -2059,
                                                      -2000, -1943, -1889, -1837, -1786,
                                                      -1737, -1690, -1644, -1599, -1556,
                                                      -1515, -1474, -1434, -1396, -1358,
                                                      -1322, -1286, -1252, -1218, -1184,
                                                      -1152, -1120, -1089, -1059, -1029,
                                                      -1000,  -971,  -943,  -916,  -889,
                                                        -862,  -837,  -811,  -786,  -761,
                                                        -737,  -713,  -690,  -667,  -644,
                                                        -621,  -599,  -578,  -556,  -535,
                                                        -515,  -494,  -474,  -454,  -434,
                                                        -415,  -396,  -377,  -358,  -340,
                                                        -322,  -304,  -286,  -269,  -252,
                                                        -234,  -218,  -201,  -184,  -168,
                                                        -152,  -136,  -120,  -105,   -89,
                                                        -74,   -59,   -44,   -29,   -14,
                                                        0 };

  if ( nVolume < 0 )
  {
    nVolume = 0;
  }
  if ( nVolume > 100 )
  {
    nVolume = 100;
  }

  //nVolume = DMUS_VOLUME_MIN + nVolume * ( DMUS_VOLUME_MAX - DMUS_VOLUME_MIN ) / 100;
  nVolume = DMUS_VOLUME_MIN + ( iVolumeRange[nVolume] + 10000 ) * ( DMUS_VOLUME_MAX - DMUS_VOLUME_MIN ) / ( 10000 ); 

  if ( m_pPerformance )
  {
    m_pPerformance->SetGlobalParam( GUID_PerfMasterVolume, (void*)&nVolume, sizeof(long) );  
  }

  return true;

}



int CDXMidi::Volume()
{

  int nVolume = 0;
  if ( m_pPerformance )
  {
    m_pPerformance->GetGlobalParam( GUID_PerfMasterVolume, (void*)&nVolume, sizeof(long) );  
  }
  return nVolume;
  
}



bool CDXMidi::IsInitialized()
{

  return ( m_pPerformance != NULL );

}



bool CDXMidi::Pause()
{

  if ( ( m_pSegment == NULL )
  ||   ( m_bPaused ) )
  {
    return false;
  }

  IDirectMusicSegment*    pdmSeg = m_pSegment->GetSegment();

  m_pPerformance->GetTime( NULL, &m_musPausedTime );

  MUSIC_TIME    mtLength;
  pdmSeg->GetLength( &mtLength );
  
  /*
  if ( FAILED( pdmSeg->GetStartPoint( &m_musPausedTime ) ) )
  {
    return false;
  }
  */
  m_pSegment->Stop();

  m_bPaused = true;

  return true;

}



bool CDXMidi::Resume()
{

  if ( ( m_pSegment == NULL )
  ||   ( !m_bPaused ) )
  {
    return false;
  }

  m_bPaused = false;
  return SUCCEEDED( m_pSegment->Resume( m_musPausedTime ) );

}