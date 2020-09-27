#include "XAudio2Sound.h"

#include <debug/DebugService.h>

#include <IO/FileStream.h>

#include <Interface/IValueStorage.h>

#include <WinSys/WinUtils.h>

#include <X3DAUDIO.H>



#pragma comment( lib, "Xaudio2.lib" )



namespace Xtreme
{
  XAudio2Sound::XAudio2Sound() :
    m_pX2Audio( NULL ),
    m_pMasteringVoice( NULL ),
    m_Initialised( false ),
    m_FXMasterVolume( 100 ),
    m_MusicMasterVolume( 100 )
  {
  }



  XAudio2Sound::~XAudio2Sound()
  {
    Release();
  }



  bool XAudio2Sound::Initialize( GR::IEnvironment& Environment, GR::u32 Channels )
  {
    m_pEnvironment = &Environment;
    m_pDebugger = (IDebugService*)m_pEnvironment->Service( "Logger" );
    if ( m_pDebugger == NULL ) 
    {
      Log( "XAudio2Sound::Initialize no Logger service set up!" );
      return false;
    }
    HRESULT hr = 0;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    if ( FAILED( hr ) )
    {
      Log( "Failed to init COM: %#X", hr );
      return false;
    }
    //#if ( _WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/)
    // Workaround for XAudio 2.7 known issue
#ifdef _DEBUG
    //HMODULE mXAudioDLL = LoadLibraryExW( L"XAudioD2_7.DLL", nullptr, 0x00000800 /* LOAD_LIBRARY_SEARCH_SYSTEM32 */ );
    HMODULE mXAudioDLL = LoadLibraryExW( L"XAudio2_9.DLL", nullptr, 0x00000800 /* LOAD_LIBRARY_SEARCH_SYSTEM32 */ );
#else
    HMODULE mXAudioDLL = LoadLibraryExW( L"XAudio2_9.DLL", nullptr, 0x00000800 /* LOAD_LIBRARY_SEARCH_SYSTEM32 */ );
#endif
    if ( !mXAudioDLL )
    {
      Log( "Failed to find XAudio 2.9 DLL" );
      CoUninitialize();
      return 0;
    }
//#endif
#endif

    GR::u32   flags = 0;
#if (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif
    if ( FAILED( hr = XAudio2Create( &m_pX2Audio, flags, XAUDIO2_DEFAULT_PROCESSOR ) ) )
    {
      Log( "XAudio2Create failed with %x", hr );
      return false;
    }

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
    // To see the trace output, you need to view ETW logs for this application:
    //    Go to Control Panel, Administrative Tools, Event Viewer.
    //    View->Show Analytic and Debug Logs.
    //    Applications and Services Logs / Microsoft / Windows / XAudio2. 
    //    Right click on Microsoft Windows XAudio2 debug logging, Properties, then Enable Logging, and hit OK 
    XAUDIO2_DEBUG_CONFIGURATION debug = { 0 };
    debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    debug.BreakMask = XAUDIO2_LOG_ERRORS;
    m_pX2Audio->SetDebugConfiguration( &debug, 0 );
#endif

    if ( Channels <= 0 )
    {
      Channels = 16;
    }
    if ( Channels > 256 )
    {
      Channels = 256;
    }
    m_Channels.resize( Channels );

    if ( FAILED( hr = m_pX2Audio->CreateMasteringVoice( &m_pMasteringVoice ) ) )
    {
      Log( "CreateMasteringVoice failed with %x", hr );
      Release();
      return false;
    }


    m_Initialised = true;

    LoadAssets();
    return true;
  }



  bool XAudio2Sound::Release()
  {
    if ( !m_Initialised )
    {
      return true;
    }
    if ( m_pX2Audio != NULL )
    {
      m_pX2Audio->StopEngine();
    }
    for each ( auto& sound in m_Sounds )
    {
      for each ( auto& voice in sound.second.SoundVoices )
      {
        if ( voice.pSourceVoice != NULL )
        {
          voice.pSourceVoice->Stop();
          voice.pSourceVoice->DestroyVoice();
        }
      }
    }
    m_Sounds.clear();
    if ( m_pMasteringVoice != NULL )
    {
      m_pMasteringVoice->DestroyVoice();
      m_pMasteringVoice = NULL;
    }
    if ( m_pX2Audio != NULL )
    {
      m_pX2Audio->Release();
      m_pX2Audio = NULL;
    }
    m_Initialised = false;
    return true;
  }




  bool XAudio2Sound::IsInitialized()
  {
    return m_Initialised;
  }




  bool XAudio2Sound::SetMasterVolume( GR::u32 Volume, const eSoundType Type )
  {
    if ( !m_Initialised )
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
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.begin() );
    while ( itS != m_Sounds.end() )
    {
      tSoundInfo& soundInfo( itS->second );

      std::list<tX2AudioSound>::iterator    itV( soundInfo.SoundVoices.begin() );
      while ( itV != soundInfo.SoundVoices.end() )
      {
        tX2AudioSound&  sourceVoice( *itV );

        if ( soundInfo.Type == ST_MUSIC )
        {
          ApplyVolume( soundInfo, sourceVoice, m_MusicMasterVolume );
        }
        else
        {
          ApplyVolume( soundInfo, sourceVoice, m_FXMasterVolume );
        }

        ++itV;
      }
      ++itS;
    }
    return true;
  }



  GR::u32 XAudio2Sound::MasterVolume( const eSoundType Type )
  {
    if ( Type == ST_SOUND_EFFECT )
    {
      return m_FXMasterVolume;
    }
    else if ( Type == ST_MUSIC )
    {
      return m_MusicMasterVolume;
    }
    else if ( Type == ST_ALL )
    {
      return m_FXMasterVolume;
    }
    return 0;
  }



  GR::u32 XAudio2Sound::GetFreeID()
  {
    GR::u32       newID = 1;

    while ( m_Sounds.find( newID ) != m_Sounds.end() )
    {
      ++newID;
    }
    return newID;
  }



  GR::u32 XAudio2Sound::LoadWave( const GR::String& Name, bool LoadAs3d, const eSoundType Type )
  {
    return LoadWave( Name.c_str(), LoadAs3d, Type );
  }



  GR::u32 XAudio2Sound::LoadWave( const char* Filename, bool LoadAs3d, const eSoundType Type )
  {
    GR::IO::FileStream      ioIn;

    if ( !ioIn.Open( Filename ) )
    {
      return 0;
    }
    return LoadWave( ioIn, LoadAs3d, Type );
  }



  GR::u32 XAudio2Sound::LoadWave( IIOStream& InStream, bool LoadAs3d, const eSoundType Type )
  {
    unsigned char       ucBuffer[16];

    GR::u32             chunkSize;


    if ( !m_Initialised )
    {
      return 0;
    }
    if ( !InStream.IsGood() )
    {
      return 0;
    }

    GR::u32       newSoundId = GetFreeID();
    tSoundInfo&   soundInfo( m_Sounds[newSoundId] );

    InStream.ReadBlock( ucBuffer, 8 );
    if ( ( ucBuffer[0] != 82 )
    &&   ( ucBuffer[1] != 73 )
    &&   ( ucBuffer[2] != 70 )
    &&   ( ucBuffer[3] != 70 ) )
    {
      // kein RIFF
      InStream.Close();
      return 0;
    }
    InStream.ReadBlock( ucBuffer, 8 );
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
      InStream.Close();
      return 0;
    }
    chunkSize = InStream.ReadU32();
    if ( chunkSize != 16 )
    {
      // Nicht Standardgröße, ich bin verwörrt
      InStream.Close();
      return 0;
    }

    // Da ist jetzt das wichtige Zeugs drin
    InStream.ReadBlock( ucBuffer, 16 );

    InStream.SetPosition( chunkSize - 16, IIOStream::PT_CURRENT );

    memcpy( &soundInfo.WaveFormat, ucBuffer, 16 );

    soundInfo.OriginalFrequency   = soundInfo.WaveFormat.Format.nSamplesPerSec;
    soundInfo.Frequency           = soundInfo.WaveFormat.Format.nSamplesPerSec;
    soundInfo.Type                = Type;

    InStream.ReadBlock( ucBuffer, 4 );
    if ( ( ucBuffer[0] != 100 )
    &&   ( ucBuffer[1] != 97 )
    &&   ( ucBuffer[2] != 116 )
    &&   ( ucBuffer[3] != 97 ) )
    {
      // kein data
      InStream.Close();
      return 0;
    }
    chunkSize = InStream.ReadU32();


    soundInfo.Size = chunkSize;
    soundInfo.SoundData.Resize( chunkSize );
    if ( InStream.ReadBlock( soundInfo.SoundData.Data(), chunkSize ) != chunkSize )
    {
      m_pDebugger->Log( "Sound", "Failed to read %d bytes from wave file", chunkSize );
      m_Sounds.erase( m_Sounds.find( newSoundId ) );
      InStream.Close();
      return 0;
    }
    InStream.Close();
    if ( !CreateSoundVoice( soundInfo ) )
    {
      m_Sounds.erase( m_Sounds.find( newSoundId ) );
      return 0;
    }

    return newSoundId;
  }



  bool XAudio2Sound::CreateSoundVoice( tSoundInfo& SoundInfo )
  {
    tX2AudioSound   x2Audio;

    x2Audio.XA2Buffer.AudioBytes  = (UINT32)SoundInfo.Size;
    x2Audio.XA2Buffer.pAudioData  = (BYTE*)SoundInfo.SoundData.Data();
    x2Audio.XA2Buffer.Flags       = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

    /*
    if ( LoadAs3d )
    {
    dsBufferDesc.dwFlags |= DSBCAPS_CTRL3D;
    }*/

    

    HRESULT     hr;
    if ( FAILED( hr = m_pX2Audio->CreateSourceVoice( &x2Audio.pSourceVoice, (WAVEFORMATEX*)&SoundInfo.WaveFormat ) ) )
    {
      m_pDebugger->Log( "Sound", "Failed to CreateSourceVoice, error %x", hr );
      return false;
    }
    SoundInfo.SoundVoices.push_back( x2Audio );
    return true;
  }



  GR::u32 XAudio2Sound::LoadWaveFromResource( const char* Resource, const char* Type, const eSoundType SoundType )
  {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    auto memStream = Win::Util::MemoryStreamFromResource( NULL, Resource, Type );
    if ( !memStream.IsGood() )
    {
      return 0;
    }
    return LoadWave( memStream, false, SoundType );
#else
    return 0;
#endif
  }



  GR::u32 XAudio2Sound::CreateBufferFromMemory( int Channels, int SamplesPerSec, int BlockAlign,
                                                int BitsPerSample, GR::up* pData, GR::up Size,
                                                bool LoadAs3d, const eSoundType Type )
  {
    if ( !m_Initialised )
    {
      return 0;
    }


    GR::u32 id = GetFreeID();

    tSoundInfo&   soundInfo( m_Sounds[id] );

    soundInfo.SoundData.Set( pData, Size );
    soundInfo.ID                = id;
    soundInfo.Looping           = false;
    soundInfo.Pan               = 0;
    soundInfo.Size              = Size;
    soundInfo.Frequency         = SamplesPerSec;
    soundInfo.OriginalFrequency = soundInfo.Frequency;
    soundInfo.Type              = Type;
    soundInfo.Volume            = m_FXMasterVolume;
    
    // Set up wave format structure.
    memset( &soundInfo.WaveFormat, 0, sizeof( WAVEFORMATEX ) );
    soundInfo.WaveFormat.Format.wFormatTag      = WAVE_FORMAT_PCM;    // Immer
    soundInfo.WaveFormat.Format.nChannels       = Channels;
    soundInfo.WaveFormat.Format.nSamplesPerSec  = SamplesPerSec;
    soundInfo.WaveFormat.Format.nBlockAlign     = BlockAlign;
    soundInfo.WaveFormat.Format.nAvgBytesPerSec = SamplesPerSec * BlockAlign;
    soundInfo.WaveFormat.Format.wBitsPerSample  = BitsPerSample;

    /*
    if ( bLoadAs3d )
    {
      dsBufferDesc.dwFlags |= DSBCAPS_CTRL3D;
    }*/

    if ( !CreateSoundVoice( soundInfo ) )
    {
      return false;
    }
    return id;
  }



  GR::u32 XAudio2Sound::CreateModifyableBuffer( int Channels, int SamplesPerSec, int BlockAlign,
                                                int BitsPerSample, GR::up Size,
                                                bool LoadAs3d, const eSoundType Type )
  {
    ByteBuffer      dummyData( Size );
    return CreateBufferFromMemory( Channels, 
                                   SamplesPerSec, 
                                   BlockAlign,
                                   BitsPerSample,
                                   (GR::up*)dummyData.Data(),
                                   Size,
                                   LoadAs3d,
                                   Type );
  }



  bool XAudio2Sound::LockModifyableBuffer( GR::u32 ID, GR::u32 Start, GR::u32 LockSize,
                                           LPVOID* pBlock1, GR::u32* Block1Size,
                                           LPVOID* pBlock2, GR::u32* Block2Size )
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Trying to lock non existant sound %d", ID );
      return false;
    }

    tSoundInfo& soundInfo( itS->second );

    *pBlock1 = soundInfo.SoundData.Data();
    *Block1Size = (GR::u32)soundInfo.Size;
    return true;
  }



  bool XAudio2Sound::UnlockModifyableBuffer( GR::u32 ID )
  {
    return true;
  }



  bool XAudio2Sound::ReleaseWave( GR::u32 ID )
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Trying to play non existant sound %d", ID );
      return false;
    }
    auto& soundInfo( itS->second );

    std::list<tX2AudioSound>::iterator    it( soundInfo.SoundVoices.begin() );
    while ( it != soundInfo.SoundVoices.end() )
    {
      tX2AudioSound&  sourceVoice( *it );

      sourceVoice.pSourceVoice->Stop();
      sourceVoice.pSourceVoice->DestroyVoice();

      ++it;
    }
    m_Sounds.erase( itS );
    return true;
  }




  GR::u32 XAudio2Sound::FindFreeChannel()
  {
    return 0;
  }



  bool XAudio2Sound::Play( GR::u32 ID, GR::u32 Offset )
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Trying to play non existant sound %d", ID );
      return false;
    }

    tSoundInfo& soundInfo( itS->second );

    int volume = m_FXMasterVolume;
    if ( soundInfo.Type == XSound::ST_MUSIC )
    {
      volume = m_MusicMasterVolume;
    }

    return PlayChannel( ID, -1, Offset, volume );
  }



  bool XAudio2Sound::PlayChannel( GR::u32 ID, GR::u32 Channel, GR::u32 Offset,
                                  GR::u32 Volume, GR::i32 Pan, GR::u32 Frequency, bool Looping )
  {
    HRESULT     hr;

    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Trying to play non existant sound %d", ID );
      return false;
    }

    tSoundInfo& soundInfo( itS->second );
    if ( soundInfo.SoundVoices.empty() )
    {
      m_pDebugger->Log( "Sound", "Trying to access non initialised sound %d", ID );
      return false;
    }

    std::list<tX2AudioSound>::iterator    it( soundInfo.SoundVoices.begin() );
    while ( it != soundInfo.SoundVoices.end() )
    {
      tX2AudioSound&  sourceVoice( *it );
      ++it;

      XAUDIO2_VOICE_STATE   state;
      sourceVoice.pSourceVoice->GetState( &state );
      if ( state.BuffersQueued == 0 )
      {
        // reuse stopped voice
        // 1 - reset
        if ( FAILED( hr = sourceVoice.pSourceVoice->FlushSourceBuffers() ) )
        {
          m_pDebugger->Log( "Sound", "Error when trying to FlushSourceBuffers for sound %d, error code %x", ID, hr );
          return false;
        }
        if ( FAILED( hr = sourceVoice.pSourceVoice->Stop() ) )
        {
          m_pDebugger->Log( "Sound", "Error when trying to stop sound %d, error code %x", ID, hr );
          return false;
        }
        SetupVoiceBuffer( soundInfo, sourceVoice, Offset, Volume, Pan, Frequency, Looping );
        if ( FAILED( hr = sourceVoice.pSourceVoice->SubmitSourceBuffer( &sourceVoice.XA2Buffer ) ) )
        {
          m_pDebugger->Log( "Sound", "Error calling SubmitSourceBuffer when playing sound %d, error code %x", ID, hr );
          return false;
        }
        if ( FAILED( hr = sourceVoice.pSourceVoice->Start( 0 ) ) )
        {
          m_pDebugger->Log( "Sound", "Error when trying to play sound %d, error code %x", ID, hr );
          return false;
        }
        return true;
      }
    }
    // all buffers busy, create a new one
    if ( !CreateSoundVoice( soundInfo ) )
    {
      return false;
    }
    auto& newVoice = soundInfo.SoundVoices.back();

    if ( Looping )
    {
      newVoice.XA2Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }
    else
    {
      newVoice.XA2Buffer.LoopCount = 0;
    }
    if ( FAILED( hr = newVoice.pSourceVoice->SubmitSourceBuffer( &newVoice.XA2Buffer ) ) )
    {
      m_pDebugger->Log( "Sound", "Error calling SubmitSourceBuffer when playing sound %d, error code %x", ID, hr );
      return false;
    }
    ApplyVolume( soundInfo, newVoice, Volume );
    if ( FAILED( hr = newVoice.pSourceVoice->Start( 0 ) ) )
    {
      m_pDebugger->Log( "Sound", "Error when trying to play sound %d, error code %x", ID, hr );
      return false;
    }
    return true;
  }



  void XAudio2Sound::SetupVoiceBuffer( const tSoundInfo& SoundInfo, tX2AudioSound& SourceVoice, GR::u32 Offset, GR::u32 Volume, GR::i32 Pan, GR::u32 Frequency, bool Looping )
  {
    if ( Looping )
    {
      SourceVoice.XA2Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }
    else
    {
      SourceVoice.XA2Buffer.LoopCount = 0;
    }

    SourceVoice.XA2Buffer.PlayBegin = Offset;
    if ( Frequency == 0 )
    {
      SourceVoice.pSourceVoice->SetFrequencyRatio( 1.0f );
    }
    else
    {
      SourceVoice.pSourceVoice->SetFrequencyRatio( (float)Frequency / SoundInfo.OriginalFrequency );
    }
    // PAN
    ApplyPan( SoundInfo, SourceVoice, Pan );

    ApplyVolume( SoundInfo, SourceVoice, Volume );
  }



  void XAudio2Sound::ApplyPan( const tSoundInfo& SoundInfo, tX2AudioSound& SourceVoice, GR::i32 Pan )
  {
    DWORD     channelMask;
    HRESULT   hr;
    if ( FAILED( hr = m_pMasteringVoice->GetChannelMask( &channelMask ) ) )
    {
      m_pDebugger->Log( "Sound", "GetChannelMask failed with error %x", hr );
      return;
    }
    float outputMatrix[8];
    for ( int i = 0; i < 8; i++ )
    {
      outputMatrix[i] = 1.0;
    }

    if ( Pan < -100 )
    {
      Pan = -100;
    }
    if ( Pan > 100 )
    {
      Pan = 100;
    }

    // pan of -1.0 indicates all left speaker, 
    // 1.0 is all right speaker, 0.0 is split between left and right
    float left = 0.5f - Pan * 0.005f;
    float right = 0.5f + Pan * 0.005f;

    switch ( channelMask )
    {
      case SPEAKER_MONO:
        outputMatrix[0] = 1.0;
        break;
      case SPEAKER_STEREO:
      case SPEAKER_2POINT1:
      case SPEAKER_SURROUND:
        outputMatrix[0] = left;
        outputMatrix[1] = right;
        break;
      case SPEAKER_QUAD:
        outputMatrix[0] = outputMatrix[2] = left;
        outputMatrix[1] = outputMatrix[3] = right;
        break;
      case SPEAKER_4POINT1:
        outputMatrix[0] = outputMatrix[3] = left;
        outputMatrix[1] = outputMatrix[4] = right;
        break;
      case SPEAKER_5POINT1:
      case SPEAKER_7POINT1:
      case SPEAKER_5POINT1_SURROUND:
        outputMatrix[0] = outputMatrix[4] = left;
        outputMatrix[1] = outputMatrix[5] = right;
        break;
      case SPEAKER_7POINT1_SURROUND:
        outputMatrix[0] = outputMatrix[4] = outputMatrix[6] = left;
        outputMatrix[1] = outputMatrix[5] = outputMatrix[7] = right;
        break;
    }

    XAUDIO2_VOICE_DETAILS voiceDetails;
    SourceVoice.pSourceVoice->GetVoiceDetails( &voiceDetails );

    XAUDIO2_VOICE_DETAILS masterVoiceDetails;
    m_pMasteringVoice->GetVoiceDetails( &masterVoiceDetails );

    if ( FAILED( hr = SourceVoice.pSourceVoice->SetOutputMatrix( NULL, voiceDetails.InputChannels, masterVoiceDetails.InputChannels, outputMatrix ) ) )
    {
      m_pDebugger->Log( "Sound", "SetOutputMatrix failed with %x", hr );
    }
  }



  void XAudio2Sound::ApplyVolume( const tSoundInfo& SoundInfo, tX2AudioSound& SourceVoice, GR::u32 Volume )
  {
    if ( Volume < 0 )
    {
      Volume = 0;
    }
    if ( Volume > 100 )
    {
      Volume = 100;
    }
    SourceVoice.pSourceVoice->SetVolume( m_VolumeRange[Volume * MasterVolume( SoundInfo.Type ) / 100] * 0.0001f + 1.0f );
  }



  bool XAudio2Sound::Loop( GR::u32 ID )
  {
    return PlayChannel( ID, -1, 0, 100, 0, 0, true );
  }



  bool XAudio2Sound::LoopChannel( GR::u32 ID, GR::u32 Channel )
  {
    return false;
  }



  bool XAudio2Sound::PlayDetail( GR::u32 ID, GR::i32 iVolume, GR::i32 iPan, GR::u32 iFrequency, bool bLooping )
  {
    return false;
  }



  bool XAudio2Sound::Stop( GR::u32 ID )
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Called Stop with non existant sound %d", ID );
      return false;
    }

    bool    hadPlayingInstance = false;
    tSoundInfo& soundInfo( itS->second );
    for each ( auto& soundVoice in soundInfo.SoundVoices )
    {
      soundVoice.pSourceVoice->Stop();
      hadPlayingInstance = true;
    }
    return hadPlayingInstance;
  }



  bool XAudio2Sound::StopChannel( GR::u32 Channel )
  {
    return false;
  }



  bool XAudio2Sound::StopAll()
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.begin() );
    while ( itS != m_Sounds.end() )
    {
      tSoundInfo& soundInfo( itS->second );
      for each ( auto& soundVoice in soundInfo.SoundVoices )
      {
        soundVoice.pSourceVoice->Stop();
        return true;
      }

      ++itS;
    }

    return false;
  }



  bool XAudio2Sound::IsPlaying( GR::u32 ID )
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Called IsPlaying non existant sound %d", ID );
      return false;
    }

    tSoundInfo& soundInfo( itS->second );
    for each ( auto& soundVoice in soundInfo.SoundVoices )
    {
      XAUDIO2_VOICE_STATE   state;
      soundVoice.pSourceVoice->GetState( &state );

      if ( state.BuffersQueued > 0 )
      {
        return true;
      }
    }
    return false;
  }



  bool XAudio2Sound::IsChannelPlaying( GR::u32 Channel )
  {
    return false;
  }




  void XAudio2Sound::ReserveChannel( GR::u32 Channel )
  {
  }



  void XAudio2Sound::UnreserveChannel( GR::u32 Channel )
  {
  }




  bool XAudio2Sound::Resume( GR::u32 ID )
  {
    return false;
  }



  bool XAudio2Sound::Pause( GR::u32 ID )
  {
    return false;
  }




  bool XAudio2Sound::SetFrequency( GR::u32 ID, GR::u32 Frequency )
  {
    return false;
  }




  GR::u32 XAudio2Sound::GetOriginalFrequency( GR::u32 ID )
  {
    return 0;
  }



  GR::u32 XAudio2Sound::GetCurrentFrequency( GR::u32 ID )
  {
    return 0;
  }




  bool XAudio2Sound::SetPos( GR::u32 ID, GR::u32 Pos )
  {
    return false;
  }



  GR::u32 XAudio2Sound::GetPos( GR::u32 ID )
  {
    std::map<GR::u32, tSoundInfo>::iterator   itS( m_Sounds.find( ID ) );
    if ( itS == m_Sounds.end() )
    {
      m_pDebugger->Log( "Sound", "Called GetPos non existant sound %d", ID );
      return false;
    }

    tSoundInfo& soundInfo( itS->second );
    for each ( auto& soundVoice in soundInfo.SoundVoices )
    {
      XAUDIO2_VOICE_STATE   state;
      soundVoice.pSourceVoice->GetState( &state );

      return (GR::u32)( soundInfo.WaveFormat.Format.wBitsPerSample * state.SamplesPlayed ) / 8;
    }
    return 0;
  }




  bool XAudio2Sound::SetPan( GR::u32 ID, signed long slPan )
  {
    return false;
  }




  bool XAudio2Sound::SetVolume( GR::u32 ID, GR::u32 ucVolume )
  {
    return false;
  }



  GR::u32 XAudio2Sound::GetVolume( GR::u32 ID )
  {
    return 0;
  }




  bool XAudio2Sound::SetChannelFrequency( GR::u32 Channel, GR::u32 Frequency )
  {
    return false;
  }




  GR::u32 XAudio2Sound::GetChannelOriginalFrequency( GR::u32 Channel )
  {
    return 0;
  }



  GR::u32 XAudio2Sound::GetChannelCurrentFrequency( GR::u32 Channel )
  {
    return 0;
  }




  bool XAudio2Sound::SetChannelPos( GR::u32 Channel, GR::u32 Pos )
  {
    return false;
  }



  GR::u32 XAudio2Sound::GetChannelPos( GR::u32 Channel )
  {
    return 0;
  }




  bool XAudio2Sound::SetChannelPan( GR::u32 Channel, signed long slPan )
  {
    return false;
  }




  bool XAudio2Sound::SetChannelVolume( GR::u32 Channel, GR::u32 ucVolume )
  {
    return false;
  }



  GR::u32 XAudio2Sound::GetChannelVolume( GR::u32 Channel )
  {
    return 0;
  }




  bool XAudio2Sound::Create3dListener()
  {
    return false;
  }



  bool XAudio2Sound::Set3dListenerPosition( float fX, float fY, float fZ )
  {
    return false;
  }



  bool XAudio2Sound::Set3dListenerOrientation( float fFrontX, float fFrontY, float fFrontZ,
    float fUpX, float fUpY, float fUpZ )
  {
    return false;
  }



  bool XAudio2Sound::Set3dListenerParams( float fX, float fY, float fZ,
    float fFrontX, float fFrontY, float fFrontZ,
    float fUpX, float fUpY, float fUpZ,
    float fVelocityX, float fVelocityY, float fVelocityZ )
  {
    return false;
  }



  bool XAudio2Sound::Set3dDistanceFactor( float fDistance )
  {
    return false;
  }



  bool XAudio2Sound::Set3dDopplerFactor( float fDoppler )
  {
    return false;
  }



  bool XAudio2Sound::Set3dRollOffFactor( float fRollOff )
  {
    return false;
  }




  bool XAudio2Sound::Play3d( GR::u32 ID, float fX, float fY, float fZ, bool bLooping )
  {
    return false;
  }



  bool XAudio2Sound::Set3dPosition( GR::u32 ID, float fX, float fY, float fZ )
  {
    return false;
  }




  void XAudio2Sound::ProcessEvent( const tGlobalEvent& Event )
  {
  }



  XAudio2Sound& XAudio2Sound::Instance()
  {
    static XAudio2Sound    staticInstance;

    return staticInstance;
  }



}
