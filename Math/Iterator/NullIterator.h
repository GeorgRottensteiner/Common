#ifndef NULL_ITERATOR_H
#define NULL_ITERATOR_H



#include "IMathIterator.h"



namespace math
{
  namespace Iterator
  {
    class NullIterator : public IMathIterator<float>
    {

      protected:

        GR::f32       m_Value;
        GR::f32       m_CurPos;


      public:

        NullIterator() :
          m_Value( 0.0f ),
          m_CurPos( 0.0f )
        {
          m_IteratorLength = 1.0f;
        }

        NullIterator( GR::f32 Value, GR::f32 Length ) :
          m_Value( Value ),
          m_CurPos( 0.0f )
        {
          m_IteratorLength = Length;
        }

        void SetValues( GR::f32 Value, GR::f32 Length )
        {
          m_Value          = Value;
          m_IteratorLength = Length;
          Reset();
        }

        float NextValue( const GR::f32 ElapsedTime = 1.0f )
        {
          m_CurPos += ElapsedTime;
          return Value();
        }

        float Reset()
        {
          m_CurPos = 0.0f;
          return Value();
        }

        float Value()
        {
          return m_Value;
        }

        virtual bool Done()
        {
          return ( m_CurPos >= m_IteratorLength );
        }

    };
  };
};


#endif // ACC_SLOWING_ITERATOR_H



