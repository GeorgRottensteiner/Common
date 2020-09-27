#ifndef PROBABILITY_CONTAINER_H
#define PROBABILITY_CONTAINER_H

#include <math/random.h>

#include <vector>



namespace math
{
  template<class T> class ProbabilityContainer
  {
    protected:

      typedef std::vector<std::pair<GR::u32,T> >    tContainer;


      tContainer                m_Container;

      GR::u32                   m_CompleteProbability;

      math::random              m_RandomGenerator;


    public:

      ProbabilityContainer() :
        m_CompleteProbability( 0 )
      {
      }



      void AddElement( const GR::u32 Probability, const T& Value )
      {
        m_CompleteProbability += Probability;
        m_Container.push_back( std::make_pair( Probability, Value ) );
      }



      const T& InvalidElement() const
      {
        static T DummyElement;

        return DummyElement;
      }



      const GR::u32 Probability( const size_t Entry )
      {
        if ( Entry >= m_Container.size() )
        {
          return 0;
        }
        return m_Container[Entry].first;
      }



      const T& Entry( const size_t dwEntry )
      {
        if ( dwEntry >= m_Container.size() )
        {
          return InvalidElement();
        }
        return m_Container[dwEntry].second;
      }



      size_t EntryCount() const
      {
        return m_Container.size();
      }



      const T& ElementAt( GR::u32 Chance )
      {
        if ( Chance >= m_CompleteProbability )
        {
          return InvalidElement();
        }

        tContainer::iterator    it( m_Container.begin() );
        while ( it != m_Container.end() )
        {
          const std::pair<GR::u32,T>&   PairElement = *it;

          if ( Chance < PairElement.first )
          {
            return PairElement.second;
          }

          Chance -= PairElement.first;

          ++it;
        }
        return InvalidElement();
      }



      void Clear()
      {
        m_CompleteProbability = 0;
        m_Container.clear();
      }



      void Seed( const GR::u32 Seed )
      {
        m_RandomGenerator.seed( Seed );
      }



      const T& RandomElement()
      {
        if ( m_CompleteProbability == 0 )
        {
          return InvalidElement();
        }
        GR::u32 chance = m_RandomGenerator.rand( m_CompleteProbability );

        return ElementAt( chance );
      }


  };

}

#endif //PROBABILITY_CONTAINER_H