#ifndef _EXCLUSIVE_BENDING_ANIMATOR_H
#define _EXCLUSIVE_BENDING_ANIMATOR_H

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


  //- bending animatoren "biegen" die Zeit mithilfe von Benderfunktionen
  //- oder Funktoren (siehe auch bender.h), um beschleunigte Bewegungen u. ä.
  //- zu ermöglichen

  //- ein Bender erwartet einen Wert im Intervall [0.0f, 1.0f]
  //- und liefert einen Wert aus dem Intervall [0.0f, 1.0f] zurück
  //- (*keine* Bereichsprüfung!)


  /////////////////////////////////////////////////////////////////////
  //-
  //-    Interface
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S, typename B >
  class exclusive_bending_animator : public IAnimator
  {
  public:
    //- Konstruktor
    exclusive_bending_animator(       S&    sValue,
                                const S&    sEndValue,
                                const B&    bender,
                                const float fDauerInSeconds = 1.0f,
                                const bool  relative = false );  //- wenn true, wird der übergebene Endwert
                                                                //- als Offset aufgedasst und
                                                                //- ein entsprechender absoluter Wert gesetzt

    //- ändert die im Konstruktor angegebene Endzeit
    void SetTime( const float fDauerInSeconds );

    //- ändert den im Konstruktor angegebenen Bender
    void SetBender( const B& bender );

    //- gibt die Adresse des beiflussten Wertes zurück
    void* Target() const;

    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    virtual bool Animate( const float& fDeltaTimeInSeconds );

    //- Wieviel Ziet hat der Animator bereits gezählt?
    virtual float ElapsedTime() const;
    virtual bool  Finished()    const;
    virtual void  Finish();

  protected:
    S&    m_sValue;                 //- Referenz auf den zu modifizierenden Wert
    float m_fElapsedTimeInSeconds;  //- die gesamte vergangene Zeit
    float m_fEndTimeInSeconds;      //- Endzeitpunkt der Animation
    S     m_sStartValue;            //- Startwert (entspricht dem ursprünglichen Value)
    S     m_sEndValue;              //- Letzter Wert, Zielwert
    B     m_Bender;
  };


  /////////////////////////////////////////////////////////////////////
  //-
  //-    Implementierung
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S, typename B >
    exclusive_bending_animator< S, B >::exclusive_bending_animator< S, B >(
                                      S&    sValue,
                                const S&    sEndValue,
                                const B& bender,
                                const float fDauerInSeconds /* = 1.0f */,
                                const bool  relative /* = false */ )  //- wenn true, wird der übergebene Endwert
                                                                     //- als Offset aufgefasst und
                                                                     //- ein entsprechender absoluter Wert gesetzt
    : m_sValue(                 sValue ),
      m_fElapsedTimeInSeconds(  0 ),
      m_Bender(                 bender ),
      m_fEndTimeInSeconds(      fDauerInSeconds <= 0 ? 1 : fDauerInSeconds ),
      m_sStartValue(            sValue ),
      m_sEndValue(              relative ? sEndValue + sValue : sEndValue )
      //- ist relative true, wird sValue als relativ angesehen
    {}

  template < typename S, typename B >
    //- ändert die im Konstruktor angegebene Endzeit
    void exclusive_bending_animator< S, B >::SetTime( const float fDauerInSeconds )
    {
      m_fEndTimeInSeconds = ( fDauerInSeconds <= 0 ? 1 : fDauerInSeconds );
    }

  template < typename S, typename B >
    void exclusive_bending_animator< S, B >::SetBender( const B& bender )
    {
      m_Bender = bender;
    }

  template < typename S, typename B >
    //- gibt die Adresse des beiflussten Wertes zurück
    void* exclusive_bending_animator< S, B >::Target() const { return &m_sValue; }

  template < typename S, typename B >
    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    bool exclusive_bending_animator< S, B >::Animate( const float& fDeltaTimeInSeconds )
    {
      m_fElapsedTimeInSeconds  += fDeltaTimeInSeconds;

      m_sValue = m_sStartValue
                  + ( m_sEndValue - m_sStartValue )
                      * m_Bender( m_fElapsedTimeInSeconds / m_fEndTimeInSeconds );

      //- Zeit nicht vorbei? => weiteranimieren
      if ( m_fElapsedTimeInSeconds < m_fEndTimeInSeconds )  return true;

      //- Überschreitung der Zeit => Endwert setzen und false zurückgeben
      m_sValue = m_sEndValue;
      return false; //- fertig
    }

    //- Wieviel Ziet hat der Animator bereits gezählt?
  template < typename S, typename B >
    float exclusive_bending_animator< S, B >::ElapsedTime() const
    { return m_fElapsedTimeInSeconds; }

  template < typename S, typename B >
    bool  exclusive_bending_animator< S, B >::Finished()    const
    { return m_fElapsedTimeInSeconds >= m_fEndTimeInSeconds; }

  template < typename S, typename B >
    void exclusive_bending_animator< S, B >::Finish() { m_sValue = m_sEndValue; }



}; // namespace anim



#endif// _EXCLUSIVE_BENDING_ANIMATOR_H