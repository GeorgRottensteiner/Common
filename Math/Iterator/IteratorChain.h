#ifndef ITERATOR_CHAIN_H
#define ITERATOR_CHAIN_H


#include <vector>

#include <Interface/ISmartPointer.h>

#include <Lang/RefCountObject.h>

#include <debug/debugclient.h>

#include "IMathIterator.h"



namespace math
{
  namespace Iterator
  {
    class Chain
    {

      public:

        struct tIteratorPos
        {
          GR::u32     m_IteratorIndex;
          GR::f32     m_IteratorTime;

          tIteratorPos() :
            m_IteratorIndex( 0 ),
            m_IteratorTime( 0.0f )
          {
          }

          void Reset()
          {
            m_IteratorIndex = 0;
            m_IteratorTime  = 0.0f;
          }
        };



      protected:

        struct tIteratorEntryImpl : public GR::CRefCountObject
        {
          IMathIterator<float>*      m_pIterator;

          tIteratorEntryImpl( IMathIterator<float>* pIterator = NULL ) :
            m_pIterator( pIterator )
          {
          }

          ~tIteratorEntryImpl()
          {
            delete m_pIterator;
          }
        };

        struct tIteratorEntry
        {
          GR::f32                   m_StartDelayTime;
          GR::f32                   m_IteratorLifeTime;
          ISmartPointer<tIteratorEntryImpl>     m_IteratorImpl;
          GR::CRefCountObject       m_refCount;

          tIteratorEntry() :
            m_StartDelayTime( 0.0f ),
            m_IteratorLifeTime( 0.0f ),
            m_IteratorImpl( NULL )
          {
          }
        };

        typedef std::vector<tIteratorEntry>    tVectIterators;


        tVectIterators          m_Iterators;



      public:

        bool                    m_Looping;



        Chain() :
          m_Looping( false )
        {
        }



        void AddIterator( IMathIterator<float>* pIterator, const GR::f32 StartDelay = 0.0f, const GR::f32 LifeTime = 0.0f )
        {
          tIteratorEntry    Entry;

          Entry.m_StartDelayTime    = StartDelay;
          Entry.m_IteratorLifeTime  = LifeTime;
          Entry.m_IteratorImpl      = new tIteratorEntryImpl( pIterator );

          m_Iterators.push_back( Entry );
        }



        void Clear()
        {
          m_Iterators.clear();
        }



        GR::u32 Count() const
        {
          return (GR::u32)m_Iterators.size();
        }



        GR::f32 CurrentValue( const tIteratorPos& Pos )
        {
          if ( m_Iterators.empty() )
          {
            return 0.0f;
          }
          if ( Pos.m_IteratorIndex >= (GR::u32)m_Iterators.size() )
          {
            return m_Iterators.back().m_IteratorImpl->m_pIterator->NextValue( m_Iterators.back().m_IteratorImpl->m_pIterator->Length() );
          }
          IMathIterator<float>* pIterator( m_Iterators[Pos.m_IteratorIndex].m_IteratorImpl->m_pIterator );

          pIterator->Reset();
          return pIterator->NextValue( Pos.m_IteratorTime );
        }



        bool IsDone( const tIteratorPos& Pos )
        {
          if ( m_Iterators.empty() )
          {
            return true;
          }
          if ( m_Looping )
          {
            return false;
          }
          if ( Pos.m_IteratorIndex >= (GR::u32)m_Iterators.size() )
          {
            return true;
          }
          return false;
        }



        bool UpdatePos( tIteratorPos& Pos, const GR::f32 ElapsedTime, bool Looping = false )
        {
          if ( m_Iterators.empty() )
          {
            dh::Log( "IteratorChain::UpdatePos, no iterators in list!" );
            return false;
          }

          GR::f32   TimeLeft = ElapsedTime;

          while ( TimeLeft > 0.0f )
          {
            if ( Pos.m_IteratorIndex >= (GR::u32)m_Iterators.size() )
            {
              if ( !Looping )
              {
                return false;
              }
              else
              {
                Pos.m_IteratorIndex = 0;
              }
            }

            IMathIterator<float>* pIterator( m_Iterators[Pos.m_IteratorIndex].m_IteratorImpl->m_pIterator );

            if ( Pos.m_IteratorTime + TimeLeft >= pIterator->Length() )
            {
              TimeLeft -= pIterator->Length() - Pos.m_IteratorTime;

              Pos.m_IteratorIndex++;
              Pos.m_IteratorTime = 0.0f;
              if ( ( m_Looping )
              &&   ( Pos.m_IteratorIndex >= (GR::u32)m_Iterators.size() ) )
              {
                Pos.m_IteratorIndex = 0;
              }
            }
            else
            {
              Pos.m_IteratorTime += TimeLeft;
              TimeLeft = 0.0f;
            }

            pIterator->Reset();
            pIterator->NextValue( Pos.m_IteratorTime );
          }
          return true;

        }

    };
  }
}


#endif // ITERATOR_CHAIN_H



