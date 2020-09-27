#ifndef _IANIMATOR_H
#define _IANIMATOR_H


/////////////////////////////////////////////////////////////////////
//-    Interface für eine animierende Klasse
/////////////////////////////////////////////////////////////////////
class IAnimator
{
public:
  virtual ~IAnimator() {};

  //- Animiere über den Zeitraum fDeltaTimeInSeconds,
  //- gib false zurück, falls Ani abgeschlossen
  virtual bool  Animate( const float& fDeltaTimeInSeconds ) = 0;
  //- Vergangene Zeit erfahren
  virtual float ElapsedTime() const = 0;
  //- Animation abgeschlossen?
  virtual bool Finished() const = 0;

  //- Animationsabschluss erzwingen (auf Wunsch von Georg)
  virtual void Finish() = 0;

  virtual void* Target() const = 0;
// private:
  // //- Sinnvoll?
  // IAnimator(); //- privater Default-Konstruktor -
                    // //- Animator MUSS mit Wertreferenz initialisisert werden
  // IAnimator( const IAnimator& rhs );
  // IAnimator& operator=( const IAnimator& rhs );
};

#endif// _IANIMATOR_H

