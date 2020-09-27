#ifndef IEVENTHANDLER_H
#define IEVENTHANDLER_H


#include <list>

#include <Lang/FastDelegate.h>



template <typename T> class IEventHandler
{

  protected:

    typedef fastdelegate::FastDelegate1<const T&>    tEventHandlerFunction;



  public:

    IEventHandler()
    {
    }



    virtual ~IEventHandler()
    {
	}



    virtual void AddHandler( tEventHandlerFunction Function ) = 0;
    virtual void RemoveHandler( tEventHandlerFunction Function ) = 0;
    virtual void RemoveAllHandlers() = 0;

    // um Events für spätere Bearbeitung (durch ProcessEventQueue) in eine Liste zu stecken
    virtual void PostEvent( const T& Event ) = 0;

    // Events direkt abarbeiten, VORSICHT - werden vor dem Queue abgearbeitet!
    virtual bool SendEvent( const T& Event ) = 0;

    // Event-Liste abarbeiten
    virtual void ProcessEventQueue() = 0;
};




template <typename T> class EventHandler : public IEventHandler<T>
{

  protected:

    std::list<T>                        m_listPostedEvents;

    std::list<typename IEventHandler<T>::tEventHandlerFunction>    m_listEventListener;

    std::list<typename IEventHandler<T>::tEventHandlerFunction>    m_listEventListenerToAdd;

    std::list<typename IEventHandler<T>::tEventHandlerFunction>    m_listEventListenerToRemove;

    bool                                m_bChanged;

    bool                                m_bRemovedAll;

    int                                 m_iSendingEvents;


  public:

    EventHandler() :
      m_bChanged( false ),
      m_bRemovedAll( false ),
      m_iSendingEvents( 0 )
    {
    }



    virtual ~EventHandler()
    {
	  }




    void AddHandler( typename IEventHandler<T>::tEventHandlerFunction Function )
    {
      typename std::list<typename IEventHandler<T>::tEventHandlerFunction >::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        typename IEventHandler<T>::tEventHandlerFunction pOldListener = *it;

        if ( pOldListener == Function )
        {
          return;
        }
        ++it;
      }
      if ( m_iSendingEvents )
      {
        m_listEventListenerToAdd.push_back( Function );
        m_bChanged = true;
      }
      else
      {
        m_listEventListener.push_back( Function );
      }
    }



    void RemoveHandler( typename IEventHandler<T>::tEventHandlerFunction Function )
    {
      typename std::list<typename IEventHandler<T>::tEventHandlerFunction>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        typename IEventHandler<T>::tEventHandlerFunction    pOldListener = *it;

        if ( pOldListener == Function )
        {
          if ( m_iSendingEvents )
          {
            m_listEventListenerToRemove.push_back( Function );
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



    void RemoveAllHandlers()
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
    void PostEvent( const T& Event )
    {
      m_listPostedEvents.push_back( Event );
    }



    // Events direkt abarbeiten, VORSICHT - werden vor dem Queue abgearbeitet!
    bool SendEvent( const T& Event )
    {
      bool    bProcessed = false;


      m_bChanged = false;

      m_iSendingEvents++;
      typename std::list<typename IEventHandler<T>::tEventHandlerFunction>::iterator   it( m_listEventListener.begin() );
      while ( it != m_listEventListener.end() )
      {
        typename IEventHandler<T>::tEventHandlerFunction    pListener = *it;

        pListener( Event );
        if ( m_bRemovedAll )
        {
          break;
        }
        // ist der Listener noch vorhanden?
        if ( m_bChanged )
        {
          bool    bHasBeenRemoved = true;
          typename std::list<typename IEventHandler<T>::tEventHandlerFunction>::iterator   itTemp( m_listEventListener.begin() );
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
        typename std::list<typename IEventHandler<T>::tEventHandlerFunction>::iterator   itRemove( m_listEventListenerToRemove.begin() );
        while ( itRemove != m_listEventListenerToRemove.end() )
        {
          typename IEventHandler<T>::tEventHandlerFunction   pListenerToRemove = *itRemove;

          m_listEventListener.remove( pListenerToRemove );

          ++itRemove;
        }
        m_bChanged = false;
      }
      return bProcessed;
    }



    // Event-Liste abarbeiten
    void ProcessEventQueue()
    {
      m_iSendingEvents++;

      typename std::list<T>::iterator    itEvent( m_listPostedEvents.begin() );
      while ( itEvent != m_listPostedEvents.end() )
      {
        typename std::list<typename IEventHandler<T>::tEventHandlerFunction>::iterator   it( m_listEventListener.begin() );
        while ( it != m_listEventListener.end() )
        {
          typename IEventHandler<T>::tEventHandlerFunction    Function = *it;

          m_bChanged = false;

          Function( *itEvent );

          ++it;
        }
        itEvent = m_listPostedEvents.erase( itEvent );
      }
      m_iSendingEvents--;
    }

};

#endif// IEVENTHANDLER_H

