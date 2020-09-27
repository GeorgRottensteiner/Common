#ifndef MATH_RANDOM_H
#define MATH_RANDOM_H



#include <Interface/IIOStream.h>



namespace math
{

  //- Unausgegorene Rand-Klasse
  class random
  {

    private:

      unsigned long   m_Seed;

		  long            m_P;

		  long            m_A;


    public:

      random( const unsigned long seed = 0 )
        : m_Seed( seed & 0x0000FFFF ),
          m_P( 1234567 ),
          m_A( 1234    )
      {}



      void seed( const unsigned long seed ) 
      { 
        m_Seed = seed & 0x0000FFFF; 
      }



      unsigned long seed() const                     
      { 
        return m_Seed; 
      }



      unsigned long rand( const unsigned long Range )
      { 
        m_Seed = ( m_A * m_Seed + m_A ) % m_P;
	      return ( Range * m_Seed ) / m_P;
      }



      bool Save( IIOStream& IO )
      {
        IO.WriteU32( m_Seed );
        IO.WriteI32( m_P );
        IO.WriteI32( m_A );

        return true;
      }



      bool Load( IIOStream& IO )
      {
        m_Seed  = IO.ReadU32();
        m_P     = IO.ReadI32();
        m_A     = IO.ReadI32();

        return true;
      }

  };
};


#endif // MATH_RANDOM_H
