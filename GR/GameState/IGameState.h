#pragma once



#include <list>

#include <Interface/IEventListener.h>

#include <Interface/ICloneAble.h>

#include <Lang/EventQueue.h>
#include <Lang/FastDelegate.h>

#include <Controls/OutputEvent.h>

#include <String/ParamList.h>

#include <debug/debugclient.h>



template <typename T> class IGameStateManager;

template <typename T> class IGameState : public ICloneAble
{

  private:

    bool                        m_IsFlaggedForRemoval;



  public:

    IGameStateManager<T>*       m_pStateManager;

    GR::Strings::ParameterList  m_ParamInit;
    bool                        m_IsFreezingStack;
    bool                        m_IsDecorativeState;
    bool                        m_StateIsNotUpdatedWhenInactive;



    IGameState() :
      m_pStateManager( NULL ),
      m_IsFreezingStack( true ),
      m_IsDecorativeState( false ),
      m_StateIsNotUpdatedWhenInactive( true ),
      m_IsFlaggedForRemoval( false )
    {
    }



    virtual ~IGameState()
    {
    }

    
    
    virtual void Init()
    {
    }



    virtual void Exit()
    {
    }



    virtual void Display( T& Viewer ) = 0;



    virtual void UpdatePerDisplayFrame( const float )
    {
    }
    
    
    
    virtual void Update()
    {
    }

    
    
    
    virtual void OnSuspend()
    {
    }

    
    
    
    virtual void OnResume()
    {
    }

    
    
    virtual void OnResume( const GR::Strings::ParameterList& )
    {
      OnResume();
    }

    
    
    bool IsPaused()
    {
      if ( m_pStateManager == NULL )
      {
        return false;
      }
      return ( m_pStateManager->m_GameStateStack.back() != this );
    }



    bool IsActivelyUpdating()
    {
      if ( m_pStateManager == NULL )
      {
        return false;
      }
      if ( m_IsDecorativeState )
      {
        return true;
      }

      // the last state that's not a deco state
      auto it( m_pStateManager->m_GameStateStack.rbegin() );
      while ( it != m_pStateManager->m_GameStateStack.rend() )
      {
        IGameState<T>*    pState( *it );

        if ( pState == this )
        {
          return true;
        }

        if ( !pState->m_IsDecorativeState )
        {
          return false;
        }
        ++it;
      }
      return false;
    }



    bool IsGameStateFrozen()
    {
      if ( m_pStateManager == NULL )
      {
        return false;
      }
      auto it( m_pStateManager->m_GameStateStack.begin() );
      while ( it != m_pStateManager->m_GameStateStack.end() )
      {
        IGameState<T>*    pState = *it;
        
        if ( pState == this )
        {
          return false;
        }
        if ( pState->m_IsFreezingStack )
        {
          return true;
        }
        ++it;
      }
      return false;
    }



    virtual void OnPauseApplication()
    {
    }



    virtual void OnResumeApplication()
    {
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& )
    {
      return false;
    }



    void ChangeState( IGameState<T>* pNewState )
    {
      GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.ChangeState" );

      tGlobalEvent    popEvent( eventType );
      popEvent.m_Param1 = (GR::up)pNewState;
      GLOBAL_QUEUE.PostEvent( popEvent );
    }



    void ChangeState( const GR::String& NewState )
    {
      GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.ChangeState" );

      tGlobalEvent    popEvent( eventType );
      popEvent.m_Param = NewState;

      if ( !m_IsFlaggedForRemoval )
      {
        m_IsFlaggedForRemoval = true;

        popEvent.m_Params["PopState"] = Misc::Format() << ( GR::up )this;
      }
      GLOBAL_QUEUE.PostEvent( popEvent );
    }



    void ChangeState( const GR::String& NewState, const GR::Strings::ParameterList& Params )
    {
      GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.ChangeState" );

      tGlobalEvent    popEvent( eventType );
      popEvent.m_Param = NewState;
      popEvent.m_Params = Params;

      if ( !m_IsFlaggedForRemoval )
      {
        m_IsFlaggedForRemoval = true;

        popEvent.m_Params["PopState"] = Misc::Format() << ( GR::up )this;
      }
      GLOBAL_QUEUE.PostEvent( popEvent );
    }



    void PushState( const GR::String& NewState, const GR::Strings::ParameterList& Params = GR::Strings::ParameterList() )
    {
      GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.PushState" );

      tGlobalEvent    popEvent( eventType );
      popEvent.m_Param  = NewState;
      popEvent.m_Params = Params;
      GLOBAL_QUEUE.PostEvent( popEvent );
    }



    void PushState( IGameState<T>* pNewState )
    {
      GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.PushState" );

      tGlobalEvent    popEvent( eventType );
      popEvent.m_Param1 = ( GR::up )pNewState;
      GLOBAL_QUEUE.PostEvent( popEvent );
    }



    void PushStateOnStack( IGameState<T>* pNewState )
    {
      GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.PushStateOnStack" );

      tGlobalEvent    popEvent( eventType );
      popEvent.m_Param1 = (GR::up)pNewState;
      GLOBAL_QUEUE.PostEvent( popEvent );
    }



    void PopState()
    {
      if ( !m_IsFlaggedForRemoval )
      {
        m_IsFlaggedForRemoval = true;

        GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.PopState" );

        tGlobalEvent    popEvent( eventType );
        popEvent.m_Param1 = (GR::up)this;
        GLOBAL_QUEUE.PostEvent( popEvent );
      }
    }



    void PopState( const GR::Strings::ParameterList& Params )
    {
      if ( !m_IsFlaggedForRemoval )
      {
        m_IsFlaggedForRemoval = true;

        GR::u32 eventType = GLOBAL_QUEUE.RegisterEvent( "App.PopState" );

        tGlobalEvent    popEvent( eventType );
        popEvent.m_Param1 = (GR::up)this;
        popEvent.m_Params = Params;
        popEvent.m_Params["State"] = Misc::Format() << (GR::up)this;
        GLOBAL_QUEUE.PostEvent( popEvent );
      }
    }



    friend class IGameStateManager<T>;

};



template <typename T> class IGameStateManager : public EventListener<GUI::OutputEvent>
{

  public:

    enum class GameStateEventType
    {
      GSE_NONE = 0,
      GSE_PRE_PUSH,
      GSE_PUSHED,
      GSE_PRE_POP,
      GSE_POPPED
    };

    struct GameStateEvent
    {
      GameStateEventType      Type;
      IGameState<T>*          pState;

      GameStateEvent( GameStateEventType Type = GameStateEventType::GSE_NONE, IGameState<T>* pState = NULL ) :
        Type( Type ),
        pState( pState )
      {
      }
    };

    typedef fastdelegate::FastDelegate1<const GameStateEvent&>    tGameStateEventHandler;
    typedef std::list<tGameStateEventHandler>                     tGameStateEventHandlers;



    GR::u32                       m_ETStateInitialized,
                                  m_ETStateExited,
                                  m_ETStatePushedOnStack,
                                  m_ETStateRemovedFromStack,
                                  m_ETSendStateParam;

    std::list<IGameState<T>*>     m_GameStateStack;

    GR::u32                       m_ChangingState;

    tGameStateEventHandlers       m_GameStateEventHandlers;



    IGameStateManager()
    {
      m_ChangingState           = 0;
      m_ETStateInitialized      = GLOBAL_QUEUE.RegisterEvent( "App.StateInitialized" );
      m_ETStateExited           = GLOBAL_QUEUE.RegisterEvent( "App.StateExited" );
      m_ETStatePushedOnStack    = GLOBAL_QUEUE.RegisterEvent( "App.StatePushedOnStack" );
      m_ETStateRemovedFromStack = GLOBAL_QUEUE.RegisterEvent( "App.StateRemovedFromStack" );
      m_ETSendStateParam        = GLOBAL_QUEUE.RegisterEvent( "App.SendStateParam" );
    }



    virtual ~IGameStateManager()
    {
      while ( !m_GameStateStack.empty() )
      {
        PopAnyState();
      }
      GLOBAL_QUEUE.UnregisterEvent( m_ETStateInitialized );
      GLOBAL_QUEUE.UnregisterEvent( m_ETStateExited );
      GLOBAL_QUEUE.UnregisterEvent( m_ETStatePushedOnStack );
      GLOBAL_QUEUE.UnregisterEvent( m_ETStateRemovedFromStack );
      GLOBAL_QUEUE.UnregisterEvent( m_ETSendStateParam );
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      IGameState<T>*  pState = LastNotDecoState();
      if ( pState )      
      {
        pState->ProcessEvent( Event );
      }
      return true;
    }



    bool IsStateOnStack( IGameState<T>* pState )
    {
      auto it( m_GameStateStack.begin() );
      while ( it != m_GameStateStack.end() )
      {
        if ( *it == pState )
        {
          return true;
        }
        ++it;
      }
      return false;
    }



    void UpdateStatesPerDisplayFrame( const float fDeltaT )
    {
      int   iIndex = 0;
      bool  noUpdatesForStates = false;
      typename std::list<IGameState<T>*>::reverse_iterator    it( m_GameStateStack.rbegin() );
      while ( it != m_GameStateStack.rend() )
      {
        IGameState<T>*    pGameState = *it;

        if ( !noUpdatesForStates )
        {
          pGameState->UpdatePerDisplayFrame( fDeltaT );
          if ( pGameState->m_IsFreezingStack )
          {
            noUpdatesForStates = true;
          }
        }
        else if ( ( pGameState->m_IsDecorativeState )
        ||        ( pGameState->m_StateIsNotUpdatedWhenInactive ) )
        {
          pGameState->UpdatePerDisplayFrame( fDeltaT );
        }
        else
        {
          // nur der aktive State "bewegt" sich
          pGameState->UpdatePerDisplayFrame( 0 );
        }
        ++iIndex;
        ++it;
      }
    }



    void DisplayStates( T& Viewer )
    {
      typename std::list<IGameState<T>*>::iterator    it( m_GameStateStack.begin() );
      while ( it != m_GameStateStack.end() )
      {
        IGameState<T>*    pGameState = *it;

        pGameState->Display( Viewer );

        ++it;
      }
    }



    void UpdateCurrentState()
    {
      IGameState<T>*  pState = LastNotDecoState();
      if ( pState )
      {
        pState->Update();
      }
    }



    IGameState<T>* LastNotDecoState()
    {
      typename std::list<IGameState<T>*>::reverse_iterator    it( m_GameStateStack.rbegin() );
      while ( it != m_GameStateStack.rend() )
      {
        IGameState<T>*    pGameState = *it;

        if ( !pGameState->m_IsDecorativeState )
        {
          return pGameState;
        }
        ++it;
      }
      return NULL;
    }



    void PushState( IGameState<T>* pNewState )
    {
      if ( pNewState )
      {
        HandleEvent( GameStateEvent( GameStateEventType::GSE_PRE_PUSH, pNewState ) );

        IGameState<T>*  pState = LastNotDecoState();
        if ( pState )
        {
          pState->OnSuspend();
        }
        m_GameStateStack.push_back( pNewState );

        pNewState->m_pStateManager = this;
        pNewState->Init();
        HandleEvent( GameStateEvent( GameStateEventType::GSE_PUSHED, pNewState ) );
        GLOBAL_QUEUE.PostEvent( "App.StateInitialized" );
      }
    }



    void PopState( IGameState<T>* pState = NULL )
    {
      if ( pState == NULL )
      {
        pState = LastNotDecoState();
      }
      if ( pState == NULL )
      {
        if ( !m_GameStateStack.empty() )
        {
          pState = m_GameStateStack.back();
        }
      }
      if ( pState )
      {
        HandleEvent( GameStateEvent( GameStateEventType::GSE_PRE_POP, pState ) );

        m_GameStateStack.remove( pState );

        pState->Exit();
        GLOBAL_QUEUE.PostEvent( "App.StateExited", pState->m_ClassName.c_str() );
        HandleEvent( GameStateEvent( GameStateEventType::GSE_POPPED, pState ) );
        delete pState;
      }
      pState = LastNotDecoState();
      if ( pState )
      {
        if ( m_ChangingState == 0 )
        {
          // only resume previous state when we're not changing states
          pState->OnResume();
        }
      }
    }



    void PopAnyState( const GR::Strings::ParameterList& Params = GR::Strings::ParameterList() )
    {
      if ( m_GameStateStack.empty() )
      {
        return;
      }
      IGameState<T>*    pState = NULL;
      if ( Params.HasParam( "State" ) )
      {
        // want to pop specific state
        pState = (IGameState<T>*)Params.GetParamIP( "State" );
      }
      if ( pState == NULL )
      {
        pState = m_GameStateStack.back();
      }
      if ( pState )
      {
        HandleEvent( GameStateEvent( GameStateEventType::GSE_PRE_POP, pState ) );

        m_GameStateStack.remove( pState );

        pState->Exit();
        GLOBAL_QUEUE.PostEvent( "App.StateExited", pState->m_ClassName.c_str() );
        HandleEvent( GameStateEvent( GameStateEventType::GSE_POPPED, pState ) );
        delete pState;
      }
      pState = LastNotDecoState();
      if ( pState )
      {
        if ( m_ChangingState == 0 )
        {
          // only resume previous state when we're not changing states
          pState->OnResume();
        }
      }
    }



    void PopState( const GR::Strings::ParameterList& Params, IGameState<T>* pState = NULL )
    {
      if ( ( pState == NULL )
      &&   ( Params.HasParam( "State" ) ) )
      {
        // want to pop specific state
        pState = (IGameState<T>*)Params.GetParamIP( "State" );
      }
      if ( pState == NULL )
      {
        IGameState<T>*  pState = LastNotDecoState();
      }
      if ( pState == NULL )
      {
        if ( !m_GameStateStack.empty() )
        {
          pState = m_GameStateStack.back();
        }
      }
      if ( pState )
      {
        HandleEvent( GameStateEvent( GameStateEventType::GSE_PRE_POP, pState ) );

        m_GameStateStack.remove( pState );

        //dh::Log( "State Exit %s", pState->m_ClassName.c_str() );
        pState->Exit();
        GLOBAL_QUEUE.PostEvent( "App.StateExited", pState->m_ClassName.c_str() );
        HandleEvent( GameStateEvent( GameStateEventType::GSE_POPPED, pState ) );
        delete pState;
      }
      pState = LastNotDecoState();
      if ( pState )
      {
        if ( m_ChangingState == 0 )
        {
          // only resume previous state when we're not changing states
          pState->OnResume( Params );
        }
      }
    }



    void ChangeState( IGameState<T>* pNewState, IGameState<T>* pOldState = NULL )
    {
      ++m_ChangingState;
      PopState( pOldState );
      PushState( pNewState );
      --m_ChangingState;
    }



    void PushStateOnStack( IGameState<T>* pState )
    {
      pState->m_pStateManager = this;

      HandleEvent( GameStateEvent( GameStateEventType::GSE_PRE_PUSH, pState ) );
      m_GameStateStack.push_back( pState );
      HandleEvent( GameStateEvent( GameStateEventType::GSE_PUSHED, pState ) );

      GLOBAL_QUEUE.PostEvent( "App.StatePushedOnStack", pState->m_ClassName.c_str() );
    }



    void RemoveStateFromStack( IGameState<T>* pState )
    {
      m_GameStateStack.remove( pState );
      pState->m_pStateManager = NULL;
      GLOBAL_QUEUE.PostEvent( "App.StateRemovedFromStack", pState->m_ClassName.c_str() );
    }



    void PopAllStates()
    {
      while ( !m_GameStateStack.empty() )
      {
        PopState();
      }
    }



    void HandleEvent( const GameStateEvent& Event )
    {
      typename tGameStateEventHandlers::iterator    it( m_GameStateEventHandlers.begin() );
      while ( it != m_GameStateEventHandlers.end() )
      {
        tGameStateEventHandler&   eventHandler( *it );

        eventHandler( Event );

        ++it;
      }
    }



    void AddHandler( tGameStateEventHandler Handler )
    {
      m_GameStateEventHandlers.push_back( Handler );
    }



    void RemoveHandler( tGameStateEventHandler Handler )
    {
      m_GameStateEventHandlers.remove( Handler );
    }


    void OnPauseApplication()
    {
      auto it( m_GameStateStack.rbegin() );
      while ( it != m_GameStateStack.rend() )
      {
        IGameState<T>*    pGameState = *it;

        pGameState->OnPauseApplication();
        ++it;
      }
    }



    void OnResumeApplication()
    {
      auto it( m_GameStateStack.rbegin() );
      while ( it != m_GameStateStack.rend() )
      {
        IGameState<T>*    pGameState = *it;

        pGameState->OnResumeApplication();
        ++it;
      }
    }

};



