#ifndef OBJECT_COMPONENT_CONTAINER_H
#define OBJECT_COMPONENT_CONTAINER_H

#include <map>
#include <string>

#include <GR/GRTypes.h>

#include <GR/LocalRegistry/LocalRegistry.h>



// Uses as a container template for a component based system
// Usage:
//    Derive the components from ObjectComponentContainer<CHILD,PARAM>::ComponentBase
//    Derive CHILD from ObjectComponentContainer<CHILD,PARAM>


class Level;


namespace GR
{
  namespace ComponentSystem
  {
    enum EventType
    {
      CE_INVALID = 0,
      CE_ADDED,
      CE_REMOVED,
      CE_CUSTOM,
    };
    struct ComponentEvent
    {
      GR::ComponentSystem::EventType      Type;

      GR::i32       Param1;
      GR::i32       Param2;
      GR::f32       ParamF;
      GR::String    Param;


      ComponentEvent( GR::ComponentSystem::EventType Type = CE_INVALID ) :
        Type( Type ),
        Param1( 0 ),
        Param2( 0 ),
        ParamF( 0.0f )
      {
      }
    };
  };
};


template<class CHILD, typename PARAM> class ObjectComponentContainer : public LocalRegistry
{

  public:

    typedef CHILD    ComponentType;
    typedef PARAM    ParameterType;

    class ComponentBase
    {

      public:

        ComponentBase()
        {
        }

        virtual ~ComponentBase()
        {
        }


        virtual void Process( ComponentType& Container, ParameterType& Params, const GR::f32 ElapsedTime )
        {
        }

        virtual void OnComponentEvent( ComponentType& Container, ParameterType& Params, GR::ComponentSystem::ComponentEvent& Event )
        {
        }

    };



  protected:

    typedef std::map<GR::String,ComponentBase*>        tComponents;

    tComponents                                         m_Components;


  public:


    ObjectComponentContainer()
    {
    }

    void AddComponent( const GR::String& Component, ComponentBase* pComponent )
    {

      m_Components[Component] = pComponent;

    }



    void RemoveComponent( const GR::String& Component )
    {

      tComponents::iterator    it( m_Components.find( Component ) );
      if ( it != m_Components.end() )
      {
        m_Components.erase( it );
      }

    }


    ComponentBase* GetComponent( const GR::String& Component )
    {

      tComponents::iterator    it( m_Components.find( Component ) );
      if ( it == m_Components.end() )
      {
        return NULL;
      }
      return it->second;

    }



    void UpdateComponents( ParameterType& Params, const GR::f32 ElapsedTime )
    {
      tComponents::iterator   it( m_Components.begin() );
      while ( it != m_Components.end() )
      {
        ComponentBase*  pComp( it->second );

        pComp->Process( (CHILD&)*this, Params, ElapsedTime );

        ++it;
      }

    }



    bool SendComponentMessage( const GR::String& Component, ParameterType& Params, GR::ComponentSystem::ComponentEvent& Event )
    {

      ComponentBase*    pTarget = GetComponent( Component );
      if ( pTarget == NULL )
      {
        return false;
      }
      pTarget->OnComponentEvent( (CHILD&)*this, Params, Event );
      return true;

    }
};



#endif // OBJECT_COMPONENT_CONTAINER_H