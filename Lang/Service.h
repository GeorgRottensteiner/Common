#ifndef SERVICE_H
#define SERVICE_H

#include <Interface/IService.h>

#include <list>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Service
  {
    struct Service : public GR::IService
    {

      public:

        IEnvironment*       m_pEnvironment;


        Service();

        virtual bool        OnServiceNotify( const char*, GR::IService* );
        virtual void        OnServiceGotSet( const char*, GR::IService* );
        virtual void        OnServiceGotUnset( const char*, GR::IService* );
        virtual void        OnSet();
        virtual void        OnUnset();


      public:

        bool                NotifyService( const char* ServiceName, const char* Event );
        void                SetEnvironment( IEnvironment* pEnvironment );

        friend struct Environment;

    };

    template <typename BASE> struct ServiceImpl : public BASE
    {

      public:

        IEnvironment*       m_pEnvironment;


        ServiceImpl() :
          m_pEnvironment( NULL )
        {
        }

        bool OnServiceNotify( const char*, GR::IService* )
        {
          return false;
        }

        void OnServiceGotSet( const char*, GR::IService* )
        {
        }

        void OnServiceGotUnset( const char*, GR::IService* )
        {
        }

        void OnSet()
        {
        }

        void OnUnset()
        {
        }

        bool NotifyService( const char* ServiceName, const char* Event )
        {
          if ( m_pEnvironment == NULL )
          {
            return false;
          }
          return m_pEnvironment->NotifyService( ServiceName, Event, this );
        }

        void SetEnvironment( IEnvironment* pEnvironment )
        {
          m_pEnvironment = pEnvironment;
        }

        friend struct Environment;

    };

    struct Environment : public GR::IEnvironment
    {
      private:

        typedef std::list<tEnvironmentEventHandler>     tEventHandlers;


        tEventHandlers      m_EventHandlers;              



        Environment();



      protected:

        typedef std::map<GR::String,IService*>      tServices;


        tServices           m_Services;


      public:



        void                SetService( const char* Name, IService* pService );
        void                RemoveService( const char* Name );
        IService*           Service( const char* Name );
        const char*         ServiceName( IService* pService ) const;
        bool                NotifyService( const char* ServiceReceiverName, const char* Event, IService* pServiceSender = NULL );

        void                AddHandler( tEnvironmentEventHandler Handler );
        void                RemoveHandler( tEnvironmentEventHandler Handler );
        void                SendEvent( EnvironmentEvent Event, const char* ServiceName, const char* ServiceEvent, IService* pService );

        static IEnvironment& Instance();
    };
  }   // namespace Service
}     // namespace GR

#endif // SERVICE_H

