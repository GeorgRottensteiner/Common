#ifndef _EXCLUSIVE_LINEAR_ANIMATOR_H
#define _EXCLUSIVE_LINEAR_ANIMATOR_H

#include <Interface\IAnimator.h>

namespace anim
{
  //- Werte animieren.
  //- geht mit allen Datentypen, die die grundlegenden Rechenoperationen zur
  //- Verfügung stellen (auch z.b. 2D- oder 3D-Vektoren)

  //- Achtung! Dieser Animator setzt den Wert IMMER ABSOLUT,
  //- abhängig von den anfangs bestimmten Intervallen
  //- Wenn also zwei Animatoren auf einen Wert wirken,
  //- wird die Änderung des zuerst wirksamen Animators vollständig
  //- überschrieben

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
  class exclusive_linear_animator : public IAnimator
  {
  public:
    //- Konstruktor
    exclusive_linear_animator(        S&    sValue,         //- Referenz auf den zu modifizierenden Wert
                                const S&    sEndValue,      //- Letzter Wert, Zielwert
                                const float fDauerInSeconds = 1.0f, //- Dauer der Animation (ab JETZT)
                                const bool  relative = false );  //- wenn true, wird der übergebene Endwert
                                                                //- als Offset aufgedasst und
                                                                //- ein entsprechender absoluter Wert gesetzt

    //- ändert die im Konstruktor angegebene Endzeit
    void SetTime( const float fDauerInSeconds );

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
    S&    m_sValue;                 //- Referenz auf den zu modifizierenden Wert
    float m_fElapsedTimeInSeconds;  //- die gesamte vergangene Zeit
    float m_fEndTimeInSeconds;      //- Endzeitpunkt der Animation
    S     m_sStartValue;            //- Startwert (entspricht dem ursprünglichen Value)
    S     m_sEndValue;              //- Letzter Wert, Zielwert
  };



  /////////////////////////////////////////////////////////////////////
  //-
  //-    Implementierung
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S >
    exclusive_linear_animator< S >::exclusive_linear_animator< S >(        S&    sValue,
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
    //- ändert die im Konstruktor angegebene Endzeit
    void exclusive_linear_animator< S >::SetTime( const float fDauerInSeconds )
    {
      m_fEndTimeInSeconds = ( fDauerInSeconds <= 0 ? 1 : fDauerInSeconds );
    }

  template < typename S >
    //- gibt die Adresse des beiflussten Wertes zurück
    void* exclusive_linear_animator< S >::Target() const { return &m_sValue; }


  template < typename S >
    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    bool exclusive_linear_animator< S >::Animate( const float& fDeltaTimeInSeconds )
    {
      m_fElapsedTimeInSeconds  += fDeltaTimeInSeconds;
      m_sValue = S( m_sStartValue + ( m_sEndValue - m_sStartValue ) / m_fEndTimeInSeconds * m_fElapsedTimeInSeconds );

      //- Zeit nicht vorbei? => weiteranimieren
      if ( m_fElapsedTimeInSeconds < m_fEndTimeInSeconds )  return true;

      //- Überschreitung der Zeit => Endwert setzen und false zurückgeben
      m_sValue = m_sEndValue;
      return false; //- fertig
    }


    //- Wieviel Ziet hat der Animator bereits gezählt?
  template < typename S >
    float exclusive_linear_animator< S >::ElapsedTime() const { return m_fElapsedTimeInSeconds; }
  template < typename S >
    bool  exclusive_linear_animator< S >::Finished()    const { return m_fElapsedTimeInSeconds >= m_fEndTimeInSeconds; }
  template < typename S >
    void  exclusive_linear_animator< S >::Finish()       { m_sValue = m_sEndValue; }


}; // namespace anim



#endif// _EXCLUSIVE_LINEAR_ANIMATOR_H