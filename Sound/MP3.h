#ifndef __MP3_H_INCLUDED__
#define __MP3_H_INCLUDED__

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

#include <dshow.h>
#include <string>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/




/*-Structures-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CMP3File
{

  protected:

    IGraphBuilder               *m_pGraphBuilder;

    IMediaControl               *m_pMediaControl;

    IMediaSeeking               *m_pMediaSeeking;

    IBaseFilter                 *m_pBaseFilter;

    IBasicAudio                 *m_pBasicAudio;

    GR::String                 m_strFileName;

    BOOL                        m_bInitialized;


  public:

    CMP3File();
    ~CMP3File();

    BOOL Open( const char *szFileName );
    void Close();

    void Play();
    void Stop();

    void SetVolume( DWORD dwVolume );
    void SetPan( int iPan );

};



/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif //__MP3_H_INCLUDED__