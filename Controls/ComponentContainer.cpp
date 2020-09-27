#include <debug\debugclient.h>

#include <Misc/Misc.h>

#include <Xtreme/XInput.h>

#include <Controls/Component.h>
#include <Controls/ComponentContainer.h>



namespace GUI
{

  ComponentContainer::ComponentContainer() :
    m_LastMouseButtons( 0 ),
    m_pActiveComponent( NULL ),
    m_pMouseOverComponent( NULL ),
    m_pDraggingComponent( NULL ),
    m_pDraggingComponentContent( NULL ),
    m_pParentContainer( NULL ),
    m_pCapturingComponent( NULL ),
    m_pModalComponent( NULL ),
    m_ContainerProcessingDisabled( false ),
    m_pDraggingComponentSource( NULL ),
    m_pFocusedComponent( NULL ),
    m_ShuttingDown( false )
  {

  }



  ComponentContainer::~ComponentContainer()
  {
    m_ShuttingDown = true;
    DeleteAllControls();
    DeleteDanglingControls();
  }



  void ComponentContainer::DeleteDanglingControls( bool Recursive )
  {
    if ( Recursive )
    {
      tListComponents::iterator   it( m_Components.begin() );
      while ( it != m_Components.end() )
      {
        Component*  pComponent = *it;

        pComponent->DeleteDanglingControls();

        it++;
      }
    }

    size_t  curIndex = 0;

    while ( curIndex < m_ComponentsToDelete.size() )
    {
      Component*  pComponent = m_ComponentsToDelete[curIndex];
      ++curIndex;
      
      // flag so no more events are sent
      pComponent->m_ShuttingDown = true;
      tListComponents::iterator   it2( m_Components.begin() );
      while ( it2 != m_Components.end() )
      {
        Component*  pComponent2 = *it2;

        if ( pComponent2 == pComponent )
        {
          m_Components.erase( it2 );
          break;
        }

        ++it2;
      }
      //dh::Log( "Actual delete of %s", pComponent->GetCaption().c_str() );
      delete pComponent;
    }
    m_ComponentsToDelete.clear();
  }



  void ComponentContainer::ToggleAllControlsVisibility()
  {

    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component*  pComponent = *it;
      
      pComponent->SetVisible( !pComponent->IsVisible() );

      it++;
    }

  }



  void ComponentContainer::DeleteAllControls( bool bDeleteOnlyHidden )
  {
    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component*  pComponent = *it;
      
      if ( ( bDeleteOnlyHidden )
      &&   ( pComponent->IsVisible() ) )
      {
        ++it;
        continue;
      }
      Delete( pComponent );

      ++it;
    }
  }



  void ComponentContainer::RemoveAllControls()
  {

    ComponentContainer*    pTopLevel = GetTopLevelParent();

    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component*  pComponent = *it;

      if ( ( pTopLevel )
      &&   ( pTopLevel->GetFocusedComponent() == pComponent ) )
      {
        pTopLevel->SetFocus( NULL );
      }
      if ( pTopLevel->m_pActiveComponent == pComponent )
      {
        pTopLevel->m_pActiveComponent = NULL;
      }
      if ( m_pModalComponent == pComponent )
      {
        m_pModalComponent = NULL;
      }
      if ( m_pCapturingComponent == pComponent )
      {
        ReleaseCapture();
      }
      it++;
    }
    m_Components.clear();

  }



  void ComponentContainer::Add( Component *pComponent )
  {
    if ( pComponent == NULL )
    {
      return;
    }
    if ( pComponent->m_pParentContainer )
    {
      dh::Log( "Component already has parent!" );
      return;
    }
    if ( pComponent == this )
    {
      dh::Log( "Tried to add component to itself!" );
      return;
    }
    pComponent->m_pParentContainer = this;
    m_Components.push_back( pComponent );

    pComponent->ProcessEvent( ComponentEvent( CET_ADDED ) );
    ProcessEvent( ComponentEvent( CET_ADDED_CHILD, pComponent ) );
  }



  void ComponentContainer::Remove( Component* pComponent )
  {
    ComponentContainer*    pTopLevel = GetTopLevelParent();

    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      if ( *it == pComponent )
      {
        // die Component ist eingetragen, also raus damit!
        if ( ( pTopLevel )
        &&   ( pTopLevel->GetFocusedComponent() == pComponent ) )
        {
          pTopLevel->SetFocus( NULL );
        }
        if ( pTopLevel->m_pActiveComponent == pComponent )
        {
          pTopLevel->m_pActiveComponent = NULL;
        }
        if ( m_pMouseOverComponent == pComponent )
        {
          m_pMouseOverComponent = NULL;
        }
        if ( m_pModalComponent == pComponent )
        {
          m_pModalComponent = NULL;
        }
        if ( m_pCapturingComponent == pComponent )
        {
          ReleaseCapture();
          m_pCapturingComponent = NULL;
        }
        m_Components.erase( it );

        pComponent->m_pParent           = NULL;
        pComponent->m_pParentContainer  = NULL;

        pComponent->ProcessEvent( ComponentEvent( CET_REMOVED ) );
        ProcessEvent( ComponentEvent( CET_REMOVED_CHILD, pComponent ) );
        return;
      }
      it++;
    }
  }



  void ComponentContainer::Delete( Component* pComponent )
  {
    if ( ( pComponent == NULL )
    ||   ( pComponent->IsDestroyed() ) )
    {
      return;
    }
    ComponentContainer*    pTopLevel = GetTopLevelParent();

    tListComponents::iterator   it( m_Components.begin() );

    while ( it != m_Components.end() )
    {
      if ( *it == pComponent )
      {
        if ( !m_ShuttingDown )
        {
          pComponent->RecursiveChildEvent( OET_DESTROY );
          pComponent->m_ShuttingDown = true;
        }
        if ( ( pTopLevel )
        &&   ( pTopLevel->GetModalComponent() == pComponent ) )
        {
          pTopLevel->SetModalMode();
        }

        if ( ( pTopLevel )
        &&   ( pTopLevel->GetFocusedComponent() == pComponent ) )
        {
          pTopLevel->SetFocus( NULL );
        }
        if ( pTopLevel->m_pActiveComponent == pComponent )
        {
          pTopLevel->m_pActiveComponent = NULL;
        }
        if ( m_pModalComponent == pComponent )
        {
          m_pModalComponent = NULL;
        }
        if ( m_pMouseOverComponent == pComponent )
        {
          m_pMouseOverComponent = NULL;
        }
        if ( pTopLevel->m_pDraggingComponent == pComponent )
        {
          pTopLevel->m_pDraggingComponent->StopDragging();
          pTopLevel->m_pDraggingComponent = NULL;
        }
        if ( pTopLevel->m_pDraggingComponentContent == pComponent )
        {
          pTopLevel->m_pDraggingComponentContent->StopDragging();
          pTopLevel->m_pDraggingComponentContent = NULL;
        }
        if ( pTopLevel->m_pCapturingComponent == pComponent )
        {
          ReleaseCapture();
          pTopLevel->m_pCapturingComponent = NULL;
        }

        pComponent->RemoveAllListeners();

        m_ComponentsToDelete.push_back( *it );

        // childs are deleted indirectly
        pComponent->m_ComponentFlags |= GUI::COMPFT_DESTROYED;

        //dh::Log( "pre-Deleted component %s (%d) (%s)", pComponent->GetCaption().c_str(), pComponent->Id(), pComponent->m_ClassName.c_str() );
        return;
      }
      it++;
    }
  }



  void ComponentContainer::ScreenToLocal( GR::tPoint& pt, Component* pComponent )
  {
    while ( pComponent )
    {
      pt -= pComponent->Position() + pComponent->m_ClientRect.position();

      pComponent = pComponent->GetComponentParent();
    }
  }



  void ComponentContainer::ScreenToNonClient( GR::tPoint& pt, Component* pComponent )
  {
    if ( pComponent )
    {
      pt -= pComponent->Position();
      pComponent = pComponent->GetComponentParent();
    }

    while ( pComponent )
    {
      pt -= pComponent->Position() + pComponent->m_ClientRect.position();

      pComponent = pComponent->GetComponentParent();
    }
  }



  void ComponentContainer::LocalToScreen( GR::tPoint& pt, Component* pComponent )
  {
    while ( pComponent )
    {
      GR::tRect   rcClient;

      pComponent->GetClientRect( rcClient );

      pt += pComponent->Position() + rcClient.position();

      pComponent = pComponent->GetComponentParent();
    }
  }



  void ComponentContainer::LocalToScreen( GR::tRect& rc, Component* pComponent )
  {
    while ( pComponent )
    {
      GR::tRect   rcClient;

      pComponent->GetClientRect( rcClient );

      rc.offset( pComponent->Position() + rcClient.position() );

      pComponent = pComponent->GetComponentParent();
    }
  }



  void ComponentContainer::PostComponentEvent( const GUI::ComponentEvent& Event )
  {
    m_ComponentEventQueue.push_back( Event );
  }



  void ComponentContainer::ProcessComponentEventQueue()
  {
    std::list<GUI::ComponentEvent>::iterator    itCE( m_ComponentEventQueue.begin() );
    while ( itCE != m_ComponentEventQueue.end() )
    {
      ProcessEvent( *itCE );

      ++itCE;
    }
    m_ComponentEventQueue.clear();
  }



  bool ComponentContainer::ProcessEvent( const ComponentEvent& Event )
  {
    if ( m_ContainerProcessingDisabled )
    {
      return true;
    }

    if ( m_pParentContainer == NULL )
    {
      if ( m_pDraggingComponentContent != NULL )
      {
        Component* pCompUnderMouse = NULL;
        if ( Event.Type == CET_MOUSE_UPDATE )
        {
          pCompUnderMouse = GetComponentAt( Event.Position, true, true );
        }
        // TODO - query while dragging?
        if ( ( Event.Type == CET_MOUSE_UPDATE )
        &&   ( Event.MouseButtons == 0 ) )
        {
          // schluß mit draggen
          GUI::DragContentInfo    dragInfo;

          dragInfo.pComponentDragContent  = m_pDraggingComponentContent;
          dragInfo.pComponentDragSource   = m_pDraggingComponentSource;
          dragInfo.DragUserData           = m_pDraggingComponentContent->GetUserData();
          if ( ( pCompUnderMouse )
          &&   ( pCompUnderMouse->Flags() & GUI::COMPFT_DRAG_CONTENT_TARGET ) )
          {
            pCompUnderMouse->GenerateEvent( OET_DRAG_CONTENT_DROP, (GR::up)&dragInfo );
          }
          else
          {
            m_pDraggingComponentSource->GenerateEvent( OET_DRAG_CONTENT_CANCEL, ( GR::up )&dragInfo );
          }
          Delete( m_pDraggingComponentContent );
          m_pDraggingComponentContent = NULL;
          m_pDraggingComponentSource = NULL;
          return true;
        }

        // Hier passiert das eigentliche Draggen
        if ( Event.Type == CET_MOUSE_UPDATE )
        {
          m_pDraggingComponentContent->SetLocation( Event.Position.x, Event.Position.y );
          m_pDraggingComponentContent->m_DragPos = Event.Position;
        }
        return true;
      }
      if ( m_pDraggingComponent != NULL )
      {
        if ( ( Event.Type == CET_MOUSE_UPDATE )
        &&   ( Event.MouseButtons == 0 ) )
        {
          // schluß mit draggen
          m_pDraggingComponent->StopDragging();
          m_pDraggingComponent = NULL;
          return true;
        }

        // Hier passiert das eigentliche Draggen
        if ( Event.Type == CET_MOUSE_UPDATE )
        {
          ComponentEvent     newEvent( Event );

          GR::tPoint    ptSoll = GR::tPoint( m_pDraggingComponent->Position().x + newEvent.Position.x - m_pDraggingComponent->DragPosition().x,
                                             m_pDraggingComponent->Position().y + newEvent.Position.y - m_pDraggingComponent->DragPosition().y );

          m_pDraggingComponent->SetLocation( ptSoll.x, ptSoll.y );
          m_pDraggingComponent->m_DragPos = newEvent.Position + ( m_pDraggingComponent->Position() - ptSoll );
        }
        return true;
      }
      if ( ( m_pCapturingComponent )
      &&   ( m_pCapturingComponent != this ) )
      {
        if ( ( Event.Type == CET_MOUSE_UPDATE )
        ||   ( Event.Type == CET_MOUSE_UP )
        ||   ( Event.Type == CET_MOUSE_DOWN )
        ||   ( Event.Type == CET_MOUSE_WHEEL )
        ||   ( Event.Type == CET_MOUSE_RUP )
        ||   ( Event.Type == CET_MOUSE_RDOWN ) )
        { 
          ComponentEvent     newEvent( Event );

          ScreenToNonClient( newEvent.Position, m_pCapturingComponent );
          
          if ( m_pCapturingComponent->IsMouseInsideNonClientArea( newEvent.Position ) )
          {
            if ( !m_pCapturingComponent->IsMouseInside() )
            {
              m_pCapturingComponent->m_ComponentFlags |= GUI::COMPFT_MOUSE_INSIDE;
              ComponentEvent     mouseinEvent( CET_MOUSE_IN );
              mouseinEvent.MouseButtons   = newEvent.MouseButtons;
              mouseinEvent.Position       = newEvent.Position - m_pCapturingComponent->m_ClientRect.position();
              m_pCapturingComponent->ProcessEvent( mouseinEvent );
            }
          }
          else
          {
            if ( m_pCapturingComponent->IsMouseInside() )
            {
              m_pCapturingComponent->m_ComponentFlags &= ~GUI::COMPFT_MOUSE_INSIDE;
              ComponentEvent    newEvent( CET_MOUSE_OUT );
              m_pCapturingComponent->ProcessEvent( newEvent );
            }
          }
          if ( m_pCapturingComponent->ProcessEvent( newEvent ) )
          {
            return true;
          }
          return true;
        }
      }
    }

    bool                bFoundInsideComponent = false;

    switch ( Event.Type )
    {
      case CET_MOUSE_UP:
      case CET_MOUSE_DOWN:
      case CET_MOUSE_RUP:
      case CET_MOUSE_RDOWN:
      case CET_MOUSE_UPDATE:
      case CET_MOUSE_WHEEL:
        {
          Component*   pComponentUnderMouse = GetComponentAt( Event.Position, false, true );

          if ( ModalDisabled( pComponentUnderMouse ) )
          {
            break;
          }

          if ( m_pMouseOverComponent != pComponentUnderMouse )
          {
            if ( ( m_pMouseOverComponent )
            &&   ( m_pMouseOverComponent->IsMouseInside() ) )
            {
              m_pMouseOverComponent->m_ComponentFlags &= ~GUI::COMPFT_MOUSE_INSIDE;
              ComponentEvent     newEvent( CET_MOUSE_OUT );
              m_pMouseOverComponent->ProcessEvent( newEvent );
            }
            m_pMouseOverComponent = pComponentUnderMouse;
          }

          if ( pComponentUnderMouse )
          {
            // Event weitergeben
            ComponentEvent     newEvent( Event );

            newEvent.Position -= pComponentUnderMouse->Position();

            GR::tPoint      ptPosInClientCoord = Event.Position;

            ptPosInClientCoord -= pComponentUnderMouse->Position() + pComponentUnderMouse->GetClientOffset();

            if ( !pComponentUnderMouse->IsMouseInside( ptPosInClientCoord ) )
            {
              // ein NC-Event
              GR::u32     NCEvent = CET_NULL;

              // Events auf NC mappen
              switch ( Event.Type )
              {
                case CET_MOUSE_UPDATE:
                  NCEvent = CET_NC_MOUSE_UPDATE;
                  break;
                case CET_MOUSE_DOWN:
                  NCEvent = CET_NC_MOUSE_DOWN;
                  break;
                case CET_MOUSE_UP:
                  NCEvent = CET_NC_MOUSE_UP;
                  break;
                case CET_MOUSE_RDOWN:
                  NCEvent = CET_NC_MOUSE_RDOWN;
                  break;
                case CET_MOUSE_RUP:
                  NCEvent = CET_NC_MOUSE_RUP;
                  break;
              }
              if ( NCEvent != CET_NULL )
              {
                newEvent.Type = NCEvent;
                pComponentUnderMouse->ProcessEvent( newEvent );

                // ein NC-Event ist immer das Ende der Bearbeitungskette!
                return true;
              }
            }
            if ( pComponentUnderMouse->IsMouseInside( ptPosInClientCoord ) )
            {
              newEvent.Position = ptPosInClientCoord;
              return pComponentUnderMouse->ProcessEvent( newEvent );
            }
            break;
          }
        }
        break;
      case CET_MOUSE_IN:
        break;
      case CET_MOUSE_OUT:
        {
          if ( m_pMouseOverComponent )
          {
            m_pMouseOverComponent->m_ComponentFlags &= ~GUI::COMPFT_MOUSE_INSIDE;
            ComponentEvent     newEvent( CET_MOUSE_OUT );
            m_pMouseOverComponent->ProcessEvent( newEvent );
            m_pMouseOverComponent = NULL;
          }
        }
        break;
      case CET_KEY_DOWN:
        break;
    }

    if ( bFoundInsideComponent )
    {
       return true;
    }
    return false;
  }



  ComponentContainer* ComponentContainer::GetTopLevelParent()
  {

    ComponentContainer*   pContainer = this;

    while ( pContainer )
    {
      if ( pContainer->m_pParentContainer == NULL )
      {
        return pContainer;
      }

      pContainer = pContainer->m_pParentContainer;
    }
    return NULL;

  }



  void ComponentContainer::GenerateEvent( Component* pParentComponent, GR::u32 Id, OutputEventType oeType, 
                                           GR::up Param1, GR::up Param2, GR::up Param3, GR::up Param4 )
  {
    // gibt es einen Handler?
    Component::tMapEventHandler::iterator    itHandler( pParentComponent->m_EventHandler.find( oeType ) );
    if ( itHandler != pParentComponent->m_EventHandler.end() )
    {
      // make a copy of the list in case it's modified during handling
      Component::tEventHandlers    listHandler = itHandler->second;

      Component::tEventHandlers::iterator   itList( listHandler.begin() );
      while ( itList != listHandler.end() )
      {
        Component::tEventHandlerFunction   funcHandler = *itList;

        funcHandler( GUI::OutputEvent( oeType, Id, pParentComponent, Param1, Param2, Param3, Param4 ) );

        ++itList;
      }
    }
    pParentComponent->SendEvent( GUI::OutputEvent( oeType, Id, pParentComponent, Param1, Param2, Param3, Param4 ) );
  }



  void ComponentContainer::GenerateEventForParent( Component* pParentComponent, GR::u32 Id, OutputEventType oeType, 
                                                    GR::up Param1, GR::up Param2, GR::up Param3, GR::up Param4 )
  {
    if ( pParentComponent == NULL )
    {
      return;
    }
    if ( pParentComponent->IsDestroyed() )
    {
      return;
    }
    ComponentContainer*   pContainer = GetTopLevelParent();

    if ( pContainer )
    {
      pContainer->SendEvent( GUI::OutputEvent( oeType, Id, pParentComponent, Param1, Param2, Param3, Param4 ) );
        //GenerateEvent( pParentComponent, Id, oeType, Param1, Param2, Param3, Param4 );
    }
    GenerateEvent( pParentComponent, Id, oeType, Param1, Param2, Param3, Param4 );
  }



  void ComponentContainer::SetCaptureToComponent( Component* pComponent )
  {
    ComponentContainer* pContainer = GetTopLevelParent();

    if ( pContainer == NULL )
    {
      pContainer = this;
    }
    if ( pContainer )
    {
      if ( pContainer->m_pCapturingComponent != pComponent )
      {
        if ( pContainer->m_pCapturingComponent )
        {
          pContainer->m_pCapturingComponent->GenerateEvent( OET_CAPTURE_END, (GR::up)pComponent );
        }
      }
      pContainer->m_pCapturingComponent = pComponent;
      pComponent->GenerateEvent( OET_CAPTURE_BEGIN );
    }
  }



  void ComponentContainer::ReleaseCapture()
  {
    ComponentContainer* pContainer = GetTopLevelParent();

    if ( pContainer == NULL )
    {
      pContainer = this;
    }
    if ( pContainer )
    {
      if ( pContainer->m_pCapturingComponent )
      {
        pContainer->m_pCapturingComponent->GenerateEvent( OET_CAPTURE_END );
      }
      pContainer->m_pCapturingComponent = NULL;
    }
  }



  void ComponentContainer::GetClientRect( GR::tRect& rectClient ) const
  {
    rectClient = m_ClientRect;
    rectClient.offset( -rectClient.Left, -rectClient.Top );
  }



  GR::tRect ComponentContainer::GetClientRect() const
  {
    GR::tRect rectClient = m_ClientRect;
    rectClient.offset( -rectClient.Left, -rectClient.Top );

    return rectClient;
  }



  GR::tPoint ComponentContainer::GetClientOffset() const
  {
    return m_ClientRect.position();
  }



  void ComponentContainer::EnableProcessing( bool Enable )
  {
    m_ContainerProcessingDisabled = !Enable;
  }



  Component* ComponentContainer::GetModalComponent()
  {
    return m_pModalComponent;
  }



  void ComponentContainer::SetModalMode( Component* pComponent )
  {
    if ( pComponent == NULL )
    {
      // Modalen Modus aufheben
      if ( m_pModalComponent == NULL )
      {
        return;
      }
      m_pModalComponent = NULL;
    }
    m_pModalComponent = pComponent;
  }



  Component* ComponentContainer::GetComponent( GR::u32 Id )
  {
    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component*  pComp = *it;

      if ( pComp->IsDestroyed() )
      {
        ++it;
        continue;
      }
      if ( pComp->Id() == Id )
      {
        return pComp;
      }

      pComp = pComp->GetComponent( Id );
      if ( pComp )
      {
        return pComp;
      }

      ++it;
    }
    return NULL;
  }



  bool ComponentContainer::ProcessEvent( const GUI::OutputEvent& Event )
  {
    return false;
  }



  void ComponentContainer::Dump( int Level )
  {
    GR::string     indent;

    for ( int i = 0; i < Level; ++i )
    {
      indent += "  ";
    }

    GR::string     line;

    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component*   pComponent = *it;

      line = CMisc::printf( "%s%s - (%s)", indent.c_str(), pComponent->Class().c_str(), pComponent->GetCaption().c_str() );

      if ( pComponent->IsFocused() )
      {
        line += " Focus";
      }
      if ( !pComponent->IsEnabled() )
      {
        line += " Disabled";
      }
      dh::Log( line.c_str() );
      pComponent->Dump( Level + 1 );

      ++it;
    }
  }



  Component* ComponentContainer::GetComponentAt( const GR::tPoint& Position, bool Recursive, bool IgnoreMouseIgnorer )
  {
    tListComponents::reverse_iterator   rit( m_Components.rbegin() );
    while ( rit != m_Components.rend() )
    {
      Component *pComponent = *rit;

      if ( !pComponent->IsVisible() )
      {
        ++rit;
        continue;
      }
      if ( ( IgnoreMouseIgnorer )
      &&   ( pComponent->Flags() & GUI::COMPFT_DONT_CATCH_MOUSE ) )
      {
        ++rit;
        continue;
      }

      GR::tPoint    compNCPos = Position - pComponent->Position();

      if ( pComponent->IsMouseInsideNonClientArea( compNCPos ) )
      {
        if ( Recursive )
        {
          GR::tPoint  ptClientOffset = pComponent->GetClientOffset();

          Component*   pChildComp = pComponent->GetComponentAt( compNCPos - ptClientOffset, Recursive, IgnoreMouseIgnorer );
          if ( pChildComp )
          {
            return pChildComp;
          }
        }
        return pComponent;
      }

      ++rit;
    }
    return NULL;
  }



  bool ComponentContainer::ModalDisabled( Component* pComponent )
  {
    if ( pComponent == NULL )
    {
      return false;
    }
    ComponentContainer*    pTopLevel = GetTopLevelParent();
    if ( pTopLevel )
    {
      if ( ( pTopLevel->m_pModalComponent )
      &&   ( pTopLevel->m_pModalComponent != pComponent )
      &&   ( !pTopLevel->m_pModalComponent->IsIndirectChild( pComponent ) )
      &&   ( !pComponent->IsIndirectChild( pTopLevel->m_pModalComponent ) ) )
      {
        return true;
      }
    }
    return false;
  }



  void ComponentContainer::ReplaceFont( Interface::IFont* pOldFont, Interface::IFont* pFont )
  {
    tListComponents::reverse_iterator   rit( m_Components.rbegin() );
    while ( rit != m_Components.rend() )
    {
      Component* pComponent = *rit;

      if ( pComponent->m_pFont == pOldFont )
      {
        pComponent->m_pFont = pFont;
      }

      pComponent->ReplaceFont( pOldFont, pFont );

      ++rit;
    }
  }



  Component* ComponentContainer::GetFocusedComponent()
  {
    return m_pFocusedComponent;
  }



  void ComponentContainer::SetFocus( Component* pComponent )
  {
    if ( m_pFocusedComponent == pComponent )
    {
      return;
    }

    Component*   pOldFocused = m_pFocusedComponent;
    if ( m_pFocusedComponent )
    {
      m_pFocusedComponent->ModifyFlags( 0, GUI::COMPFT_FOCUSED );

      if ( ( !m_ShuttingDown )
      &&   ( !m_pFocusedComponent->m_ShuttingDown ) )
      {
        m_pFocusedComponent->ProcessEvent( ComponentEvent( CET_FOCUS_LOOSE ) );
        GenerateEventForParent( m_pFocusedComponent, m_pFocusedComponent->Id(), OET_FOCUS_LOOSE, ( GR::up )pComponent );
      }
      m_pFocusedComponent = NULL;
    }

    if ( pComponent )
    {
      m_pFocusedComponent = pComponent;
      m_pFocusedComponent->ModifyFlags( GUI::COMPFT_FOCUSED );
      if ( ( !m_ShuttingDown )
      &&   ( !m_pFocusedComponent->m_ShuttingDown ) )
      {
        m_pFocusedComponent->ProcessEvent( ComponentEvent( CET_FOCUS_GET ) );
        GenerateEventForParent( pComponent, pComponent->Id(), OET_FOCUS_GET, ( GR::up )pOldFocused );
      }
    }
  }
        


  Component* ComponentContainer::GetFirstChildComponent() const
  {
    if ( m_Components.empty() )
    {
      return NULL;
    }
    return m_Components.front();
  }



  Component* ComponentContainer::GetNextChildComponent( Component* pComp ) const
  {
    tListComponents::const_iterator   it = std::find( m_Components.begin(), m_Components.end(), pComp );
    if ( it == m_Components.end() )
    {
      return NULL;
    }
    it++;
    if ( it == m_Components.end() )
    {
      return NULL;
    }
    return *it;
  }



  Component* ComponentContainer::GetPrevChildComponent( Component* pComp ) const
  {
    tListComponents::const_reverse_iterator   it = std::find( m_Components.rbegin(), m_Components.rend(), pComp );
    if ( it == m_Components.rend() )
    {
      return NULL;
    }
    it++;
    if ( it == m_Components.rend() )
    {
      return NULL;
    }
    return *it;
  }



  Component* ComponentContainer::FindNextTabComponent( Component* pOldFocusedComponent, bool Previous )
  {
    if ( m_Components.empty() )
    {
      return NULL;
    }

    bool    myComponentFound = false;

    if ( Previous )
    {
      tListComponents::reverse_iterator   it( m_Components.rbegin() );
      while ( it != m_Components.rend() )
      {
        Component*   pOtherComponent = *it;

        if ( pOtherComponent->IsVisible() )
        {
          if ( ( !( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP ) )
          &&   ( pOtherComponent->IsEnabled() ) )
          {
            // prüfen, ob es ein Child gibt, das Tab-Stop hat!
            Component*   pChild = pOtherComponent->FindNextTabComponent( pOldFocusedComponent, Previous );
            if ( pChild )
            {
              return pChild;
            }

            ++it;
            continue;
          }
          if ( ( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP )
          &&   ( myComponentFound ) )
          {
            return pOtherComponent;
          }
        }
        if ( pOtherComponent == pOldFocusedComponent )
        {
          myComponentFound = true;
        }

        ++it;
      }

      // keine gefunden, nach hinten suchen
      it = m_Components.rbegin();
      while ( it != m_Components.rend() )
      {
        Component*   pOtherComponent = *it;

        if ( ( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP )
        &&   ( pOtherComponent->IsVisible() )
        &&   ( pOtherComponent->IsEnabled() ) )
        {
          return pOtherComponent;
        }
        ++it;
      }
      return pOldFocusedComponent;
    }

    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component*   pOtherComponent = *it;

      if ( pOtherComponent->IsVisible() )
      {
        if ( ( !( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP ) )
        &&   ( pOtherComponent->IsEnabled() ) )
        {
          // prüfen, ob es ein Child gibt, das Tab-Stop hat!
          Component*   pChild = pOtherComponent->FindNextTabComponent( pOldFocusedComponent, Previous );
          if ( pChild )
          {
            return pChild;
          }

          ++it;
          continue;
        }
        if ( ( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP )
        &&   ( myComponentFound ) )
        {
          return pOtherComponent;
        }
      }
      if ( pOtherComponent == pOldFocusedComponent )
      {
        myComponentFound = true;
      }

      ++it;
    }

    // keine gefunden, nach vorne suchen
    it = m_Components.begin();
    while ( it != m_Components.end() )
    {
      Component*   pOtherComponent = *it;

      if ( ( pOtherComponent->Flags() & GUI::COMPFT_TAB_STOP )
      &&   ( pOtherComponent->IsVisible() )
      &&   ( pOtherComponent->IsEnabled() ) )
      {
        return pOtherComponent;
      }
      ++it;
    }
    return pOldFocusedComponent;
  }

}