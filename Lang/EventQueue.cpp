#include "EventQueue.h"

#include <debug/debugclient.h>



EventQueue& EventQueue::Instance()
{
  static EventQueue    g_Instance;

  return g_Instance;
}


EventQueue::EventQueue() :
  m_bProcessingEvents( false ),
  m_LastEventType( 0 ),
  m_bChanged( false )
{
  m_ETAddListener     = RegisterEvent( "Queue.AddListener" );
  m_ETRemoveListener  = RegisterEvent( "Queue.RemoveListener" );

  AddListener( this );
}



EventQueue::~EventQueue()
{
  UnregisterEvent( m_ETAddListener );
  UnregisterEvent( m_ETRemoveListener );

  if ( m_mapRegisteredEvents.size() )
  {
    dh::Log( "EventQueue %d Events not unregistered!", m_mapRegisteredEvents.size() );
    tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.begin() );
    while ( it != m_mapRegisteredEvents.end() )
    {
      dh::Log( "Event %s", it->first.c_str() );

      ++it;
    }
  }

  m_listEvents.clear();
  m_listListener.clear();
  m_mapRegisteredEvents.clear();
}



GR::u32 EventQueue::RegisteredEvent( const GR::Char* strEvent ) const
{
  tMapRegisteredEvents::const_iterator    it( m_mapRegisteredEvents.find( strEvent ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    return -1;
  }
  return it->second;
}



GR::u32 EventQueue::RegisterEvent( const GR::Char* strEvent )
{
  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.find( strEvent ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    m_LastEventType++;
    m_mapRegisteredEvents[strEvent] = m_LastEventType;

    return m_LastEventType;
  }

  return it->second;
}



void EventQueue::UnregisterEvent( const GR::u32 Event )
{
  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.begin() );
  while ( it != m_mapRegisteredEvents.end() )
  {
    if ( it->second == Event )
    {
      m_mapRegisteredEvents.erase( it );
      return;
    }

    ++it;
  }
}



void EventQueue::AddListener( IGlobalEventListener* pListener )
{
  tListListener::iterator   it( m_listListener.begin() );
  while ( it != m_listListener.end() )
  {
    if ( *it == pListener )
    {
      return;
    }

    ++it;
  }
  if ( m_bProcessingEvents )
  {
    m_bChanged = true;
    m_listListenersToAdd.push_back( pListener );
    return;
  }
  m_listListener.push_back( pListener );
}



void EventQueue::RemoveListener( IGlobalEventListener* pListener )
{
  if ( m_bProcessingEvents )
  {
    m_bChanged = true;
    pListener->m_bRemoved = true;
    m_setListenersToRemove.insert( pListener );
    return;
  }

  tListListener::iterator   it( m_listListener.begin() );
  while ( it != m_listListener.end() )
  {
    if ( *it == pListener )
    {
      m_listListener.erase( it );
      return;
    }

    ++it;
  }
}



void EventQueue::PostEvent( const tGlobalEvent& Event )
{
  if ( m_bProcessingEvents )
  {
    m_listStoredEvents.push_back( Event );
  }
  else
  {
    m_listEvents.push_back( Event );
  }
}



void EventQueue::PostEvent( const GR::Char* strEventName, const GR::Char* strParam, 
                             const GR::up dwParam1, 
                             const GR::up dwParam2, 
                             const GR::up dwParam3, 
                             const GR::up dwParam4 )
{
  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.find( strEventName ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    dh::Log( "Event %s not found (%d Events registered)", strEventName, m_mapRegisteredEvents.size() );
    return;
  }
  tGlobalEvent    tempEvent( 0, strParam, dwParam1, dwParam2, dwParam3, dwParam4 );

  tempEvent.m_Type = it->second;

  PostEvent( tempEvent );
}



void EventQueue::PostEvent( const GR::Char* strEventName, 
                             const GR::up dwParam1, 
                             const GR::up dwParam2, 
                             const GR::up dwParam3, 
                             const GR::up dwParam4 )
{
  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.find( strEventName ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    return;
  }
  tGlobalEvent    tempEvent( 0, dwParam1, dwParam2, dwParam3, dwParam4 );

  tempEvent.m_Type = it->second;

  PostEvent( tempEvent );
}



void EventQueue::SendEvent( const tGlobalEvent& Event )
{
  tListListener::iterator   it( m_listListener.begin() );
  while ( it != m_listListener.end() )
  {
    IGlobalEventListener*   pListener = *it;

    if ( !pListener->m_bRemoved )
    {
      pListener->ProcessEvent( Event );
    }

    ++it;
  }
}



void EventQueue::PostEvent( const GR::Char* strEventName, const tGlobalEvent& Event )
{
  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.find( strEventName ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    return;
  }
  tGlobalEvent    tempEvent( Event );

  tempEvent.m_Type = it->second;

  PostEvent( tempEvent );
}



void EventQueue::SendEvent( const GR::Char* strEventName, const tGlobalEvent& Event )
{
  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.find( strEventName ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    return;
  }
  tGlobalEvent    tempEvent( Event );

  tempEvent.m_Type = it->second;

  SendEvent( tempEvent );
}



void EventQueue::SendEvent( const GR::Char* strEventName, const GR::Char* strParam,
                             const GR::up dwParam1, const GR::up dwParam2,
                             const GR::up dwParam3, const GR::up dwParam4 )
{

  tMapRegisteredEvents::iterator    it( m_mapRegisteredEvents.find( strEventName ) );
  if ( it == m_mapRegisteredEvents.end() )
  {
    return;
  }
  tGlobalEvent    tempEvent( 0, strParam );

  tempEvent.m_Type = it->second;
  tempEvent.m_Param1  = dwParam1;
  tempEvent.m_Param2  = dwParam2;
  tempEvent.m_Param3  = dwParam3;
  tempEvent.m_Param4  = dwParam4;

  SendEvent( tempEvent );

}



void EventQueue::ProcessQueue()
{
  m_bProcessingEvents = true;

  tListListener::iterator   it( m_listListener.begin() );
  while ( it != m_listListener.end() )
  {
    IGlobalEventListener*   pListener = *it;

    if ( m_setListenersToRemove.find( pListener ) != m_setListenersToRemove.end() )
    {
      ++it;
      continue;
    }

    tListEvents::iterator   itEvent( m_listEvents.begin() );
    while ( itEvent != m_listEvents.end() )
    {
      tGlobalEvent& Event = *itEvent;

      pListener->ProcessEvent( Event );

      if ( m_bChanged )
      {
        if ( m_setListenersToRemove.find( pListener ) != m_setListenersToRemove.end() )
        {
          break;
        }
      }

      ++itEvent;
    }

    ++it;
  }
  m_listEvents.clear();

  tListEvents::iterator   itEvent( m_listStoredEvents.begin() );
  while ( itEvent != m_listStoredEvents.end() )
  {
    m_listEvents.push_back( *itEvent );

    ++itEvent;
  }
  m_listStoredEvents.clear();

  if ( !m_listListenersToAdd.empty() )
  {
    tListListener::iterator   itA( m_listListenersToAdd.begin() );
    while ( itA != m_listListenersToAdd.end() )
    {
      m_listListener.push_back( *itA );
      ++itA;
    }
    m_listListenersToAdd.clear();
  }
  if ( !m_setListenersToRemove.empty() )
  {
    // alle zu löschenden raus
    tSetListener::iterator   itSetRemove( m_setListenersToRemove.begin() );
    while ( itSetRemove != m_setListenersToRemove.end() )
    {
      IGlobalEventListener*   pListenerToRemove = *itSetRemove;

      m_listListener.remove( pListenerToRemove );

      ++itSetRemove;
    }
    m_setListenersToRemove.clear();
  }
  m_bProcessingEvents = false;
  m_bChanged = false;
}



void EventQueue::ProcessEvent( const tGlobalEvent& Event )
{
  if ( Event.m_Type == m_ETAddListener )
  {
    AddListener( (IGlobalEventListener*)Event.m_Param1 );
  }
  else if ( Event.m_Type == m_ETRemoveListener )
  {
    RemoveListener( (IGlobalEventListener*)Event.m_Param1 );
  }
}



void EventQueue::AddHandler( tEventHandler Handler )
{
  m_EventHandler.push_back( Handler );
}



void EventQueue::RemoveHandler( tEventHandler Handler )
{
  m_EventHandler.remove( Handler );
}
