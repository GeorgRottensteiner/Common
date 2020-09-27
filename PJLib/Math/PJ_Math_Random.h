#ifndef _PJ_MATH_RANDOM_H_INCLUDED
#define _PJ_MATH_RANDOM_H_INCLUDED

#include <math.h>
#include <PJLib\Math\PJ_functional.h>

  namespace math
  {



    // //- float oder double im Bereich 0.0 - 1.0 zurückgeben
    // template < class T >
      // inline float rnd() { return T( rand() ) / RAND_MAX; }
    //- float oder double im Bereich 0.0 - 1.0 zurückgeben
    template < class T >
      inline float rnd() { return T( rand() ) / RAND_MAX; }

    //- float oder double im Bereich 0.0 - range zurückgeben
    template < class T >
      inline T rndLaPlace( const T& range ) { return range * rand() / RAND_MAX; }

    inline float GaussDichte( const float& f, 
                              const float& ErwartungsWert     = 0.0f,
                              const float& StandardAbweichung = 1.0f ) 
    {
      // return exp( -( ( f - ErwartungsWert ) * ( f - ErwartungsWert ) ) / 2.0f / ( StandardAbweichung * StandardAbweichung ) ) 
                          // / sqrt( 2 * 3.1415f ) / StandardAbweichung;
    }

    //- float oder double im Bereich 0.0 - range zurückgeben
    template < class T >
      inline T rndGauss( const T& range, const int level = 5 ) 
      { 
        float x1;
        float x2;
        float y1;
        // float y2;
        T f = -1000;

        // while( !Inside_CO<T>( 0, f, range ) )  
        // while( ( f < 0 ) || ( f >= range ) )  
        {
          x1 = rnd<float>();
          x2 = rnd<float>();
          y1 = (float)( sqrt( - 2 * log(x1) ) * cos( 2 *3.1415 * x2 ) );
          // y2 = sqrt( - 2 * log(x1) ) * sin( 2 *3.1415 *x2 );

          f = (float)( ( y1 + 3.1415 ) / 2 / 3.1415 * range );
        }

        return f;
      }

    
    
    
    
    inline float rndAbweichungLaPlace( const float& normalizedPercents )
    {
      return 1.0f - normalizedPercents + 2 * rnd<float>() * normalizedPercents;
    }

    inline float rndAbweichungGauss( const float& normalizedPercents, const int level = 5 )
    {
      // int iLevel = level;
      // if ( iLevel < 1 ) iLevel = 1;
      // float tRnd = 0;
      // // for ( int iCounter = 1; iCounter <= iLevel; ++iCounter )
      // for ( int iCounter = 1; iCounter <= 2; ++iCounter )
        // tRnd += rnd<float>();

     
    
      float t = rnd<float>();
  
      float result = (float)( exp( ( t * t ) / -2 ) / sqrt( 2 * 3.1415 ) );

      // if ( !Inside_CO

      result =  Limit_CC<float>( 0.0f, ( result / 2 + 0.5f ), 1.0f );
      // return result;
      

      // return 1.0f - normalizedPercents + 2 * normalizedPercents * tRnd / iLevel;
      return 1.0f - normalizedPercents + 2 * normalizedPercents * result / 2;
    }



    /////////////////////////////////////////////////////////////////////
    //-  
    //-    Funktoren
    //-  
    /////////////////////////////////////////////////////////////////////
    
    class CRndFunctor
    {
    public:
      CRndFunctor() {}
      CRndFunctor( const CRndFunctor& rhs ) {}
      CRndFunctor& operator=( const CRndFunctor& rhs ) { return *this; };
      virtual ~CRndFunctor() {}
      virtual CRndFunctor* clone()                                const = 0;
      virtual float operator()()                                      const = 0;
      virtual float operator()( const float& range )                      const = 0;
      virtual float Abweichung( const float& normalizedPercents ) const = 0;
    };
    
    class CRndFunctorLaPlace : public CRndFunctor
    {
    public:
      CRndFunctorLaPlace() : CRndFunctor()  {}
      CRndFunctorLaPlace( const CRndFunctorLaPlace& rhs ) : CRndFunctor( rhs )  {}
      CRndFunctorLaPlace& operator=( const CRndFunctorLaPlace& rhs ) { return *this; }
      virtual ~CRndFunctorLaPlace() {}
      virtual CRndFunctor* clone()        const { return new CRndFunctorLaPlace( *this ); }
      virtual float operator()()                 const { return rnd<float>(); }
      virtual float operator()( const float& range ) const { return rndLaPlace<float>( range ); }
      virtual float Abweichung( const float& normalizedPercents ) const 
      { return rndAbweichungLaPlace( normalizedPercents ); }
    };
    
    class CRndFunctorGauss : public CRndFunctor
    {
    private:
      long m_iSteps; //Anzahl der Laplace-Aufrufe, um Gausswert zu generieren
    public:
      CRndFunctorGauss( const long steps = 5 ) : CRndFunctor(), m_iSteps( steps ) {}
      CRndFunctorGauss( const CRndFunctorGauss& rhs ) : CRndFunctor( rhs ), m_iSteps( rhs.m_iSteps ) {}
      CRndFunctorGauss& operator=( const CRndFunctorGauss& rhs ) { return *this; }
      virtual ~CRndFunctorGauss() {}
      virtual CRndFunctor* clone()        const { return new CRndFunctorGauss( *this ); }
      virtual float operator()()                 const { return rndGauss<float>( 1, m_iSteps  ); }
      virtual float operator()( const float& range ) const { return rndGauss<float>( range, m_iSteps ); }
      virtual float Abweichung( const float& normalizedPercents ) const 
      { return rndAbweichungGauss( normalizedPercents, m_iSteps  ); }
      inline void steps( const long steps ) { m_iSteps = steps; } 
      inline long steps() { return m_iSteps; } 
    };
    

    class CRandomValue
    {
    private:
      float        m_Value;
      float        m_Delta;
      CRndFunctor* m_pRand; //- wird ja garnicht freigegeben!

    public:
      CRandomValue( const float& value, const float& delta = 0, CRndFunctor* rand = NULL )
      : m_Value( value ), m_Delta( delta ), m_pRand( rand )
      {
        if ( !m_pRand ) m_pRand = new CRndFunctorLaPlace();
        if ( m_Delta < 0 ) m_Delta = -m_Delta;
      }

      CRandomValue( const CRandomValue& rhs )
      : m_Value( rhs.m_Value ),
        m_Delta( rhs.m_Delta ),
        m_pRand( rhs.m_pRand->clone() )
      {}

      CRandomValue& operator=( const CRandomValue& rhs )
      {
        if ( this == &rhs ) return *this;
        m_Value = rhs.m_Value;
        m_Delta = rhs.m_Delta;
        m_pRand = rhs.m_pRand->clone();
        return *this;
      }

      CRandomValue& operator=( const float& value )
      {
        m_Value = value;
        return *this;
      }

      inline operator float() const 
      {
        return m_Value - m_Delta + (*m_pRand)( 2 * m_Delta );
      }

      inline float generate() const 
      {
        return m_Value - m_Delta + (*m_pRand)( 2 * m_Delta );
      }

      inline void value( const float& value ) { m_Value = value; }
      inline float value() const              { return m_Value; }
      inline void delta( const float& delta ) { m_Delta = delta; if ( m_Delta < 0 ) m_Delta = -m_Delta; }
      inline float delta() const              { return m_Delta; }
    };

    



    //- Unausgegorene Rand-Klasse
    class CRand
    {
    private:
      unsigned long m_iSeed;
			const long    m_iP;
			const long    m_iA;

    public:
      CRand( const unsigned long seed = 0 )
        : m_iSeed( seed & 0x0000FFFF ),
          m_iP( 1234567 ),
          m_iA( 1234    )
      {}

      CRand( const CRand& rhs )
        : m_iSeed( rhs.m_iSeed ),
          m_iP( rhs.m_iP ),
          m_iA( rhs.m_iA )
      {}

      CRand& operator=( const CRand& rhs )
      {
        if ( this != &rhs )
        {
          m_iSeed = rhs.m_iSeed;
        }
        return *this;
      }

      void          seed( const unsigned long seed ) { m_iSeed = seed & 0x0000FFFF; }
      unsigned long seed() const                     { return m_iSeed; }

      unsigned long operator()( const unsigned long bereich )
      { 
        m_iSeed = ( m_iA * m_iSeed + m_iA ) % m_iP;
	      return ( bereich * m_iSeed ) / m_iP;
      }

    };
  };


#endif// _PJ_MATH_RANDOM_H_INCLUDED
