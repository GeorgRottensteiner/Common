#ifndef IMATHITERATOR_H
#define IMATHITERATOR_H

#include <GR/GRTypes.h>



template <class T> class IMathIterator
{

  public:

    bool        m_Looping;

    GR::f32     m_IteratorLength;


    IMathIterator() :
      m_Looping( false ),
      m_IteratorLength( 0.0f )
    {
    }

    virtual ~IMathIterator()
    {
    }

    virtual T Value() = 0;

    virtual T NextValue( const GR::f32 ElapsedTime = 1.0f ) = 0;

    virtual T Reset() = 0;

    GR::f32 Length()
    {
      return m_IteratorLength;
    }

    virtual bool Done() = 0;

};

#endif // IMATHITERATOR_H



