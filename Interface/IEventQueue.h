#pragma once

#include <Interface/IEventListener.h>
#include <Lang/Service.h>

#include <GR/GRTypes.h>

#include <Lang/GlobalAllocator.h>

#include <String/ParamList.h>

#include <string>
#include <list>
#include <map>
#include <set>


#define GLOBAL_QUEUE    EventQueue::Instance()



struct tGlobalEvent
{
  GR::u32                     m_Type;
  GR::up                      m_Param1;
  GR::up                      m_Param2;
  GR::up                      m_Param3;
  GR::up                      m_Param4;
  GR::String                  m_Param;

  GR::Strings::ParameterList  m_Params;

  tGlobalEvent( GR::u32 dwType = 0, GR::up dwParam1 = 0, GR::up dwParam2 = 0, GR::up dwParam3 = 0, GR::up dwParam4 = 0 ) :
    m_Type( dwType ),
    m_Param1( dwParam1 ),
    m_Param2( dwParam2 ),
    m_Param3( dwParam3 ),
    m_Param4( dwParam4 )
  {
  }

  tGlobalEvent( GR::u32 dwType, const GR::String& strParam, GR::up dwParam1 = 0, GR::up dwParam2 = 0, GR::up dwParam3 = 0, GR::up dwParam4 = 0 ) :
    m_Type( dwType ),
    m_Param1( dwParam1 ),
    m_Param2( dwParam2 ),
    m_Param3( dwParam3 ),
    m_Param4( dwParam4 ),
    m_Param( strParam )
  {
  }

  tGlobalEvent( GR::u32 dwType, const GR::Strings::ParameterList& ParamList ) :
    m_Type( dwType ),
    m_Param1( 0 ),
    m_Param2( 0 ),
    m_Param3( 0 ),
    m_Param4( 0 ),
    m_Params( ParamList )
  {
  }

};



struct IGlobalEventListener
{

  public:

    IGlobalEventListener() :
      m_bRemoved( false )
    {
    }


    virtual ~IGlobalEventListener()
    {
    }

    virtual void          ProcessEvent( const tGlobalEvent& Event ) = 0;

  private:

    bool                  m_bRemoved;


    friend class EventQueue;

};


class IEventQueue : public GR::Service::Service
{

  public:


    virtual ~IEventQueue()
    {
    }

    virtual GR::u32         RegisterEvent( const GR::Char* Event ) = 0;
    virtual void            UnregisterEvent( const GR::u32 Event ) = 0;

    virtual GR::u32         RegisteredEvent( const GR::Char* Event ) const = 0;

    virtual void            AddListener( IGlobalEventListener* pListener ) = 0;
    virtual void            RemoveListener( IGlobalEventListener* pListener ) = 0;

    virtual void            PostEvent( const tGlobalEvent& Event ) = 0;
    virtual void            PostEvent( const GR::Char* EventName, const tGlobalEvent& Event ) = 0;
    virtual void            PostEvent( const GR::Char* EventName, const GR::Char* Param, 
                               const GR::up dwParam1 = 0, 
                               const GR::up dwParam2 = 0, 
                               const GR::up dwParam3 = 0, 
                               const GR::up dwParam4 = 0 ) = 0;
    virtual void            PostEvent( const GR::Char* EventName, 
                               const GR::up dwParam1 = 0, 
                               const GR::up dwParam2 = 0, 
                               const GR::up dwParam3 = 0, 
                               const GR::up dwParam4 = 0 ) = 0;

    virtual void            SendEvent( const tGlobalEvent& Event ) = 0;
    virtual void            SendEvent( const GR::Char* EventName, const GR::Char* Param,
                               const GR::up dwParam1 = 0, const GR::up dwParam2 = 0,
                               const GR::up dwParam3 = 0, const GR::up dwParam4 = 0 ) = 0;
    virtual void            SendEvent( const GR::Char* EventName, const tGlobalEvent& Event = tGlobalEvent() ) = 0;

    virtual void            ProcessQueue() = 0;


};
