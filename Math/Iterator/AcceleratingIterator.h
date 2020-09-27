#ifndef ACCELERATING_ITERATOR_H
#define ACCELERATING_ITERATOR_H



#include "IMathIterator.h"



namespace math
{
  namespace Iterator
  {
    class AcceleratingIterator : public IMathIterator<float>
    {

      protected:

        float       m_fStartValue,
                    m_fEndValue,
                    m_fElapsedTime;


      public:

        AcceleratingIterator() :
          m_fStartValue( 0.0f ),
          m_fEndValue( 1.0f ),
          m_fElapsedTime( 0.0f )
        {
          m_IteratorLength = 1.0f;
        }

        AcceleratingIterator( float fStart, float fEnd, float fLength ) :
          m_fStartValue( fStart ),
          m_fEndValue( fEnd ),
          m_fElapsedTime( 0.0f )
        {
          m_IteratorLength = fLength;
        }

        void SetValues( float fStart, float fEnd, float fLength )
        {
          m_fStartValue     = fStart;
          m_fEndValue       = fEnd;
          m_IteratorLength  = fLength;
          Reset();
        }

        float NextValue( const float fElapsedTime = 1.0f )
        {
          m_fElapsedTime += fElapsedTime;
          return Value();
        }

        float Reset()
        {
          m_fElapsedTime = 0.0f;
          return Value();
        }

        float Value()
        {
          if ( m_fElapsedTime >= m_IteratorLength )
          {
            return m_fEndValue;
          }
          else if ( m_fElapsedTime <= 0.0f )
          {
            return m_fStartValue;
          }

          return m_fStartValue + ( m_fEndValue - m_fStartValue ) * ( 1.0f - cosf( m_fElapsedTime / m_IteratorLength * 3.1415926f * 0.5f ) );
        }

        virtual bool Done()
        {
          return ( m_fElapsedTime >= m_IteratorLength );
        }

    };
  };
};


#endif // ACCELERATING_ITERATOR_H



