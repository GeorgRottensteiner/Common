#ifndef GUI_GAME_STATE_H
#define GUI_GAME_STATE_H


#include "IGameState.h"

#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>
#include <Interface/IGUILoader.h>

#include <Lang/FastDelegate.h>
#include <Lang/Service.h>

#include <Controls/ComponentDisplayerBase.h>
#include <Controls/Component.h>



template <typename T,typename GUI_BASE_CLASS> class GUIGameState : public IGameState<T>
{
  public:

    GUI::Component*                   m_pStateScreen;


    using IGameState<T>::Display;



    GUIGameState() :
      m_pStateScreen( NULL )
    {
    }



    void OnDraw( const GUI::OutputEvent& Event )
    {
      GUI::OwnerDrawInfo*    pODInfo = (GUI::OwnerDrawInfo*)Event.Param1;

      Display( *(T*)pODInfo->pDisplayer->m_pRenderer );
    }

};




template <typename T, typename GUI_BASE_CLASS> class GUIGameStateManager : public IGameStateManager<T>
{
  public:

    typedef GUIGameState<T, GUI_BASE_CLASS>       GameStateType;

    std::map<GUIGameState<T, GUI_BASE_CLASS>*,GUI::Component*>    CreatedScreens;


    using IGameStateManager<T>::AddHandler;
    using IGameStateManager<T>::RemoveHandler;



    GUIGameStateManager()
    {
      AddHandler( fastdelegate::MakeDelegate( this, &GUIGameStateManager<T, GUI_BASE_CLASS>::HandleGameStateEvent ) );
    }

    virtual ~GUIGameStateManager()
    {
      RemoveHandler( fastdelegate::MakeDelegate( this, &GUIGameStateManager<T, GUI_BASE_CLASS>::HandleGameStateEvent ) );
    }



    void OnChildComponentAdded( const GUI::OutputEvent& Event )
    {
      GUI::IGUILoader*          pGUILoader = (GUI::IGUILoader*)GR::Service::Environment::Instance().Service( "GUILoader" );

      if ( pGUILoader )
      {
        pGUILoader->UpdateGUITextFromDB( Event.pComponent );
      }
      //dh::Log( "State added a child GUI element: %s", Event.pComponent->Class().c_str() );
    }



    void HandleGameStateEvent( const typename IGameStateManager<T>::GameStateEvent& Event )
    {
      GUIGameState<T, GUI_BASE_CLASS>*    pState = (GUIGameState<T, GUI_BASE_CLASS>*)Event.pState;
      GUI::ComponentDisplayerBase*        pGUI = (GUI::ComponentDisplayerBase*)GR::Service::Environment::Instance().Service( "GUI" );
      GR::RenderFrame*                    pRenderFrame = (GR::RenderFrame*)GR::Service::Environment::Instance().Service( "WindowFrame" );
      
      switch ( Event.Type )
      {
        case IGameStateManager<T>::GameStateEventType::GSE_PRE_PUSH:
          if ( pGUI )
          {
            if ( pState->m_pStateScreen == NULL )
            {
              pState->m_pStateScreen = (GUI::Component*)pGUI->CreateObject( "Screen" );
              if ( pState->m_pStateScreen == NULL )
              {
                dh::Log( "Screen is not a registered GUI class (add GUIScreen to project!)" );
              }
              else
              {
                pState->m_pStateScreen->AddHandler( OET_ADDED_CHILD, fastdelegate::MakeDelegate( this, &GUIGameStateManager::OnChildComponentAdded ) );
                CreatedScreens[pState] = pState->m_pStateScreen;
              }
              if ( pRenderFrame )
              {
                GR::tPoint      targetSize( pRenderFrame->Width(), pRenderFrame->Height() );
                targetSize = pGUI->ScreenToVirtual( targetSize );
                pState->m_pStateScreen->SetSize( targetSize.x, targetSize.y );

                // add directly
                pGUI->Add( pState->m_pStateScreen );
              }
            }
            else
            {
              if ( pRenderFrame )
              {
                GR::tPoint      targetSize( pRenderFrame->Width(), pRenderFrame->Height() );
                targetSize = pGUI->ScreenToVirtual( targetSize );
                pState->m_pStateScreen->SetSize( targetSize.x, targetSize.y );

                // add directly
                pGUI->Add( pState->m_pStateScreen );
              }
            }
          }
          break;
        case IGameStateManager<T>::GameStateEventType::GSE_PUSHED:
          if ( pGUI )
          {
            bool    wasReplaced = false;

            typename std::map<GUIGameState<T, GUI_BASE_CLASS>*,GUI::Component*>::iterator    itCS( CreatedScreens.find( pState ) );
            if ( itCS != CreatedScreens.end() )
            {
              if ( itCS->second != pState->m_pStateScreen )
              {
                wasReplaced = true;

                // remove previously added screen again
                pGUI->Remove( itCS->second );

                // the screen was replaced
                delete itCS->second;
              }
              else
              {
                pState->m_pStateScreen->RemoveHandler( OET_ADDED_CHILD, fastdelegate::MakeDelegate( this, &GUIGameStateManager::OnChildComponentAdded ) );
              }
              CreatedScreens.erase( itCS );
            }

            if ( pState->m_pStateScreen == NULL )
            {
              pState->m_pStateScreen = (GUI::Component*)pGUI->CreateObject( "Screen" );
              if ( pState->m_pStateScreen == NULL )
              {
                dh::Log( "Screen is not a registered GUI class (add GUIScreen to project!)" );
              }
              if ( pRenderFrame )
              {
                GR::tPoint      targetSize( pRenderFrame->Width(), pRenderFrame->Height() );
                targetSize = pGUI->ScreenToVirtual( targetSize );
                pState->m_pStateScreen->SetSize( targetSize.x, targetSize.y );
              }
            }
            pState->m_pStateScreen->ModifyFlags( GUI::COMPFT_OWNER_DRAW );
            pState->m_pStateScreen->AddHandler( OET_OWNER_DRAW, fastdelegate::MakeDelegate( pState, &GUIGameState<T, GUI_BASE_CLASS>::OnDraw ) );
            if ( wasReplaced )
            {
              pGUI->Add( pState->m_pStateScreen );
            }
            // use focus from set component
            if ( pState->m_pStateScreen->GetFocusedComponent() )
            {
              pGUI->SetFocus( pState->m_pStateScreen->GetFocusedComponent() );
            }
          }
          break;
        case IGameStateManager<T>::GameStateEventType::GSE_POPPED:
          if ( pGUI )
          {
            pGUI->Delete( pState->m_pStateScreen );
            pState->m_pStateScreen = NULL;
          }
          break;
      }
    }


};


#endif// GUI_GAME_STATE_H

