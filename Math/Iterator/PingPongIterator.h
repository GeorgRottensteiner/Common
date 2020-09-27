#ifndef PINGPONG_ITERATOR_H
#define PINGPONG_ITERATOR_H



#include "IMathIterator.h"



class CPingPongIterator : public IMathIterator<int>
{

  protected:

    int         m_iMin,
                m_iMax,
                m_iPos,
                m_iStep;

    bool        m_bUp;


  public:

    CPingPongIterator()
    {
      m_iMin  = 0;
      m_iMax  = 100;
      m_iStep = 1;
      m_bUp   = true;
      m_iPos  = m_iMin;
    }
    CPingPongIterator( int iMin, int iMax, int iStep = 1 )
    {
      m_iMin  = iMin;
      m_iMax  = iMax;
      m_iStep = iStep;
      m_bUp   = true;
      m_iPos  = m_iMin;
    }

    int NextValue( const float fElapsedTime = 1.0f )
    {
      if ( m_bUp )
      {
        m_iPos += m_iStep;
        if ( m_iPos >= m_iMax )
        {
          m_iPos = m_iMax;
          m_bUp = false;
        }
      }
      else
      {
        m_iPos -= m_iStep;
        if ( m_iPos <= m_iMin )
        {
          m_iPos = m_iMin;
          m_bUp = true;
        }
      }
      return Value();
    }

    int Value()
    {
      return m_iPos;
    }

    virtual int Reset()
    {
      m_bUp   = true;
      m_iPos  = m_iMin;
      return m_iPos;
    }

    virtual bool Done()
    {
      return false;
    }

};



#endif // PINGPONG_ITERATOR_H



