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

        float       m_StartValue,
                    m_EndValue,
                    m_ElapsedTime;


      public:



        AcceleratingIterator( float Start = 0.0f, float End = 1.0f, float Length = 1.0f ) :
          m_StartValue( Start ),
          m_EndValue( End ),
          m_ElapsedTime( 0.0f )
        {
          m_IteratorLength = Length;
        }



        void SetValues( float Start, float End, float Length )
        {
          m_StartValue      = Start;
          m_EndValue        = End;
          m_IteratorLength  = Length;
          Reset();
        }



        float NextValue( const float ElapsedTime = 1.0f )
        {
          m_ElapsedTime += ElapsedTime;
          return Value();
        }



        float Reset()
        {
          m_ElapsedTime = 0.0f;
          return Value();
        }



        float Value()
        {
          if ( m_ElapsedTime >= m_IteratorLength )
          {
            return m_EndValue;
          }
          else if ( m_ElapsedTime <= 0.0f )
          {
            return m_StartValue;
          }

          return m_StartValue + ( m_EndValue - m_StartValue ) * ( 1.0f - cosf( m_ElapsedTime / m_IteratorLength * 3.1415926f * 0.5f ) );
        }



        virtual bool Done()
        {
          return ( m_ElapsedTime >= m_IteratorLength );
        }

    };



  }
}


#endif 



