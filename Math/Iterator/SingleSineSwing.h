#ifndef SINGLE_SINE_SWING_ITERATOR_H
#define SINGLE_SINE_SWING_ITERATOR_H


// Single Sine Swing for e.g. jump curves



#include "IMathIterator.h"



namespace math
{
  namespace Iterator
  {
    class SingleSineSwing : public IMathIterator<float>
    {

      protected:

        float       m_Radius,
                    m_Offset,
                    m_SinusValue;


      public:

        SingleSineSwing()
        {
          m_Radius          = 1.0f;
          m_SinusValue      = 0.0f;
          m_Offset          = 0.0f;
          m_IteratorLength  = 1.0f;
        }

        SingleSineSwing( GR::f32 Radius, GR::f32 Duration = 1.0f, GR::f32 Offset = 0.0f )
        {
          SetValues( Radius, Duration, Offset );
        }

        void SetValues( GR::f32 Radius, GR::f32 Duration = 1.0f, GR::f32 Offset = 0.0f )
        {
          m_Radius          = Radius;
          m_IteratorLength  = Duration;
          m_Offset          = Offset;
          m_SinusValue      = 0.0f;
          if ( m_IteratorLength <= 0.0f )
          {
            m_IteratorLength = 1.0f;
          }
        }

        float NextValue( const GR::f32 ElapsedTime = 1.0f )
        {
          m_SinusValue += ElapsedTime;
          if ( m_SinusValue >= m_IteratorLength )
          {
            m_SinusValue = m_IteratorLength;
          }
          return Value();
        }

        virtual GR::f32 Reset()
        {
          m_SinusValue = 0.0f;
          return Value();
        }

        virtual bool Done()
        {
          return ( m_SinusValue >= m_IteratorLength );
        }

        GR::f32 Value()
        {
          return (GR::f32)( m_Offset + m_Radius * sin( m_SinusValue * 180.0f / m_IteratorLength * 3.1415926f / 180.0f ) );
        }

    };
  };
};

#endif // SINGLE_SINE_SWING_ITERATOR_H



