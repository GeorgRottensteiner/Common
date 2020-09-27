#ifndef POSITIVE_SINE_SWING_ITERATOR_H
#define POSITIVE_SINE_SWING_ITERATOR_H


// Positive Sine Swing for e.g. jump curves



#include "IMathIterator.h"



namespace math
{
  namespace Iterator
  {
    class PositiveSineSwing : public IMathIterator<float>
    {

      protected:

        float       m_Radius,
                    m_Offset,
                    m_SinusValue,
                    m_SwingLength;


      public:

        PositiveSineSwing()
        {
          m_Radius          = 1.0f;
          m_SinusValue      = 0.0f;
          m_Offset          = 0.0f;
          m_SwingLength     = 1.0f;
        }



        PositiveSineSwing( GR::f32 Radius, GR::f32 SwingLength = 1.0f, GR::f32 Offset = 0.0f )
        {
          SetValues( Radius, SwingLength, Offset );
        }



        void SetValues( GR::f32 Radius, GR::f32 SwingLength = 1.0f, GR::f32 Offset = 0.0f )
        {
          m_Radius          = Radius;
          m_Offset          = Offset;
          m_SinusValue      = 0.0f;
          m_SwingLength     = SwingLength;
          if ( m_SwingLength <= 0.0f )
          {
            m_SwingLength = 1.0f;
          }
        }



        float NextValue( const GR::f32 ElapsedTime = 1.0f )
        {
          m_SinusValue += ElapsedTime;
          return Value();
        }



        virtual GR::f32 Reset()
        {
          m_SinusValue = 0.0f;
          return Value();
        }



        virtual bool Done()
        {
          return false;
        }



        GR::f32 Value()
        {
          return (GR::f32)( m_Offset + m_Radius * fabsf( sin( m_SinusValue * 180.0f / m_SwingLength * 3.1415926f / 180.0f ) ) );
        }

    };
  };
};

#endif // POSITIVE_SINE_SWING_ITERATOR_H



