#ifndef XMUSIC_H_INCLUDED__
#define XMUSIC_H_INCLUDED__

#include <GR/GRTypes.h>

#include <Lang/Service.h>



struct IIOStream;

class XMusic : public GR::Service::Service
{

  public:

    virtual ~XMusic()
    {
    }


    virtual bool            Initialize( GR::IEnvironment& Environment ) = 0;
    virtual bool            Release() = 0;

    virtual bool            IsInitialized() = 0;

    virtual bool            SetVolume( int Volume ) = 0;
    virtual int             Volume() = 0;

    virtual bool            LoadMusic( const GR::Char* FileName ) = 0;
    virtual bool            LoadMusic( IIOStream& Stream ) = 0;

    virtual bool            Play( bool Looped = true ) = 0;
    virtual void            Stop() = 0;
    virtual bool            IsPlaying() = 0;

    virtual bool            Resume() = 0;
    virtual bool            Pause() = 0;

};


#endif //XMUSIC_H_INCLUDED__