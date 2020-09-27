#ifndef _EXCLUSIVE_DISCRETE_KEYFRAME_ANIMATOR_H
#define _EXCLUSIVE_DISCRETE_KEYFRAME_ANIMATOR_H

#include <Interface\IAnimator.h>
#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <map>


namespace anim
{
/////////////////////////////////////////////////////////////////////
//-    Achtung! Dieser Animator sagt nicht Bescheid, wann Ende ist - Peter fragen!
/////////////////////////////////////////////////////////////////////

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
  class exclusive_discrete_keyframe_animator : public IAnimator
  {
  public:
    typedef std::map< float, S > keymap_t;


    //- Konstruktor
    exclusive_discrete_keyframe_animator(  S&    sValue,                    //- Referenz auf den zu modifizierenden Wert
                                         const bool  relative = false );  //- wenn true, wird der übergebene Endwert
                                                                          //- als Offset aufgedasst und
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

    void AddKey( const float fKeytime, const S& KeyValue );

  protected:
    S&        m_sValue;                 //- Referenz auf den zu modifizierenden Wert
    float     m_fElapsedTimeInSeconds;  //- die gesamte vergangene Zeit
    S         m_sStartValue;            //- Startwert (entspricht dem ursprünglichen Value)
    bool      m_bRelative;
    keymap_t  m_keys;

  };



  /////////////////////////////////////////////////////////////////////
  //-
  //-    Implementierung
  //-
  /////////////////////////////////////////////////////////////////////

  template < typename S >
    exclusive_discrete_keyframe_animator< S >::exclusive_discrete_keyframe_animator< S >(
                                S&          sValue,
                                const bool  relative /* = false */ )  //- wenn true, wird der übergebene Endwert
                                                                     //- als Offset aufgefasst und
                                                                     //- ein entsprechender absoluter Wert gesetzt
    : m_sValue(                 sValue ),
      m_fElapsedTimeInSeconds(  0 ),
      m_sStartValue(            sValue ),
      m_bRelative(              relative )
    {}

  template < typename S >
    //- gibt die Adresse des beiflussten Wertes zurück
    void* exclusive_discrete_keyframe_animator< S >::Target() const { return &m_sValue; }


  template < typename S >
    //- Animieren über DeltaT
    //- gibt true zurück, falls weiteranimiert werden muss
    bool exclusive_discrete_keyframe_animator< S >::Animate( const float& fDeltaTimeInSeconds )
    {
      m_fElapsedTimeInSeconds  += fDeltaTimeInSeconds;

        //- Finde den letzten Key, der zeitmässig überschritten wurde
      keymap_t::iterator it = m_keys.upper_bound( m_fElapsedTimeInSeconds );
      if ( it == m_keys.end() )
      {
        m_sValue = m_sStartValue;
        m_fElapsedTimeInSeconds = 0.0f; //- Baustelle - wird Ruckeln verursachen!
      }
      else
      {
        m_sValue = it->second;
      }

      return true; //- niemals fertig (baustelle)
    }

    //- Wieviel Ziet hat der Animator bereits gezählt?
  template < typename S >
    float exclusive_discrete_keyframe_animator< S >::ElapsedTime() const { return m_fElapsedTimeInSeconds; }

  template < typename S >
    bool  exclusive_discrete_keyframe_animator< S >::Finished()    const { return false; } //- baustelle

  template < typename S >
    void exclusive_discrete_keyframe_animator< S >::Finish()
    {
      if ( !m_keys.empty() )
      {
        keymap_t::iterator itnow( m_keys.end() );
        --itnow;
        m_sValue = itnow->second;
      }
    }

  template < typename S >
    void exclusive_discrete_keyframe_animator< S >::AddKey( const float fKeytime, const S& KeyValue )
    {
      m_keys[ fKeytime ] = m_bRelative ? KeyValue + m_sValue : KeyValue; //- funktioniert das? habs noch nicht getestet!
    }

}; // namespace anim



#endif// _EXCLUSIVE_DISCRETE_KEYFRAME_ANIMATOR_H