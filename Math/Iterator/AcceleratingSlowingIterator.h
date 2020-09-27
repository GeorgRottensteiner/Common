#ifndef ACC_SLOWING_ITERATOR_H
#define ACC_SLOWING_ITERATOR_H



#include "IMathIterator.h"



namespace math
{
  namespace Iterator
  {

    class AcceleratingSlowingIterator : public IMathIterator<float>
    {

      protected:

        float       m_StartValue,
                    m_EndValue,
                    m_ElapsedTime;


      public:

        AcceleratingSlowingIterator() :
          m_StartValue( 0.0f ),
          m_EndValue( 1.0f ),
          m_ElapsedTime( 0.0f )
        {
          m_IteratorLength = 1.0f;
        }



        AcceleratingSlowingIterator( float Start, float End, float Length, bool Looping = false ) :
          m_StartValue( Start ),
          m_EndValue( End ),
          m_ElapsedTime( 0.0f )
        {
          m_IteratorLength  = Length;
          m_Looping         = Looping;
        }



        void SetValues( float Start, float End, float Length, bool Looping = false )
        {
          m_StartValue      = Start;
          m_EndValue        = End;
          m_IteratorLength  = Length;
          m_Looping         = Looping;
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
          if ( m_Looping )
          {
            while ( m_ElapsedTime >= m_IteratorLength )
            {
              m_ElapsedTime -= m_IteratorLength;
            }
          }

          if ( m_ElapsedTime >= m_IteratorLength )
          {
            return m_EndValue;
          }
          else if ( m_ElapsedTime <= 0.0f )
          {
            return m_StartValue;
          }

          return m_StartValue + ( m_EndValue - m_StartValue ) * ( 0.5f - cosf( m_ElapsedTime / m_IteratorLength * 3.1415926f ) * 0.5f );
        }



        virtual bool Done()
        {
          return ( m_ElapsedTime >= m_IteratorLength );
        }

    };
  };
};

#endif // ACC_SLOWING_ITERATOR_H



