#ifndef _EXCLUSIVE_LINEAR_KEYFRAME_ANIMATOR_H
#define _EXCLUSIVE_LINEAR_KEYFRAME_ANIMATOR_H

#include <Interface\IAnimator.h>

namespace anim
{

#pragma message( "exclusive_linear_keyframe_animator: Do not benutz this! it is a baustelle! Peter asken! (ups, habs ganz vergessen! Anm. Peter)" )
  //- Werte animieren.
  //- geht mit allen Datentypen, die die grundlegenden Rechenoperationen zur
  //- Verf�gung stellen (auch z.b. 2D- oder 3D-Vektoren)

  //- Achtung! Dieser Animator setzt den Wert IMMER ABSOLUT,
  //- abh�ngig von den anfangs bestimmten Intervallen
  //- Wenn also zwei Animatoren auf einen Wert wirken,
  //- wird die �nderung des zuerst wirksamen Animators vollst�ndig
  //- �berschrieben

  //- exclusive_animatoren setzen den Wert immer absolut, es hat also
  //- keinen Sinn, mehrere Animatoren auf einen Wert anzusetzen.
  //- Jeweils der zuletzt aktive Animator wird den Wert so �berschreiben, wie
  //- es aus seiner Sicht richtig ist.



  /////////////////////////////////////////////////////////////////////
  //-
  //-    Interface
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S >
  class exclusive_linear_keyframe_animator : public IAnimator
  {
  public:
    //- Konstruktor
    exclusive_linear_keyframe_animator(  S&    sValue,                    //- Referenz auf den zu modifizierenden Wert
                                         const bool  relative = false );  //- wenn true, wird der �bergebene Endwert
                                                                          //- als Offset aufgedasst und
                                                                          //- ein entsprechender absoluter Wert gesetzt
    //- gibt die Adresse des beiflussten Wertes zur�ck
    void* Target() const;

    //- Animieren �ber DeltaT
    //- gibt true zur�ck, falls weiteranimiert werden muss
    virtual bool Animate( const float& fDeltaTimeInSeconds );

    //- Wieviel Zeit hat der Animator bereits gez�hlt?
    virtual float ElapsedTime() const;
    virtual bool  Finished()    const;
    virtual void  Finish();

    void AddKey( const float fKeytime, const S& KeyValue );


  protected:
    S&    m_sValue;                 //- Referenz auf den zu modifizierenden Wert
    float m_fElapsedTimeInSeconds;  //- die gesamte vergangene Zeit
    S     m_sStartValue;            //- Startwert (entspricht dem urspr�nglichen Value)
  };



  /////////////////////////////////////////////////////////////////////
  //-
  //-    Implementierung
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S >
    exclusive_linear_keyframe_animator< S >::exclusive_linear_keyframe_animator< S >(
                                S&          sValue,
                                const bool  relative /* = false */ )  //- wenn true, wird der �bergebene Endwert
                                                                     //- als Offset aufgedasst und
                                                                     //- ein entsprechender absoluter Wert gesetzt
    : m_sValue(                 sValue ),
      m_fElapsedTimeInSeconds(  0 ),
      m_sStartValue(            sValue )
      // ,
      // m_sEndValue(              relative ? sEndValue + sValue : sEndValue )
      //- ist relative true, wird sValue als relativ angesehen
    {}

  template < typename S >
    //- gibt die Adresse des beiflussten Wertes zur�ck
    void* exclusive_linear_keyframe_animator< S >::Target() const { return &m_sValue; }


  template < typename S >
    //- Animieren �ber DeltaT
    //- gibt true zur�ck, falls weiteranimiert werden muss
    bool exclusive_linear_keyframe_animator< S >::Animate( const float& fDeltaTimeInSeconds )
    {
      m_fElapsedTimeInSeconds  += fDeltaTimeInSeconds;
      m_sValue = S( m_sStartValue + ( m_sEndValue - m_sStartValue ) / m_fEndTimeInSeconds * m_fElapsedTimeInSeconds );

      //- Zeit nicht vorbei? => weiteranimieren
      if ( m_fElapsedTimeInSeconds < m_fEndTimeInSeconds )  return true;

      //- �berschreitung der Zeit => Endwert setzen und false zur�ckgeben
      m_sValue = m_sEndValue;
      return false; //- fertig
    }


    //- Wieviel Ziet hat der Animator bereits gez�hlt?
  template < typename S >
    float exclusive_linear_keyframe_animator< S >::ElapsedTime() const { return m_fElapsedTimeInSeconds; }
  template < typename S >
    bool  exclusive_linear_keyframe_animator< S >::Finished()    const { return m_fElapsedTimeInSeconds >= m_fEndTimeInSeconds; }


}; // namespace anim



#endif// _EXCLUSIVE_LINEAR_KEYFRAME_ANIMATOR_H