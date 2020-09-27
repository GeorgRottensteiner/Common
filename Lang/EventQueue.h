#pragma once

#include <Interface/IEventListener.h>
#include <Interface/IEventQueue.h>

#include <GR/GRTypes.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Lang/GlobalAllocator.h>
#endif

#include <Lang/FastDelegate.h>

#include <String/ParamList.h>

#include <string>
#include <list>
#include <map>
#include <set>


#define GLOBAL_QUEUE    EventQueue::Instance()


class EventQueue : public IGlobalEventListener,
                   public IEventQueue
{

  protected:

#if OPERATING_SYSTEM == OS_WINDOWS
    typedef std::list<IGlobalEventListener*, GR::STL::global_allocator<IGlobalEventListener*> >    tListListener;

    typedef std::set< IGlobalEventListener*, 
                      std::less< IGlobalEventListener* >, 
                      GR::STL::global_allocator< IGlobalEventListener* > >      tSetListener;

    typedef std::list<tGlobalEvent, GR::STL::global_allocator<tGlobalEvent> >   tListEvents;
#else
    typedef std::list<IGlobalEventListener*>      tListListener;

    typedef std::set<IGlobalEventListener*>       tSetListener;

    typedef std::list<tGlobalEvent>               tListEvents;

#endif

    typedef fastdelegate::FastDelegate1<const tGlobalEvent&>                    tEventHandler;
    typedef std::list<tEventHandler>                                            tEventHandlers;

    typedef std::map<GR::String, GR::u32>                                       tMapRegisteredEvents;

    


    GR::u32                 m_LastEventType;

    GR::u32                 m_ETAddListener;

    GR::u32                 m_ETRemoveListener;

    tListListener           m_listListener;
    tListListener           m_listListenersToAdd;
    tSetListener            m_setListenersToRemove;

    tListEvents             m_listEvents;
    tListEvents             m_listStoredEvents;

    tMapRegisteredEvents    m_mapRegisteredEvents;

    tEventHandlers          m_EventHandler;

    bool                    m_bProcessingEvents;

    bool                    m_bChanged;


  public:


    static EventQueue& Instance();

    virtual ~EventQueue();

    GR::u32                 RegisterEvent( const GR::Char* Event );
    void                    UnregisterEvent( const GR::u32 Event );

    GR::u32                 RegisteredEvent( const GR::Char* Event ) const;

    void                    AddListener( IGlobalEventListener* pListener );
    void                    RemoveListener( IGlobalEventListener* pListener );

    void                    AddHandler( tEventHandler Handler );
    void                    RemoveHandler( tEventHandler Handler );

    void                    PostEvent( const tGlobalEvent& Event );
    void                    PostEvent( const GR::Char* EventName, const tGlobalEvent& Event );
    void                    PostEvent( const GR::Char* EventName, const GR::Char* Param, 
                                       const GR::up dwParam1 = 0, 
                                       const GR::up dwParam2 = 0, 
                                       const GR::up dwParam3 = 0, 
                                       const GR::up dwParam4 = 0 );
    void                    PostEvent( const GR::Char* EventName, 
                                       const GR::up dwParam1 = 0, 
                                       const GR::up dwParam2 = 0, 
                                       const GR::up dwParam3 = 0, 
                                       const GR::up dwParam4 = 0 );

    void                    SendEvent( const tGlobalEvent& Event );
    void                    SendEvent( const GR::Char* EventName, const GR::Char* Param,
                                       const GR::up dwParam1 = 0, const GR::up dwParam2 = 0,
                                       const GR::up dwParam3 = 0, const GR::up dwParam4 = 0 );
    void                    SendEvent( const GR::Char* EventName, const tGlobalEvent& Event = tGlobalEvent() );

    void                    ProcessQueue();

    virtual void            ProcessEvent( const tGlobalEvent& Event );


  protected:

    EventQueue();

};
