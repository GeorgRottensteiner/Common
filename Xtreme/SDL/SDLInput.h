#ifndef SDL_INPUT_H
#define SDL_INPUT_H

#include <Xtreme/Input/XBasicInput.h>

#include <SDL_scancode.h>


union SDL_Event;
 
class SDLInput : public XBasicInput
{
  private:

    static char                   m_KeyName[240][25];



  public:

    SDLInput();

    virtual ~SDLInput();


    virtual bool                  HasDeviceForceFeedback( GR::u32 Device ) const;
    virtual GR::u32               DeviceForceFeedbackAxisCount( GR::u32 Device ) const;
    virtual bool                  SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const;

    // und die elementare Poll-Funktion
    virtual void                  Poll();


    virtual void                  Init();
    virtual void                  Exit();
    virtual void                  Update( const float fElapsedTime );

    virtual bool                  Initialize( GR::IEnvironment& Environment );
    virtual bool                  Release();

    void UpdateMouse( int X, int Y, int Buttons );

    void                          SetActive( bool Active = true );

    void                          OnSDLEvent( const SDL_Event& Event );
    Xtreme::eInputDefaultButtons  MapToKey( SDL_Scancode SDLKey );
};




#endif // SDL_INPUT_H
