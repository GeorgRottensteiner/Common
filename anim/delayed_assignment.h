#ifndef _DELAYED_ASSIGNMENT_H
#define _DELAYED_ASSIGNMENT_H

#include <Interface\IAnimator.h>
#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten


namespace anim
{
  //- Dieser Animator kann zum verzögerten Setzen eines Wertes benutzt werden

  //- exclusive_animatoren setzen den Wert immer absolut, es hat also
  //- keinen Sinn, mehrere Animatoren auf einen Wert anzusetzen.
  //- Jeweils der zuletzt aktive Animator wird den Wert so Überschreiben, wie
  //- es aus seiner Sicht richtig ist.



  /////////////////////////////////////////////////////////////////////
  //-
  //-    Interface
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S >
  class delayed_assignment : public IAnimator
  {
  public:
    //- Konstruktor
    delayed_assignment(       S&    sValue,
                              const S&    sEndValue,
                              const float fDauerInSeconds = 1.0f,
                              const bool  relative = false );  //- wenn true, wird der übergebene Endwert
                                                                //- als Offset aufgedasst und
                                                                //- ein entsprechender absoluter Wert gesetzt
                                                                          //- ein entsprechender absoluter Wert gesetzt
    //- gibt die Adresse des beiflussten Wertes zurück
    void* Target() const;

    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    virtual bool Animate( const float& fDeltaTimeInSeconds );

    //- Wieviel Zeit hat der Animator bereits gezählt?
    virtual float ElapsedTime() const;
    virtual bool  Finished()    const;
    virtual void  Finish();


  protected:
  protected:
    S&        m_sValue;                 //- Referenz auf den zu modifizierenden Wert
    float     m_fElapsedTimeInSeconds;  //- die gesamte vergangene Zeit
    float     m_fEndTimeInSeconds;      //- Endzeitpunkt der Animation
    S         m_sStartValue;            //- Startwert (entspricht dem ursprünglichen Value)
    S         m_sEndValue;              //- Letzter Wert, Zielwert
    bool      m_bRelative;
  };



  /////////////////////////////////////////////////////////////////////
  //-
  //-    Implementierung
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S >
    delayed_assignment< S >::delayed_assignment< S >(
                                      S&    sValue,
                                const S&    sEndValue,
                                const float fDauerInSeconds /* = 1.0f */,
                                const bool  relative /* = false */ )  //- wenn true, wird der übergebene Endwert
                                                                     //- als Offset aufgedasst und
                                                                     //- ein entsprechender absoluter Wert gesetzt
    : m_sValue(                 sValue ),
      m_fElapsedTimeInSeconds(  0 ),
      m_fEndTimeInSeconds(      fDauerInSeconds <= 0 ? 1 : fDauerInSeconds ),
      m_sStartValue(            sValue ),
      m_sEndValue(              relative ? sEndValue + sValue : sEndValue )
      //- ist relative true, wird sValue als relativ angesehen
    {}


  template < typename S >
    //- gibt die Adresse des beiflussten Wertes zurück
    void* delayed_assignment< S >::Target() const { return &m_sValue; }


  template < typename S >
    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    bool delayed_assignment< S >::Animate( const float& fDeltaTimeInSeconds )
    {
      m_fElapsedTimeInSeconds  += fDeltaTimeInSeconds;

      //- Zeit nicht vorbei? => weiteranimieren
      if ( m_fElapsedTimeInSeconds < m_fEndTimeInSeconds )  return true;

      //- Überschreitung der Zeit => Endwert setzen und false zurückgeben
      m_sValue = m_sEndValue;
      return false; //- fertig
    }

    //- Wieviel Zeit hat der Animator bereits gezählt?
  template < typename S >
    float delayed_assignment< S >::ElapsedTime() const { return m_fElapsedTimeInSeconds; }

  template < typename S >
    bool  delayed_assignment< S >::Finished()    const { return m_fElapsedTimeInSeconds >= m_fEndTimeInSeconds; }

  template < typename S >
    void delayed_assignment< S >::Finish() { m_sValue = m_sEndValue; }

}; // namespace anim



#endif// _DELAYED_ASSIGNMENT_H