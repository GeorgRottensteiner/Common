#ifndef _DELAYED_METHOD_CALL_H
#define _DELAYED_METHOD_CALL_H

#include <Interface\IAnimator.h>
#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten

#include <method_call.h>

namespace anim
{
  //- Dieser Animator kann zum verzögerten Aufruf einer Memberfunktion verwendet werden

  /////////////////////////////////////////////////////////////////////
  //-
  //-    Interface
  //-
  /////////////////////////////////////////////////////////////////////

  class delayed_method_call : public IAnimator
  {
  public:

    delayed_method_call( rsi::method_call_interface *newed, const float fDauerInSeconds = 1.0f )
    : m_pCall(                 newed ),
      m_fElapsedTimeInSeconds(  0 ),
      m_fEndTimeInSeconds(      fDauerInSeconds <= 0 ? 1 : fDauerInSeconds )
    {}

    //- Destruktor
    ~delayed_method_call()
    {
      delete m_pCall;
      m_pCall = 0;
    }

    //- gibt die Adresse des beiflussten Wertes zurück (hier nutzlos)
    void* Target() const { return 0; }


    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    bool Animate( const float& fDeltaTimeInSeconds )
    {
      m_fElapsedTimeInSeconds  += fDeltaTimeInSeconds;

      //- Zeit nicht vorbei? => weiteranimieren
      if ( m_fElapsedTimeInSeconds < m_fEndTimeInSeconds )  return true;

      //- Überschreitung der Zeit => Endwert setzen und false zurückgeben
      if ( m_pCall ) (*m_pCall)();
      return false; //- fertig
    }


    //- Wieviel Zeit hat der Animator bereits gezählt?
    float ElapsedTime() const { return m_fElapsedTimeInSeconds; }

    bool  Finished()    const { return m_fElapsedTimeInSeconds >= m_fEndTimeInSeconds; }

    void  Finish() { if ( m_pCall ) (*m_pCall)(); }

  protected:
    float     m_fElapsedTimeInSeconds;  //- die gesamte vergangene Zeit
    float     m_fEndTimeInSeconds;      //- Endzeitpunkt der Animation
    rsi::method_call_interface* m_pCall;
  };

}; // namespace anim



#endif// _DELAYED_ASSIGNMENT_H