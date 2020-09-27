/*----------------------------------------------------------------------------+
 | Programmname       :Sound Routinen                                         |
 +----------------------------------------------------------------------------+
 | Autor              :Georg Rottensteiner                                    |
 | Datum              :30.06.99                                               |
 | Version            :1.0                                                    |
 +----------------------------------------------------------------------------*/


 
/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Sound/MP3.h>

#include <Misc/Misc.h>



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CMP3File::CMP3File()
{

  m_pGraphBuilder = NULL;
  m_pMediaControl = NULL;
  m_pMediaSeeking = NULL;
  m_pBaseFilter   = NULL;
  m_pBasicAudio   = NULL;
  m_bInitialized  = FALSE;

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CMP3File::~CMP3File()
{

  Close();

}



/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CMP3File::Open( const char *szFileName )
{

  HRESULT hr;


  if ( m_bInitialized )
  {
    return FALSE;
  }

  // Initialize COM
  if ( FAILED ( hr = CoInitialize( NULL ) ) )
  {
    return FALSE;
  }

  // Create DirectShow Graph
  if ( FAILED ( hr = CoCreateInstance( CLSID_FilterGraph, 
                                       NULL,
                                       CLSCTX_INPROC, 
                                       IID_IGraphBuilder,
                                       reinterpret_cast<void **>( &m_pGraphBuilder ) ) ) )
  {
    return FALSE;
  }

  // Get the IMediaControl Interface
  if ( FAILED (m_pGraphBuilder->QueryInterface( IID_IMediaControl,
                                                reinterpret_cast<void **>( &m_pMediaControl ) ) ) )
  {
    return FALSE;
  }

  // Get the IMediaControl Interface
  if ( FAILED (m_pGraphBuilder->QueryInterface( IID_IMediaSeeking,
                                                reinterpret_cast<void **>( &m_pMediaSeeking ) ) ) )
  {
    return FALSE;
  }

  DWORD   dwAttr = GetFileAttributes( szFileName );

  if ( dwAttr == (DWORD)-1 )
  {
    return FALSE;
  }


  WCHAR     wFileName[MAX_PATH];


  MultiByteToWideChar( CP_ACP, 0, szFileName, -1, wFileName, MAX_PATH );

  m_strFileName = szFileName;
  hr = m_pGraphBuilder->AddSourceFilter( wFileName, wFileName, &m_pBaseFilter );

  if ( !SUCCEEDED( hr ) )
  {
    return FALSE;
  }

  if ( !SUCCEEDED( hr = m_pGraphBuilder->QueryInterface( IID_IBasicAudio,
                                        reinterpret_cast<void **>( &m_pBasicAudio ) ) ) )
  {
  }

  IPin      *pPin = NULL;


  hr = m_pBaseFilter->FindPin( L"Output", &pPin );  

  if ( SUCCEEDED( hr ) ) 
  {
    hr = m_pMediaControl->Stop();
  }

  // We have the new ouput pin. Render it
  if ( SUCCEEDED( hr ) ) 
  {
    hr = m_pGraphBuilder->Render( pPin );
  }

  if ( pPin )
  {
    pPin->Release();
    pPin = NULL;
  }

  m_bInitialized = TRUE;
  return TRUE;

}



/*-Close----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMP3File::Close()
{

  Stop();
  if ( m_pGraphBuilder != NULL )
  {
    m_pGraphBuilder->RemoveFilter( m_pBaseFilter );
  }

  // Release all remaining pointers
  SafeRelease( m_pBaseFilter );
  SafeRelease( m_pBasicAudio );
  SafeRelease( m_pMediaSeeking );
  SafeRelease( m_pMediaControl );
  SafeRelease( m_pGraphBuilder );

  m_bInitialized = FALSE;

}



/*-Stop-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMP3File::Stop()
{

  if ( m_pMediaControl )
  {
    m_pMediaControl->Stop();
  }

}



/*-Play-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMP3File::Play()
{

  if ( m_pMediaSeeking == NULL )
  {
    return;
  }

  HRESULT   hr;


  // Re-seek the graph to the beginning
  LONGLONG llPos = 0;
  hr = m_pMediaSeeking->SetPositions( &llPos, AM_SEEKING_AbsolutePositioning,
                                      &llPos, AM_SEEKING_NoPositioning );

  // Start the graph
  if ( SUCCEEDED( hr ) ) 
  {
    hr = m_pMediaControl->Run();
  }


}



/*-SetVolume------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMP3File::SetVolume( DWORD dwVolume )
{

  if ( m_pBasicAudio == NULL )
  {
    return;
  }
  if ( dwVolume > 100 )
  {
    dwVolume = 100;
  }

  int                   iVolumeRange[101] = { -10000, -6644, -5644, -5059, -4644,
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

  m_pBasicAudio->put_Volume( iVolumeRange[dwVolume] );

}



/*-SetPan---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMP3File::SetPan( int iPan )
{

  if ( m_pBasicAudio == NULL )
  {
    return;
  }
  if ( iPan < -100 )
  {
    iPan = -100;
  }
  if ( iPan > 100 )
  {
    iPan = 100;
  }
  m_pBasicAudio->put_Balance( iPan );

}

