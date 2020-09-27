#include "OggXAudio2.h"

#include <debug/debugclient.h>

#include  <process.h>           //for threading support



namespace Xtreme
{

  const int BUFFER_HALF_SIZE = 25 * 1024;     //15 kb, which is roughly one second of OGG sound



  XOggXAudio2::XOggXAudio2() :
    m_PlayThread( 0 ),
    m_PlayThreadActive( false ),
    m_StopPlaybackEvent( 0 ),
    m_PlaybackDone( false ),
    m_Paused( false ),
    m_FileLoaded( false ),
    m_Volume( 100 ),
    m_PlayBufferID( 0 )
  {
#if ( ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) ) \
||      ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE ) ) )
    InitializeCriticalSectionEx( &m_CriticalSection, 0, 0 );
#else
    InitializeCriticalSection( &m_CriticalSection );
#endif
  }




  bool XOggXAudio2::Allocate()
  {
    if ( !m_XAudio.IsInitialized() )
    {
      return false;
    }


    //----- open OGG file

    //get a handle to the file
    if ( m_Filename.empty() )
    {
      return false;
    }

    FILE* fileHandle = NULL;
    if ( fopen_s( &fileHandle, GetFileName().c_str(), "rb" ) != 0 )
    {
      return false;
    }

    //test to make sure that the file was opened succesfully

    if ( !fileHandle ) return false;

    //open the file as an OGG file (allocates internal OGG buffers)

    if ( ( ov_open( fileHandle, &m_VorbisFile, NULL, 0 ) ) != 0 )
    {
      //close the file

      fclose( fileHandle );
      //return error

      return false;
    }


    //----- Get OGG info

    //get information about OGG file

    vorbis_info* vorbisInfo = ov_info( &m_VorbisFile, -1 );

    //----- setup buffer to hold OGG

    //get a wave format structure, which we will use later to create the DirectSound buffer description
    WAVEFORMATEX      waveFormat;
    //clear memory

    memset( &waveFormat, 0, sizeof( waveFormat ) );
    //set up wave format structure

    waveFormat.cbSize = sizeof( waveFormat );                 //how big this structure is

    waveFormat.nChannels = vorbisInfo->channels;                 //how many channelse the OGG contains

    waveFormat.wBitsPerSample = 16;                         //always 16 in OGG

    waveFormat.nSamplesPerSec = vorbisInfo->rate;                   //sampling rate (11 Khz, 22 Khz, 44 KHz, etc.)

    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nChannels * 2; //average bytes per second

    waveFormat.nBlockAlign = 2 * waveFormat.nChannels;               //what block boundaries exist

    waveFormat.wFormatTag = 1;                          //always 1 in OGG



    //----- prepare DirectSound buffer description

    //get a buffer description
    m_PlayBufferID = m_XAudio.CreateModifyableBuffer( waveFormat.nChannels, waveFormat.nSamplesPerSec, waveFormat.nBlockAlign, waveFormat.wBitsPerSample, BUFFER_HALF_SIZE * 2, false, XSound::ST_MUSIC );
    //dsBufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_STICKYFOCUS;   //buffer must support notifications

    //----- create notification event object used to signal when playback should stop

#if ( ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) ) \
||      ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE ) ) )
    m_StopPlaybackEvent = CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
#else
    m_StopPlaybackEvent = CreateEvent( NULL, false, false, NULL );
#endif

    Fill( true );

    return true;
  }




  XOggXAudio2::~XOggXAudio2()
  {
    if ( IsPlaying() )
    {
      Stop();
    }
    DeleteCriticalSection( &m_CriticalSection );
  }



  void XOggXAudio2::Cleanup()
  {
    if ( m_FileLoaded )
    {
      ov_clear( &m_VorbisFile );
      m_FileLoaded = false;
    }

    if ( m_StopPlaybackEvent )
    {
      CloseHandle( m_StopPlaybackEvent );
      m_StopPlaybackEvent = NULL;
    }

    m_XAudio.ReleaseWave( m_PlayBufferID );
    m_PlayBufferID = 0;
  }



  bool XOggXAudio2::Fill( const bool FirstHalf )
  {
    LPVOID  pFirstSegment;              //holds a pointer to the first segment that we lock

    GR::u32 nFirstSegmentSize = 0;          //holds how big the first segment is

    LPVOID  pSecondSegment;             //holds a pointer to the second segment that we lock

    GR::u32 nSecondSegmentSize = 0;         //holds how big the second segment is


    if ( !m_XAudio.LockModifyableBuffer( m_PlayBufferID, 
                                         FirstHalf ? 0 : BUFFER_HALF_SIZE,
                                         BUFFER_HALF_SIZE,
                                         &pFirstSegment,               //pointer that will receive the locked segment start address
                                         &nFirstSegmentSize,           //will return how big the first segment is (should always be BUFFER_HALF_SIZE)
                                         &pSecondSegment,              //pointer that will receive the second locked segment start address (in case of wrapping)
                                         &nSecondSegmentSize ) )       //how big a chunk we wrapped with (in case of wrapping)
    {
      dh::Log( "OggPlayer: LockModifyableBuffer failed" );
      return false;
    }

    //----- debug safety: we should always have locked a complete segment of the size we requested
    if ( nFirstSegmentSize < BUFFER_HALF_SIZE )
    {
      dh::Log( "OggPlayer: Lock returned wrong size!" );
      m_XAudio.UnlockModifyableBuffer( m_PlayBufferID );
      return false;
    }

    //----- decode OGG file into buffer

    unsigned int nBytesReadSoFar = 0; //keep track of how many bytes we have read so far

    long nBytesReadThisTime = 1; //keep track of how many bytes we read per ov_read invokation (1 to ensure that while loop is entered below)

    int nBitStream = 0; //used to specify logical bitstream 0

    //decode vorbis file into buffer half (continue as long as the buffer hasn't been filled with something (=sound/silence)
    while ( nBytesReadSoFar < nFirstSegmentSize )
    {
      //decode
      nBytesReadThisTime = ov_read(
        &m_VorbisFile,                //what file to read from
        (char*)pFirstSegment + nBytesReadSoFar,   //where to put the decoded data
        nFirstSegmentSize - nBytesReadSoFar,     //how much data to read
        0,                      //0 specifies little endian decoding mode
        2,                      //2 specifies 16-bit samples
        1,                      //1 specifies signed data
        &nBitStream
      );

      //new position corresponds to the amount of data we just read

      nBytesReadSoFar += nBytesReadThisTime;


      //----- do special processing if we have reached end of the OGG file
      if ( nBytesReadThisTime == 0 )
      {
        //----- if looping we fill start of OGG, otherwise fill with silence
        if ( m_Looping )
        {
          //seek back to beginning of file
          ov_time_seek( &m_VorbisFile, 0 );
        }
        else
        {
          //fill with silence
          for ( unsigned int i = nBytesReadSoFar; i < nFirstSegmentSize; i++ )
          {
            //silence = 0 in 16 bit sampled data (which OGG always is)
            *( (char*)pFirstSegment + i ) = 0;
          }

          //signal that playback is over
          m_PlaybackDone = true;

          //and exit the reader loop
          nBytesReadSoFar = nFirstSegmentSize;
        }
      }
    }


    //----- unlock buffer
    m_XAudio.UnlockModifyableBuffer( m_PlayBufferID );
    return true;
  }




  bool XOggXAudio2::IsPlaying()
  {
    return GetPlayThreadActive();
  }



  void XOggXAudio2::Stop()
  {
    if ( !IsPlaying() )
    {
      return;
    }

    m_Paused = false;


    //----- Signal the play thread to stop
    SetEvent( m_StopPlaybackEvent );

    //wait for playing thread to exit
    if ( WaitForSingleObjectEx( m_PlayThread, 500, FALSE ) == WAIT_ABANDONED )
    //if ( WaitForSingleObject( m_PlayThread, 500 ) == WAIT_ABANDONED )
    {
      //the thread hasn't terminated as expected. kill it

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      TerminateThread( m_hPlayThread, 1 );
#endif

      //not playing any more

      SetPlayThreadActive( false );

      //since playing thread has not cleaned up, this thread will have to

      Cleanup();
      //TODO: You should report this error here somehow
    }

    //----- store that we are not playing any longer
    SetPlayThreadActive( false );
  }



  bool XOggXAudio2::Play( const bool looping )
  {
    if ( IsPlaying() )
    {
      Stop();
    }
    Cleanup();

    Allocate();

    SetPlayThreadActive( true );

    m_Looping = looping;

    unsigned int nThreadID = 0;

    m_PlayThread = (HANDLE)CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)XOggXAudio2::PlayingThread, this, 0, (LPDWORD)&nThreadID );

    return ( m_PlayThread != NULL );
  }



  GR::String XOggXAudio2::GetFileName()
  {
    EnterCriticalSection( &m_CriticalSection );
    GR::String filename = m_Filename;
    LeaveCriticalSection( &m_CriticalSection );

    return filename;
  }




  unsigned int XOggXAudio2::PlayingThread( LPVOID lpParam )
  {
    XOggXAudio2* oggInstance = static_cast<XOggXAudio2*>( lpParam );

    if ( oggInstance->m_PlayBufferID == 0 )
    {
      return 1;
    }

    bool errorOccured = false;   //assume everything will go ok

    //----- indicate that we are playing
    oggInstance->m_PlaybackDone = false;


    //----- start playing the buffer (looping because we are going to refill the buffer)
    oggInstance->m_XAudio.SetVolume( oggInstance->m_PlayBufferID, oggInstance->m_Volume );

    if ( !oggInstance->m_XAudio.Loop( oggInstance->m_PlayBufferID ) )
    {
      return 0;
    }

    //----- go into loop waiting on notification event objects

    //create tracker of what half we have are playing
    bool playingFirstHalf = true;

    bool  continuePlaying = true;    //used to keep track of when to stop the while loop

    bool  playbackDoneProcessed = false; //used ot only process m_bPlaybackDone once

    int   nStopAtNextHalf = 0;      //0 signals "do not stop", 1 signals "stop at first half", 2 signals "stop at second half"


    //enter refill loop
    DWORD   prevPos = BUFFER_HALF_SIZE * 2;

    while ( ( continuePlaying )
    &&      ( !errorOccured ) )
    {
      //Sleep( 30 );

      int   start = 0;

      //if ( WaitForSingleObjectEx( oggInstance->m_StopPlaybackEvent, 0, FALSE ) == WAIT_OBJECT_0 )
      if ( WaitForSingleObjectEx( oggInstance->m_StopPlaybackEvent, 30, FALSE ) == WAIT_OBJECT_0 )
      //if ( WaitForSingleObject( oggInstance->m_StopPlaybackEvent, 0 ) == WAIT_OBJECT_0 )
      {
        break;
      }

      bool    refillNeeded = false;

      {
        if ( !continuePlaying )
        {
          break;
        }

        DWORD   soundBufferCurrentPosition = oggInstance->m_XAudio.GetPos( oggInstance->m_PlayBufferID );

        DWORD     curFragment = soundBufferCurrentPosition / BUFFER_HALF_SIZE;

        dh::Log( "CurPos %d", soundBufferCurrentPosition );
        dh::Log( "CurFrag %d / PrevFrag %d", curFragment, prevPos / BUFFER_HALF_SIZE );

        if ( curFragment != prevPos / BUFFER_HALF_SIZE )
        {
          prevPos = soundBufferCurrentPosition;
          refillNeeded = true;
          if ( soundBufferCurrentPosition % ( BUFFER_HALF_SIZE * 2 ) < (DWORD)BUFFER_HALF_SIZE )
          {
            start = BUFFER_HALF_SIZE;
          }
          else
          {
            start = 0;
          }
        }
      }

      //----- first half was reached
      if ( refillNeeded )
      {
        dh::Log( "Refill half %d", start );
        if ( start > 0 )
        {
          //check if we should stop playing back

          if ( nStopAtNextHalf == 1 )
          {
            //stop playing
            continuePlaying = false;
            //leave and do not fill the next buffer half
            break;
          }

          //fill second half with sound
          if ( !( oggInstance->Fill( false ) ) )
          {
            errorOccured = true;
          }

          //if the last fill was the final fill, we should stop next time we reach this half (i.e. finish playing whatever audio we do have)
          if ( ( oggInstance->m_PlaybackDone ) 
          &&   ( !playbackDoneProcessed ) )
          {
            //make the while loop stop after playing the next half of the buffer
            nStopAtNextHalf = 1;
            //indicate that we have already processed the playback done flag
            playbackDoneProcessed = true;
          }
        }
        else
        {
          //check if we should stop playing back
          if ( nStopAtNextHalf == 2 )
          {
            //stop playing
            continuePlaying = false;
            break;
          }

          //fill first half with sound
          if ( !( oggInstance->Fill( true ) ) )
          {
            errorOccured = true;
          }

          //if this last fill was the final fill, we should stop next time we reach this half (i.e. finish playing whatever audio we do have)
          if ( ( oggInstance->m_PlaybackDone ) 
          &&   ( !playbackDoneProcessed ) )
          {
            //make the while loop stop after playing the next half of the buffer
            nStopAtNextHalf = 2;
            //indicate that we have already processed the playback done flag
            playbackDoneProcessed = true;
          }
        }
      }
    }
    oggInstance->m_XAudio.Stop( oggInstance->m_PlayBufferID );

    oggInstance->Cleanup();

    oggInstance->SetPlayThreadActive( false );

    return ( errorOccured ? 1 : 0 );
  }



  bool XOggXAudio2::GetPlayThreadActive()
  {
    EnterCriticalSection( &m_CriticalSection );
    bool bActive = m_PlayThreadActive;
    LeaveCriticalSection( &m_CriticalSection );

    return bActive;
  }



  void XOggXAudio2::SetPlayThreadActive( bool Active )
  {
    EnterCriticalSection( &m_CriticalSection );
    m_PlayThreadActive = Active;
    LeaveCriticalSection( &m_CriticalSection );
  }



  bool XOggXAudio2::Initialize( GR::IEnvironment& Environment )
  {
    if ( m_XAudio.IsInitialized() )
    {
      return false;
    }
    if ( !m_XAudio.Initialize( Environment, 1 ) )
    {
      dh::Log( "Ogg:Init fail a" );
      return false;
    }
    return true;
  }



  bool XOggXAudio2::Release()
  {
    Stop();
    m_XAudio.Release();
    return true;
  }



  bool XOggXAudio2::IsInitialized()
  {
    return m_XAudio.IsInitialized();
  }



  int XOggXAudio2::Volume()
  {
    return m_Volume;
  }



  bool XOggXAudio2::SetVolume( int Volume )
  {
    return m_XAudio.SetMasterVolume( Volume, XSound::ST_MUSIC );
  }




  bool XOggXAudio2::LoadMusic( const GR::Char* szFileName )
  {
    Stop();
    Cleanup();

    m_Filename = GR::Convert::ToUTF8( szFileName );

    if ( !Allocate() )
    {
      return false;
    }
    m_FileLoaded = true;
    return true;
  }



  bool XOggXAudio2::LoadMusic( IIOStream& Stream )
  {
    return false;
  }



  bool XOggXAudio2::Resume()
  {
    if ( !m_Paused )
    {
      return false;
    }
    m_XAudio.Resume( 0 );

    m_Paused = false;
    return true;
  }



  bool XOggXAudio2::Pause()
  {
    if ( m_Paused )
    {
      return true;
    }
    if ( !IsPlaying() )
    {
      return false;
    }
    m_XAudio.StopChannel( 0 );
    m_Paused = true;

    return true;
  }

}