#ifndef _IEVENTLISTENER_H
#define _IEVENTLISTENER_H


#include <list>

template <typename T> class IEventListener
{

  public:

    virtual ~IEventListener() {};

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void      PostEventOnQueue( const T& Event ) = 0;

    // diese Routine überschreiben, um Events zu bearbeiten
    virtual bool      ProcessEvent( const T& Event ) = 0;

    virtual void      ProcessQueue() = 0;
    
};



template <typename T> class EventListener : public IEventListener<T>
{

  protected:

    std::list<T>      m_listPostedEvents;


  public:

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void      PostEventOnQueue( const T& Event )
    {
      m_listPostedEvents.push_back( Event );
    }

    using IEventListener<T>::ProcessEvent;

    virtual void      ProcessQueue()
    {
      typename std::list<T>::iterator    it( m_listPostedEvents.begin() );
      while ( it != m_listPostedEvents.end() )
      {
        if ( ProcessEvent( *it ) )
        {
          it = m_listPostedEvents.erase( it );
          return;
        }

        it = m_listPostedEvents.erase( it );
      }
    }
    
};


template <typename T> class EventListenerImpl
{

  protected:

    std::list<T>      m_listPostedEvents;


  public:

    virtual bool      ProcessEvent( const T& Event ) = 0;

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void      PostEventOnQueue( const T& Event )
    {
      m_listPostedEvents.push_back( Event );
    }

    virtual void      ProcessQueue()
    {
      typename std::list<T>::iterator    it( m_listPostedEvents.begin() );
      while ( it != m_listPostedEvents.end() )
      {
        if ( ProcessEvent( *it ) )
        {
          it = m_listPostedEvents.erase( it );
          return;
        }

        it = m_listPostedEvents.erase( it );
      }
    }
    
};


#endif// _IEVENTLISTENER_H

