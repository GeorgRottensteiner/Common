#ifndef DXMIDI_H
#define DXMIDI_H

#include <dmusicc.h>
#include <dmusici.h>
#include <dsound.h>

#include <Xtreme/XMusic.h>


class CMusicSegment;




class CDXMidi : public XMusic
{

  protected:

    BOOL                      m_bCleanupCOM;
    IDirectMusicLoader8*      m_pLoader;
    IDirectMusicPerformance8* m_pPerformance;
    IDirectSound3DListener*   m_pDSListener;
    DS3DLISTENER              m_dsListenerParams;                // Listener properties

    CMusicSegment*            m_pSegment;

    bool                      m_bPaused;

    MUSIC_TIME                m_musPausedTime;


  public:

    CDXMidi();
    ~CDXMidi();

    bool            Initialize( GR::IEnvironment& Environment );
    bool            Release();

    bool            Play( bool bLooped = true );
    bool            IsPlaying();
    void            Stop();
    bool            Resume();
    bool            Pause();

    bool            SetVolume( int nVolume );
    int             Volume();

    bool            LoadMusic( const char* szFileName );
    bool            LoadMusic( IIOStream& Stream );

    bool            IsInitialized();


    inline IDirectMusicLoader8*      GetLoader()      { return m_pLoader; }
    inline IDirectMusicPerformance8* GetPerformance() { return m_pPerformance; }
    inline IDirectSound3DListener*   GetListener()    { return m_pDSListener; }

    IDirectMusicAudioPath8* GetDefaultAudioPath();

    void    CollectGarbage();

    

    HRESULT CreateSegmentFromFile( CMusicSegment** ppSegment, const char* strFileName, 
		                           BOOL bDownloadNow = TRUE, BOOL bIsMidiFile = FALSE );

    HRESULT CreateChordMapFromFile( IDirectMusicChordMap8** ppChordMap, TCHAR* strFileName );
    HRESULT CreateStyleFromFile( IDirectMusicStyle8** ppStyle, TCHAR* strFileName );
    HRESULT GetMotifFromStyle( IDirectMusicSegment8** ppMotif, TCHAR* strStyle, TCHAR* wstrMotif );

    HRESULT CreateSegmentFromResource( CMusicSegment** ppSegment, TCHAR* strResource, TCHAR* strResourceType, 
		                           BOOL bDownloadNow = TRUE, BOOL bIsMidiFile = FALSE );

    void Set3DParameters( FLOAT fDistanceFactor, FLOAT fDopplerFactor, FLOAT fRolloffFactor );

};



class CMusicSegment
{

  protected:

    IDirectMusicSegment8*     m_pSegment;
    IDirectMusicLoader8*      m_pLoader;
    IDirectMusicPerformance8* m_pPerformance;
    IDirectMusicAudioPath8*   m_pEmbeddedAudioPath;
    BOOL                      m_bDownloaded;


  public:

    CMusicSegment( IDirectMusicPerformance8* pPerformance, 
                   IDirectMusicLoader8* pLoader,
                   IDirectMusicSegment8* pSegment );
    virtual ~CMusicSegment();

    inline  IDirectMusicSegment8* GetSegment() { return m_pSegment; }
    HRESULT GetStyle( IDirectMusicStyle8** ppStyle, DWORD dwStyleIndex = 0 );

    HRESULT SetRepeats( DWORD dwRepeats );
    virtual HRESULT Play( DWORD dwFlags = DMUS_SEGF_SECONDARY, IDirectMusicAudioPath8* pAudioPath = NULL );
    HRESULT Resume( MUSIC_TIME& musTime );
    HRESULT Stop( DWORD dwFlags = 0 );
    HRESULT Download( IDirectMusicAudioPath8* pAudioPath = NULL );
    HRESULT Unload( IDirectMusicAudioPath8* pAudioPath = NULL );

    BOOL    IsPlaying();

};




#endif // DXMIDI_H
