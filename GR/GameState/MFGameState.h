#ifndef _IMF_GAME_STATE_H
#define _IMF_GAME_STATE_H


#include "IGameState.h"

#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>

#include <MasterFrame/MasterFrame.h>



namespace GUI
{
  class OutputEvent;
}
class MasterFrameEvent;



template <typename T> class MFGameState : public IGameState<T>, public EventListener<MasterFrameEvent>
{

  public:


    virtual bool              ProcessEvent( const GUI::OutputEvent& )
    {
      return false;
    }

    virtual ~MFGameState()
    {
    }

    virtual bool              ProcessEvent( const MasterFrameEvent& )
    {
      return false;
    }

};



template <typename T> class MFGameStateManager : public IGameStateManager<T>
{
  public:

    IEventProducer<MasterFrameEvent>*    m_pEventProducer;


    MFGameStateManager() :
      m_pEventProducer( NULL )
    {
    }

    virtual ~MFGameStateManager()
    {
    }

    void                      SetEventProducer( IEventProducer<MasterFrameEvent>* pEventProducer )
    {
      m_pEventProducer = pEventProducer;
    }

    bool                      ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( !m_GameStateStack.empty() )
      {
        MFGameState<T>*   pState = (MFGameState<T>*)m_GameStateStack.back();
        pState->ProcessEvent( Event );
      }
      return true;
    }

    virtual bool              ProcessEvent( const MasterFrameEvent& Event )
    {
      if ( Event.m_mfEvent == MF_EVENT_POP_STATE )
      {
        DoPopState();
        if ( Event.m_dwParam1 )
        {
          PushState( (MFGameState<T>*)Event.m_dwParam1 );
        }
      }
      return false;
    }

    virtual void              PushState( IGameState<T>* pNewState )
    {
      m_GameStateStack.push_back( pNewState );
      if ( m_pEventProducer )
      {
        m_pEventProducer->AddListener( (MFGameState<T>*)pNewState );
      }
      pNewState->m_pStateManager = this;
      pNewState->Init();
    }

    virtual void              ChangeState( IGameState<T>* pNewState )
    {
      if ( m_pEventProducer )
      {
        MasterFrameEvent   Event;

        Event.m_mfEvent = MF_EVENT_POP_STATE;
        Event.m_dwParam1 = (DWORD_PTR)pNewState;
        m_pEventProducer->PostEvent( Event );
      }
      else
      {
        PopState();
        PushState( pNewState );
      }
    }

    virtual void              PopState()
    {
      if ( m_pEventProducer )
      {
        MasterFrameEvent   Event;

        Event.m_mfEvent = MF_EVENT_POP_STATE;
        m_pEventProducer->PostEvent( Event );
      }
      else
      {
        DoPopState();
      }
    }

  protected:

    void DoPopState()
    {
      if ( !m_GameStateStack.empty() )
      {
        MFGameState<T>* pOldState = (MFGameState<T>*)m_GameStateStack.back();

        m_GameStateStack.pop_back();

        pOldState->Exit();
        if ( m_pEventProducer )
        {
          m_pEventProducer->RemoveListener( pOldState );
        }

        delete pOldState;
      }
      if ( !m_GameStateStack.empty() )
      {
        if ( m_ChangingState == 0 )
        {
          // only resume previous state when we're not changing states
          MFGameState<T>* pCurrentState = (MFGameState<T>*)m_GameStateStack.back();
          pCurrentState->OnResume();
        }
      }
    }

};


#endif// _IMF_GAME_STATE_H

