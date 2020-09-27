#ifndef IEVENTPRODUCER_H
#define IEVENTPRODUCER_H

#include "IEventListener.h"



template <typename T> class IEventProducer
{

  public:

    virtual ~IEventProducer() {};


    virtual void              AddListener( IEventListener<T>* pListener ) = 0;
    virtual void              RemoveListener( IEventListener<T>* pListener ) = 0;
    virtual void              RemoveAllListeners() = 0;

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void      PostEvent( const T& Event ) = 0;

    // Events direkt abarbeiten, VORSICHT - werden vor dem Queue abgearbeitet!
    virtual bool      SendEvent( const T& Event ) = 0;

    // Event-Liste abarbeiten
    virtual void      ProcessEventQueue() = 0;

};



template <typename T> class EventProducer : public IEventProducer<T>
{

  protected:

    std::list<IEventListener<T>*>       m_listEventListener;

    std::list<IEventListener<T>*>       m_listEventListenerToAdd;

    std::list<IEventListener<T>*>       m_listEventListenerToRemove;

    bool                                m_bChanged;

    bool                                m_bRemovedAll;

    int                                 m_iSendingEvents;


  public:

    EventProducer() :
      m_bChanged( false ),
      m_bRemovedAll( false ),
      m_iSendingEvents( 0 )
    {
    }

    virtual void              AddListener( IEventListener<T>* pListener )
    {
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pOldListener = *it;

        if ( pOldListener == pListener )
        {
          return;
        }
        ++it;
      }
      if ( m_iSendingEvents )
      {
        m_listEventListenerToAdd.push_back( pListener );
        m_bChanged = true;
      }
      else
      {
        m_listEventListener.push_back( pListener );
      }
    }

    virtual void              RemoveListener( IEventListener<T>* pListener )
    {
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pOldListener = *it;

        if ( pOldListener == pListener )
        {
          if ( m_iSendingEvents )
          {
            m_listEventListenerToRemove.push_back( pListener );
            m_bChanged = true;
          }
          else
          {
            m_listEventListener.erase( it );
          }
          return;
        }
        ++it;
      }
    }

    virtual void              RemoveAllListeners()
    {
      if ( m_iSendingEvents )
      {
        m_listEventListenerToRemove = m_listEventListener;
        m_bChanged = true;
        m_bRemovedAll = true;
      }
      else
      {
        m_listEventListener.clear();
      }
    }

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void      PostEvent( const T& Event )
    {
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pListener = *it;

        pListener->PostEventOnQueue( Event );

        ++it;
      }
    }

    // Events direkt abarbeiten, VORSICHT - werden vor dem Queue abgearbeitet!
    virtual bool      SendEvent( const T& Event )
    {
      bool    bProcessed = false;


      m_bChanged = false;

      m_iSendingEvents++;
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pListener = *it;

        
        if ( pListener->ProcessEvent( Event ) )
        {
          bProcessed = true;
        }
        if ( m_bRemovedAll )
        {
          break;
        }
        // ist der Listener noch vorhanden?
        if ( m_bChanged )
        {
          bool    bHasBeenRemoved = true;
          typename std::list<IEventListener<T>*>::iterator   itTemp( m_listEventListener.begin() );
          while ( itTemp != m_listEventListener.end() )
          {
            if ( *itTemp == pListener )
            {
              // noch da
              bHasBeenRemoved = false;
              break;
            }

            ++itTemp;
          }
          if ( bHasBeenRemoved )
          {
            break;
          }
        }
        ++it;
      }
      m_bRemovedAll = false;
      m_iSendingEvents--;
      if ( m_bChanged )
      {
        // eine Änderung ist vorgenommen worden

        // alle zu addierenden rein
        m_listEventListener.merge( m_listEventListenerToAdd );
        m_listEventListenerToAdd.clear();

        // alle zu löschenden raus
        typename std::list<IEventListener<T>*>::iterator   itRemove( m_listEventListenerToRemove.begin() );
        while ( itRemove != m_listEventListenerToRemove.end() )
        {
          IEventListener<T>*   pListenerToRemove = *itRemove;

          m_listEventListener.remove( pListenerToRemove );

          ++itRemove;
        }
        m_bChanged = false;
      }
      return bProcessed;
    }

    // Event-Liste abarbeiten
    virtual void      ProcessEventQueue()
    {
      m_iSendingEvents++;
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pListener = *it;

        m_bChanged = false;

        ++it;
        pListener->ProcessQueue();
      }
      m_iSendingEvents--;
    }
    

};



template <typename T, typename BASE> class EventProducerImpl : public BASE
{

  protected:

    std::list<IEventListener<T>*>       m_listEventListener;

    std::list<IEventListener<T>*>       m_listEventListenerToAdd;

    std::list<IEventListener<T>*>       m_listEventListenerToRemove;

    bool                                m_bChanged;

    bool                                m_bRemovedAll;

    int                                 m_iSendingEvents;


  public:

    EventProducerImpl() :
      m_bChanged( false ),
      m_bRemovedAll( false ),
      m_iSendingEvents( 0 )
    {
    }

    virtual void              AddListener( IEventListener<T>* pListener )
    {
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pOldListener = *it;

        if ( pOldListener == pListener )
        {
          return;
        }
        ++it;
      }
      if ( m_iSendingEvents )
      {
        m_listEventListenerToAdd.push_back( pListener );
        m_bChanged = true;
      }
      else
      {
        m_listEventListener.push_back( pListener );
      }
    }

    virtual void              RemoveListener( IEventListener<T>* pListener )
    {
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pOldListener = *it;

        if ( pOldListener == pListener )
        {
          if ( m_iSendingEvents )
          {
            m_listEventListenerToRemove.push_back( pListener );
            m_bChanged = true;
          }
          else
          {
            m_listEventListener.erase( it );
          }
          return;
        }
        ++it;
      }
    }

    virtual void              RemoveAllListeners()
    {
      if ( m_iSendingEvents )
      {
        m_listEventListenerToRemove = m_listEventListener;
        m_bChanged = true;
        m_bRemovedAll = true;
      }
      else
      {
        m_listEventListener.clear();
      }
    }

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void      PostEvent( const T& Event )
    {
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pListener = *it;

        pListener->PostEventOnQueue( Event );

        ++it;
      }
    }

    // Events direkt abarbeiten, VORSICHT - werden vor dem Queue abgearbeitet!
    virtual bool      SendEvent( const T& Event )
    {
      bool    bProcessed = false;


      m_bChanged = false;

      m_iSendingEvents++;
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pListener = *it;

        
        if ( pListener->ProcessEvent( Event ) )
        {
          bProcessed = true;
        }
        if ( m_bRemovedAll )
        {
          break;
        }
        // ist der Listener noch vorhanden?
        if ( m_bChanged )
        {
          bool    bHasBeenRemoved = true;
          typename std::list<IEventListener<T>*>::iterator   itTemp( m_listEventListener.begin() );
          while ( itTemp != m_listEventListener.end() )
          {
            if ( *itTemp == pListener )
            {
              // noch da
              bHasBeenRemoved = false;
              break;
            }

            ++itTemp;
          }
          if ( bHasBeenRemoved )
          {
            break;
          }
        }
        ++it;
      }
      m_bRemovedAll = false;
      m_iSendingEvents--;
      if ( m_bChanged )
      {
        // eine Änderung ist vorgenommen worden

        // alle zu addierenden rein
        m_listEventListener.merge( m_listEventListenerToAdd );
        m_listEventListenerToAdd.clear();

        // alle zu löschenden raus
        typename std::list<IEventListener<T>*>::iterator   itRemove( m_listEventListenerToRemove.begin() );
        while ( itRemove != m_listEventListenerToRemove.end() )
        {
          IEventListener<T>*   pListenerToRemove = *itRemove;

          m_listEventListener.remove( pListenerToRemove );

          ++itRemove;
        }
        m_bChanged = false;
      }
      return bProcessed;
    }

    // Event-Liste abarbeiten
    virtual void      ProcessEventQueue()
    {
      m_iSendingEvents++;
      typename std::list<IEventListener<T>*>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        IEventListener<T>*    pListener = *it;

        m_bChanged = false;

        ++it;
        pListener->ProcessQueue();
      }
      m_iSendingEvents--;
    }
    

};

#endif// IEVENTPRODUCER_H

