#ifndef SINUS_ITERATOR_H__
#define SINUS_ITERATOR_H__



#include "IMathIterator.h"




namespace math
{
  namespace Iterator
  {
    class SinusIterator : public IMathIterator<GR::f32>
    {

      protected:

        GR::f32     m_Radius,
                    m_Offset,
                    m_SinusValue,
                    m_Speed;


      public:

        SinusIterator()
        {
          m_Radius     = 1.0f;
          m_SinusValue = 0.0f;
          m_Offset     = 0.0f;
          m_Speed      = 1.0f;
        }

        SinusIterator( GR::f32 Radius, GR::f32 Speed = 1.0f, GR::f32 Offset = 0.0f )
        {
          m_Speed      = Speed;
          m_Radius     = Radius;
          m_SinusValue = 0.0f;
          m_Offset     = Offset;
        }

        void SetValues( GR::f32 Radius, GR::f32 Speed = 1.0f, GR::f32 Offset = 0.0f )
        {
          m_Speed      = Speed;
          m_Radius     = Radius;
          m_SinusValue = 0.0f;
          m_Offset     = Offset;
        }

        GR::f32 NextValue( const GR::f32 ElapsedTime = 1.0f )
        {
          m_SinusValue += m_Speed * ElapsedTime;
          return Value();
        }

        virtual GR::f32 Reset()
        {
          m_SinusValue = 0.0f;
          return 0.0f;
        }

        virtual bool Done()
        {
          return false;
        }

        GR::f32 Value()
        {
          return (GR::f32)( m_Offset + m_Radius * sin( m_SinusValue * 3.1415926f / 180.0f ) );
        }

    };
  };
};

#endif // SINUS_ITERATOR_H__



