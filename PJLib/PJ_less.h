#ifndef _PJ_LESS_TEMPLATE_H
#define _PJ_LESS_TEMPLATE_H

namespace PJ
{
  /////////////////////////////////////////////////////////////////////
  //-  
  //-    less_triple< class T, valuetype >
  //-  
  /////////////////////////////////////////////////////////////////////
  template < class T, class V >
  class less_triple
  {
  private:
    const V (T::* m_Attrib_1)() const;
    const V (T::* m_Attrib_2)() const;
    const V (T::* m_Attrib_3)() const;

    V m_ValueWeight;

  public:
    less_triple(  const V weight,
                    const V (T::* attrib_1)() const = NULL,
                    const V (T::* attrib_2)() const = NULL,
                    const V (T::* attrib_3)() const = NULL
                  )
    {
      m_ValueWeight = weight;
      m_Attrib_1 = attrib_1;
      m_Attrib_2 = attrib_2;
      m_Attrib_3 = attrib_3;
    }

    bool operator()( const T& item1, const T& item2 )
    {
      if ( &item1 == &item2 ) return false;
      if ( item1  == item2  ) return false;
    
      V Value1 = 0;
      V Value2 = 0;

      if ( m_Attrib_1 != NULL )
      {
        Value1 += (item1.*m_Attrib_1)() * m_ValueWeight * m_ValueWeight;
        Value2 += (item2.*m_Attrib_1)() * m_ValueWeight * m_ValueWeight;
      }
    
      if ( m_Attrib_2 != NULL )
      {
        Value1 += (item1.*m_Attrib_2)() * m_ValueWeight;
        Value2 += (item2.*m_Attrib_2)() * m_ValueWeight;
      }

      if ( m_Attrib_3 != NULL )
      {
        Value1 += (item1.*m_Attrib_3)();
        Value2 += (item2.*m_Attrib_3)();
      }
    
      return Value1 < Value2;
    }
  };

  /////////////////////////////////////////////////////////////////////
  //-  
  //-    less_triple_updown< class T, valuetype >
  //-  
  /////////////////////////////////////////////////////////////////////
  template < class T, class V >
  class less_triple_updown
  {
  private:
    const V (T::* m_Attrib_1)() const;
    const V (T::* m_Attrib_2)() const;
    const V (T::* m_Attrib_3)() const;

    V m_order1;
    V m_order2;
    V m_order3;
  
    V m_ValueWeight;

  public:

    less_triple_updown(   const V weight,
                            const V (T::* attrib_1)() const = NULL,
                            const V order1 = 1,
                            const V (T::* attrib_2)() const = NULL,
                            const V order2 = 1,
                            const V (T::* attrib_3)() const = NULL,
                            const V order3 = 1 )
    {
      m_ValueWeight = weight;
    
      m_Attrib_1 = attrib_1;
      m_Attrib_2 = attrib_2;
      m_Attrib_3 = attrib_3;

      if ( order1 < 0 ) m_order1 = -1;
      else              m_order1 =  1;

      if ( order2 < 0 ) m_order2 = -1;
      else              m_order2 =  1;
    
      if ( order3 < 0 ) m_order3 = -1;
      else              m_order3 =  1;
    }


    bool operator()( const T& item1, const T& item2 )
    {
      if ( &item1 == &item2 ) return false;
      if ( item1  == item2  ) return false;
    
      V Value1 = 0;
      V Value2 = 0;

      if ( m_Attrib_1 != NULL )
      {
        Value1 += (item1.*m_Attrib_1)() * m_order1 * m_ValueWeight * m_ValueWeight;
        Value2 += (item2.*m_Attrib_1)() * m_order1 * m_ValueWeight * m_ValueWeight;
      }
    
      if ( m_Attrib_2 != NULL )
      {
        Value1 += (item1.*m_Attrib_2)() * m_order2 * m_ValueWeight;
        Value2 += (item2.*m_Attrib_2)() * m_order2 * m_ValueWeight;
      }

      if ( m_Attrib_3 != NULL )
      {
        Value1 += (item1.*m_Attrib_3)() * m_order3;
        Value2 += (item2.*m_Attrib_3)() * m_order3;
      }
    
      return Value1 < Value2;
    }
  };

};

#endif //_PJ_LESS_TEMPLATE_H