#include "Service.h"

#include <debug/debugclient.h>



namespace GR
{

  namespace Service
  {

    Service::Service() :
      m_pEnvironment( NULL )
    {
    }



    bool Service::OnServiceNotify( const char*, GR::IService* )
    {
      return false;
    }



    void Service::OnServiceGotSet( const char*, GR::IService* )
    {
    }



    void Service::OnServiceGotUnset( const char*, GR::IService* )
    {
    }



    void Service::OnSet()
    {
    }



    void Service::OnUnset()
    {
    }



    bool Service::NotifyService( const char* ServiceName, const char* Event )
    {
      return m_pEnvironment->NotifyService( ServiceName, Event, this );
    }



    void Service::SetEnvironment( IEnvironment* pEnvironment )
    {
      m_pEnvironment = pEnvironment;
    }



    Environment::Environment()
    {
    }



    void Environment::SetService( const char* Name, IService* pService )
    {
      if ( m_Services.find( Name ) != m_Services.end() )
      {
        dh::Log( "IEnvironment::SetService Service %s already registered!", Name );
        return;
      }

      //dh::Log( "SetService %s", strName.c_str() );

      m_Services[Name] = pService;

      if ( pService != NULL )
      {
        pService->SetEnvironment( this );
        pService->OnSet();
      }

      // notify other services
      tServices::iterator    it( m_Services.begin() );
      while ( it != m_Services.end() )
      {
        GR::IService*   pOldService = it->second;
      
        if ( pOldService != pService )
        {
          pOldService->OnServiceGotSet( Name, pService );
        }

        ++it;
      }

      SendEvent( GR::Service::Environment::EE_SERVICE_SET, Name, "", pService );
    }



    void Environment::RemoveService( const char* Name )
    {
      tServices::iterator    it( m_Services.find( Name ) );
      if ( it != m_Services.end() )
      {
        GR::IService* pService = it->second;

        pService->OnUnset();
        pService->SetEnvironment( NULL );

        // notify other services
        tServices::iterator    itNotify( m_Services.begin() );
        while ( itNotify != m_Services.end() )
        {
          GR::IService*   pOldService = itNotify->second;
          
          if ( ( pOldService != pService )
          &&   ( pOldService != NULL ) )
          {
            pOldService->OnServiceGotUnset( Name, pService );
          }

          ++itNotify;
        }

        m_Services.erase( it );

        SendEvent( GR::Service::Environment::EE_SERVICE_REMOVED, Name, "", pService );
      }
    }



    IService* Environment::Service( const char* Name )
    {
      tServices::iterator    it( m_Services.find( Name ) );
      if ( it != m_Services.end() )
      {
        return it->second;
      }
      return NULL;
    }



    const char* Environment::ServiceName( IService* pService ) const
    {
      tServices::const_iterator    it( m_Services.begin() );
      while ( it != m_Services.end() )
      {
        if ( it->second == pService )
        {
          return it->first.c_str();
        }

        ++it;
      }
      return "";
    }



    bool Environment::NotifyService( const char* ServiceReceiverName, const char* Event, IService* pServiceSender )
    {
      tServices::iterator    it( m_Services.find( ServiceReceiverName ) );
      if ( it == m_Services.end() )
      {
        dh::Log( "IEnvironment::NotifyService Service %s unknown!", ServiceReceiverName );
        return false;
      }

      GR::IService*   pServiceReceiver = it->second;

      char    temp[2048];

#ifdef __STDC_WANT_SECURE_LIB__
      sprintf_s( temp, 2048, "%s.%s", ServiceName( pServiceSender ), Event );
#else
      sprintf( temp, "%s.%s", ServiceName( pServiceSender ), Event );
#endif
      bool result = pServiceReceiver->OnServiceNotify( temp, pServiceSender );

      SendEvent( GR::Service::Environment::EE_SERVICE_NOTIFY, ServiceReceiverName, Event, pServiceReceiver );

      return result;
    }



    Environment::IEnvironment& Environment::Instance()
    {
      static Environment    g_Instance;

      return g_Instance;
    }



    void Environment::AddHandler( tEnvironmentEventHandler Handler )
    {
      tEventHandlers::iterator    it( m_EventHandlers.begin() );
      while ( it != m_EventHandlers.end() )
      {
        if ( *it == Handler )
        {
          return;
        }
        ++it;
      }
      m_EventHandlers.push_back( Handler );
    }



    void Environment::RemoveHandler( tEnvironmentEventHandler Handler )
    {
      m_EventHandlers.remove( Handler );
    }



    void Environment::SendEvent( EnvironmentEvent Event, const char* ServiceName, const char* ServiceEvent, IService* pService )
    {

      tEventHandlers::iterator    it( m_EventHandlers.begin() );
      while ( it != m_EventHandlers.end() )
      {
        tEnvironmentEventHandler&   Handler( *it );

        if ( Handler )
        {
          Handler( Event, ServiceName, ServiceEvent, pService );
        }
        ++it;
      }

    }
  }

}



