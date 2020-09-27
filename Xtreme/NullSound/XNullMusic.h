#ifndef XNullMusic_H_INCLUDED
#define XNullMusic_H_INCLUDED



#include <Xtreme/XMusic.h>



class XNullMusic : public XMusic
{

  protected:

  public:

    virtual ~XNullMusic();

    virtual bool            Initialize( GR::IEnvironment& Environment );
    virtual bool            Release();

    virtual bool            IsInitialized();

    virtual bool            SetVolume( int iVolume );
    virtual int             Volume();

    virtual bool            LoadMusic( const GR::Char* szFileName );
    virtual bool            LoadMusic( IIOStream& Stream );

    virtual bool            Play( bool bLooped = true );
    virtual void            Stop();
    virtual bool            IsPlaying();

    virtual bool            Resume();
    virtual bool            Pause();

};


#endif //XNullMusic_H_INCLUDED