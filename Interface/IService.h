#ifndef ISERVICE_H
#define ISERVICE_H

#include <string>
#include <map>

#include <Lang/FastDelegate.h>



namespace GR
{
  struct IEnvironment;

  struct IService
  {

    public:

      virtual ~IService()
      {
      }

      // notification from any other service
      virtual bool          OnServiceNotify( const char*, GR::IService* ) = 0;

      // called if any other service got set
      virtual void          OnServiceGotSet( const char*, GR::IService* ) = 0;

      // called if any other service got unset
      virtual void          OnServiceGotUnset( const char*, GR::IService* ) = 0;

      // called if set
      virtual void          OnSet() = 0;

      // called if unset
      virtual void          OnUnset() = 0;

    public:

      virtual bool          NotifyService( const char* ServiceName, const char* Event ) = 0;
      virtual void          SetEnvironment( IEnvironment* pEnvironment ) = 0;

      friend struct IEnvironment;

  };

  struct IEnvironment
  {

    public:

      enum EnvironmentEvent
      {
        EE_INVALID,
        EE_SERVICE_SET,
        EE_SERVICE_REMOVED,
        EE_SERVICE_NOTIFY
      };


      typedef fastdelegate::FastDelegate4<EnvironmentEvent,const char*,const char*,IService*>    tEnvironmentEventHandler;


      virtual ~IEnvironment()
      {
      }

      virtual void          SetService( const char* strName, IService* pService ) = 0;
      virtual void          RemoveService( const char* strName ) = 0;
      virtual IService*     Service( const char* strName ) = 0;
      virtual const char*   ServiceName( IService* pService ) const = 0;
      virtual bool          NotifyService( const char* ServiceReceiverName, const char* Event, IService* pServiceSender = NULL ) = 0;

      virtual void          AddHandler( tEnvironmentEventHandler Handler ) = 0;
      virtual void          RemoveHandler( tEnvironmentEventHandler Handler ) = 0;
      virtual void          SendEvent( EnvironmentEvent Event, const char* ServiceName, const char* ServiceEvent, IService* pService ) = 0;

  };
}    // namespace GR

#endif// ISERVICE_H

