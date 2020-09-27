#ifndef GR_MATH_ITERATOR_LINEAR_H
#define GR_MATH_ITERATOR_LINEAR_H



#include "IMathIterator.h"




namespace math
{
  namespace Iterator
  {
    class Linear : public IMathIterator<GR::f32>
    {

      protected:

        GR::f32     m_Length,
                    m_Start,
                    m_End,
                    m_CurrentPos;


      public:

        Linear()
        {
          m_Start           = 0.0f;
          m_End             = 0.0f;
          m_IteratorLength  = 1.0f;
          m_CurrentPos      = 0.0f;
        }

        Linear( GR::f32 Start, GR::f32 End, GR::f32 Duration = 1.0f )
        {
          m_Start           = Start;
          m_End             = End;
          m_IteratorLength  = Duration;
          m_CurrentPos      = 0.0f;
        }

        void SetValues( GR::f32 Start, GR::f32 End, GR::f32 Duration = 1.0f )
        {
          m_Start           = Start;
          m_End             = End;
          m_CurrentPos      = 0.0f;
          m_IteratorLength  = Duration;
        }

        GR::f32 NextValue( const GR::f32 ElapsedTime = 1.0f )
        {
          m_CurrentPos += ElapsedTime;
          if ( m_CurrentPos > m_IteratorLength )
          {
            m_CurrentPos = m_IteratorLength;
          }
          return Value();
        }

        virtual GR::f32 Reset()
        {
          m_CurrentPos = 0.0f;
          return 0.0f;
        }

        virtual bool Done()
        {
          if ( m_CurrentPos >= m_IteratorLength )
          {
            return true;
          }
          return false;
        }

        GR::f32 Value()
        {
          return m_Start + ( m_CurrentPos * ( m_End - m_Start ) ) / m_IteratorLength;
        }

    };
  };
};

#endif // GR_MATH_ITERATOR_LINEAR_H



