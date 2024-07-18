#include <Misc/Misc.h>

#include <Controls/Component.h>
#include <Controls/ComponentDisplayerBase.h>
#include <Interface\IFont.h>

#include <String/StringUtil.h>
#include <String/XML.h>

#include <String/XML.h>

#include <Lang/EventQueue.h>

#include <debug\debugclient.h>



namespace GUI
{

  Component::Component( GR::u32 Id ) :
    m_pFont( NULL ),
    m_UserData( 0 ),
    m_Style( 0 ),
    m_VisualStyle( GUI::VFT_DEFAULT ),
    m_TextAlignment( GUI::AF_DEFAULT ),
    m_HottipID( -1 ),
    m_DoubleTextOffset( 1, 1 ),
    m_ItemData( 0 )
  {
    m_ClassName             = "Component";

    m_Position.x            = 0;
    m_Position.y            = 0;
    m_Width                 = 0;
    m_Height                = 0;
    m_ComponentFlags        = GUI::COMPFT_DEFAULT;
    m_ID                    = Id;

    m_pMouseOverComponent   = NULL;
    m_pDraggingComponent    = NULL;
    m_pDraggingComponentContent = NULL;
    m_pParent               = NULL;
    m_pOwner                = NULL;

    for ( int i = 0; i < GUI::COL_LAST_ENTRY; ++i )
    {
      m_UseCustomColors[i] = false;
    }

    RecalcClientRect();
  }



  Component::Component( int X, int Y, int Width, int Height, GR::u32 Id ) :
    m_pFont( NULL ),
    m_UserData( 0 ),
    m_Style( 0 ),
    m_VisualStyle( GUI::VFT_DEFAULT ),
    m_TextAlignment( GUI::AF_DEFAULT ),
    m_HottipID( -1 ),
    m_DoubleTextOffset( 1, 1 ),
    m_ItemData( 0 )
  {
    m_ClassName             = "Component";

    m_Position.x            = X;
    m_Position.y            = Y;
    m_Width                 = Width;
    m_Height                = Height;
    m_ComponentFlags        = GUI::COMPFT_DEFAULT;
    m_ID                    = Id;

    m_pMouseOverComponent   = NULL;
    m_pDraggingComponent    = NULL;
    m_pDraggingComponentContent = NULL;
    m_pParent               = NULL;
    m_pOwner                = NULL;

    for ( int i = 0; i < GUI::COL_LAST_ENTRY; ++i )
    {
      m_UseCustomColors[i] = false;
    }

    RecalcClientRect();
  }



  Component::Component( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id ) :
    m_pFont( NULL ),
    m_Style( 0 ),
    m_UserData( 0 ),
    m_VisualStyle( GUI::VFT_DEFAULT ),
    m_TextAlignment( GUI::AF_DEFAULT ),
    m_HottipID( -1 ),
    m_DoubleTextOffset( 1, 1 ),
    m_ItemData( 0 )
  {
    m_ClassName             = "Component";

    m_Position.x            = X;
    m_Position.y            = Y;
    m_Width                 = Width;
    m_Height                = Height;
    m_ComponentFlags        = GUI::COMPFT_DEFAULT;
    m_Caption               = Caption;
    m_ID                    = Id;

    m_pMouseOverComponent   = NULL;
    m_pDraggingComponent    = NULL;
    m_pDraggingComponentContent = NULL;
    m_pParent               = NULL;
    m_pOwner                = NULL;

    for ( int i = 0; i < GUI::COL_LAST_ENTRY; ++i )
    {
      m_UseCustomColors[i] = false;
    }

    RecalcClientRect();
  }



  Component::~Component()
  {
  }



  void Component::RemoveAllControls()
  {
    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      delete (Component*)( *it );
      it++;
    }
    m_Components.clear();
  }



  void Component::SetLocation( int X, int Y )
  {
    ComponentEvent    Event( CET_SET_POSITION );

    Event.Position.x = X;
    Event.Position.y = Y;

    ProcessEvent( Event );
  }



  void Component::SetSize( int Width, int Height )
  {
    ComponentEvent    EventSize( CET_SET_SIZE );

    EventSize.Position.x   = Width;
    EventSize.Position.y   = Height;

    ProcessEvent( EventSize );
  }



  void Component::SetClientSize( int Width, int Height )
  {
    ComponentEvent    EventSize( CET_SET_CLIENT_SIZE );

    EventSize.Position.x   = Width;
    EventSize.Position.y   = Height;

    ProcessEvent( EventSize );
  }



  void Component::SetCaption( const GR::Char* szCaption )
  {
    ComponentEvent   Event( CET_SET_CAPTION );

    GR::String  text = szCaption;

    Event.Value = (GR::up)&text;

    ProcessEvent( Event );
  }



  void Component::SetCaption( const GR::String& Caption )
  {
    ComponentEvent   Event( CET_SET_CAPTION );

    GR::String    text = Caption;

    Event.Value = (GR::up)&text;

    ProcessEvent( Event );
  }



  void Component::Add( Component *pComponent )
  {
    if ( pComponent == NULL )
    {
      return;
    }
    tListComponents::iterator   it( m_Components.begin() );

    while ( it != m_Components.end() )
    {
      if ( *it == pComponent )
      {
        // die Component ist bereits eingetragen!
        return;
      }
      it++;
    }
    if ( pComponent == this )
    {
      dh::Log( "Tried to add component to itself!" );
      return;
    }

    ComponentContainer::Add( pComponent );

    pComponent->m_pParent = this;
    if ( pComponent->Owner() == NULL )
    {
      pComponent->SetOwner( this );
    }
  }



  void Component::Remove( Component *pComponent )
  {
    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      if ( *it == pComponent )
      {
        // die Component ist eingetragen, also raus damit!
        ( *it )->m_pParent = NULL;
        ( *it )->m_pParentContainer = NULL;
        m_Components.erase( it );
        return;
      }
      it++;
    }

  }



  bool Component::DefaultEventProc( const ComponentEvent& Event )
  {
    switch ( Event.Type )
    {
      case CET_STYLE_CHANGED:
        Invalidate();
        return true;
      case CET_SET_CAPTION:
        {
          GR::String* pString = (GR::String*)Event.Value;

          if ( m_Caption != *pString )
          {
            m_Caption = *pString;
            Invalidate();
          }
        }
        return true;
      case CET_DESTROY:
        {
          ComponentContainer*    pTopLevel = GetTopLevelParent();
          if ( pTopLevel )
          {
            if ( pTopLevel->GetModalComponent() == this )
            {
              pTopLevel->SetModalMode();
            }
          }
          GenerateEventForParent( OET_DESTROY );
        }
        break;
      case CET_MOUSE_IN:
        GenerateEventForParent( OET_MOUSE_IN );
        break;
      case CET_MOUSE_OUT:
        GenerateEventForParent( OET_MOUSE_OUT );
        break;
      case CET_NC_MOUSE_UPDATE:
        if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
        {
          if ( !IsMouseInside() )
          {
            // TODO - Mouse-Out bei anderem
            m_ComponentFlags |= GUI::COMPFT_MOUSE_INSIDE;

            ComponentEvent     mouseinEvent( CET_MOUSE_IN );

            mouseinEvent.MouseButtons   = Event.MouseButtons;
            mouseinEvent.Position       = Event.Position;
            ProcessEvent( mouseinEvent );
          }
          if ( ( Event.MouseButtons )
          &&   ( !( Flags() & GUI::COMPFT_NOT_FOCUSABLE ) ) )
          {
            SetFocus();
          }
          if ( Event.MouseButtons & 1 )
          {
            if ( m_ComponentFlags & GUI::COMPFT_DRAG_ENABLED )
            {
              if ( StartDragging() )
              {
                GR::tPoint    ptScreen = Event.Position;
                LocalToScreen( ptScreen );
                m_DragPos = ptScreen;
                GetTopLevelParent()->m_pDraggingComponent = this;
                return true;
              }
            }
          }
        }
        break;
      case CET_MOUSE_UPDATE:
        {
          if ( ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
          &&   ( !( m_ComponentFlags & GUI::COMPFT_DONT_CATCH_MOUSE ) ) )
          {
            if ( ( !IsMouseInside() )
            &&   ( IsMouseInsideNonClientArea( Event.Position ) ) )
            {
              m_ComponentFlags |= GUI::COMPFT_MOUSE_INSIDE;

              ComponentEvent     mouseinEvent( CET_MOUSE_IN );

              mouseinEvent.MouseButtons   = Event.MouseButtons;
              mouseinEvent.Position       = Event.Position;
              ProcessEvent( mouseinEvent );
            }

            GenerateEventForParent( OET_MOUSE_UPDATE, Event.Position.x, Event.Position.y, Event.MouseButtons, Event.Value );

            if ( ( Event.MouseButtons & 1 )
            &&   ( IsFocused() ) )
            {
              if ( m_ComponentFlags & GUI::COMPFT_DRAG_CONTENT )
              {
                if ( GetTopLevelParent()->m_pDraggingComponentContent == NULL )
                {
                  if ( ( math::absolute( Event.Position.x - m_MouseDownPos.x ) >= 5 )
                  ||   ( math::absolute( Event.Position.y - m_MouseDownPos.y ) >= 5 ) )
                  {
                    // drag offset big enough
                    GUI::DragContentInfo    dragInfo;

                    ComponentEvent  dragEvent( CET_DRAG_CONTENT_START, this );
                    dragEvent.Value = ( GR::ip )&dragInfo;

                    if ( ( ProcessEvent( dragEvent ) )
                    &&   ( dragInfo.pComponentDragContent ) )
                    {
                      dragInfo.pComponentDragContent->StartDragging();
                      // to not check ourselves as drag drop target
                      dragInfo.pComponentDragContent->ModifyFlags( GUI::COMPFT_DONT_CATCH_MOUSE );
                      dragInfo.pComponentDragSource = this;

                      GetTopLevelParent()->m_pDraggingComponentContent = dragInfo.pComponentDragContent;
                      GetTopLevelParent()->Add( dragInfo.pComponentDragContent );
                      GetTopLevelParent()->m_pDraggingComponentSource = this;
                      GR::tPoint    pos( Event.Position );
                      LocalToScreen( pos );
                      m_DragPos = dragInfo.DragOffset;
                      dragInfo.pComponentDragContent->SetLocation( pos.x + m_DragPos.x, pos.y + m_DragPos.y );

                      GenerateEventForParent( OET_DRAG_CONTENT_START, ( GR::up )&dragInfo );
                    }
                    else
                    {
                      dragInfo.DragOffset = Event.Position;
                      GenerateEventForParent( OET_DRAG_CONTENT_QUERY, (GR::up)&dragInfo );
                      dragInfo.DragOffset.Clear();
                      if ( dragInfo.pComponentDragContent )
                      {
                        dragInfo.pComponentDragContent->StartDragging();
                        // to not check ourselves as drag drop target
                        dragInfo.pComponentDragContent->ModifyFlags( GUI::COMPFT_DONT_CATCH_MOUSE );
                        dragInfo.pComponentDragSource = this;

                        GetTopLevelParent()->m_pDraggingComponentContent = dragInfo.pComponentDragContent;
                        GetTopLevelParent()->Add( dragInfo.pComponentDragContent );
                        GetTopLevelParent()->m_pDraggingComponentSource = this;
                        GR::tPoint    pos( Event.Position );
                        LocalToScreen( pos );
                        m_DragPos = dragInfo.DragOffset;
                        dragInfo.pComponentDragContent->SetLocation( pos.x + m_DragPos.x, pos.y + m_DragPos.y );

                        GenerateEventForParent( OET_DRAG_CONTENT_START, ( GR::up )&dragInfo );
                      }
                    }
                    return true;
                  }
                }
              }
            }
          }
        }
        break;
      case CET_MOUSE_DOWN:
        {
          // Komponente draggen?
          if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
          {
            GenerateEventForParent( OET_MOUSE_DOWN, Event.Position.x, Event.Position.y, Event.MouseButtons, Event.Value );

            if ( GetComponentAt( Event.Position, false ) == NULL )
            {
              // ich bin definitiv das getroffene
              if ( !( Flags() & GUI::COMPFT_NOT_FOCUSABLE ) )
              {
                SetFocus();
              }
              m_MouseDownPos = Event.Position;
              return true;
            }
          }
        }
        break;
      case CET_MOUSE_UP:
        {
          if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
          {
            GenerateEventForParent( OET_MOUSE_UP, Event.Position.x, Event.Position.y, Event.MouseButtons, Event.Value );
          }
        }
        break;
      case CET_MOUSE_RDOWN:
        {
          if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
          {
            GenerateEventForParent( OET_MOUSE_RDOWN, Event.Position.x, Event.Position.y, Event.MouseButtons, Event.Value );

            if ( GetComponentAt( Event.Position, false ) == NULL )
            {
              // ich bin definitiv das getroffene
              if ( !( Flags() & GUI::COMPFT_NOT_FOCUSABLE ) )
              {
                SetFocus();
              }
            }
          }
        }
        break;
      case CET_MOUSE_RUP:
        {
          if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
          {
            GenerateEventForParent( OET_MOUSE_RUP, Event.Position.x, Event.Position.y, Event.MouseButtons, Event.Value );
          }
        }
        break;
      case CET_MOUSE_WHEEL:
        if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
        {
          GenerateEventForParent( OET_MOUSE_WHEEL, Event.Position.x, Event.Position.y, Event.MouseButtons, Event.Value );
        }
        break;
      case CET_VISUAL_STYLE_CHANGED:
        RecalcClientRect();
        return true;
      case CET_SET_SIZE:
        m_Width  = Event.Position.x;
        m_Height = Event.Position.y;
        RecalcClientRect();
        GenerateEvent( OET_SET_SIZE, m_Width, m_Height );
        return true;
      case CET_SET_CLIENT_SIZE:
        {
          GR::tPoint      ptClientSize( Event.Position );
          AdjustRect( ptClientSize, true );

          m_Width  = ptClientSize.x;
          m_Height = ptClientSize.y;

          RecalcClientRect();
          GenerateEvent( OET_SET_SIZE, m_Width, m_Height );
        }
        return true;
      case CET_SET_POSITION:
        m_Position.x = Event.Position.x;
        m_Position.y = Event.Position.y;
        GenerateEvent( OET_SET_POSITION, m_Position.x, m_Position.y );
        return true;
      case CET_KEY_PRESSED:
        GenerateEventForParent( OET_KEY_PRESSED, Event.Character );
        break;
      case CET_KEY_DOWN:
        GenerateEventForParent( OET_KEY_DOWN, Event.Character );
        break;
      case CET_KEY_UP:
        GenerateEventForParent( OET_KEY_UP, Event.Character );
        break;
      case CET_ENABLE:
        if ( Event.Value )
        {
          m_ComponentFlags &= ~GUI::COMPFT_DISABLED;
        }
        else
        {
          m_ComponentFlags |= GUI::COMPFT_DISABLED;
        }
        break;
      case CET_FOCUS_LOOSE:
        ComponentContainer::GenerateEvent( this, Id(), OET_FOCUS_LOOSE );
        break;
    }

    return false;

  }



  bool Component::ProcessEvent( const ComponentEvent& Event )
  {
    if ( m_ComponentFlags & GUI::COMPFT_DESTROYED )
    {
      return false;
    }

    if ( !IsEnabled() )
    {
      if ( ( Event.Type == CET_MOUSE_DOWN )
      ||   ( Event.Type == CET_MOUSE_IN )
      ||   ( Event.Type == CET_MOUSE_OUT )
      ||   ( Event.Type == CET_MOUSE_RDOWN )
      ||   ( Event.Type == CET_MOUSE_RUP )
      ||   ( Event.Type == CET_MOUSE_UP )
      ||   ( Event.Type == CET_MOUSE_UPDATE )
      ||   ( Event.Type == CET_MOUSE_WHEEL )
      ||   ( Event.Type == CET_NC_MOUSE_DOWN )
      ||   ( Event.Type == CET_NC_MOUSE_RDOWN )
      ||   ( Event.Type == CET_NC_MOUSE_RUP )
      ||   ( Event.Type == CET_NC_MOUSE_UP )
      ||   ( Event.Type == CET_NC_MOUSE_UPDATE )
      ||   ( Event.Type == CET_KEY_DOWN )
      ||   ( Event.Type == CET_KEY_PRESSED )
      ||   ( Event.Type == CET_KEY_UP ) )
      {
        return false;
      }
    }

    switch ( Event.Type )
    {
      case CET_ADDED:
        GenerateEvent( OET_ADDED_TO_PARENT );
        break;
      case CET_REMOVED:
        GenerateEvent( OET_REMOVED_FROM_PARENT );
        break;
      case CET_CLOSE:
        PerformClose( (GR::u32)Event.Value );
        break;
      case CET_ADDED_CHILD:
        RaiseHandlerEvent( GUI::OutputEvent( OET_ADDED_CHILD, Event.pComponent->Id(), Event.pComponent ) );
        break;
      case CET_REMOVED_CHILD:
        RaiseHandlerEvent( GUI::OutputEvent( OET_REMOVED_CHILD, Event.pComponent->Id(), Event.pComponent ) );
        break;
    }

    bool  bResult = ComponentContainer::ProcessEvent( Event );

    if ( bResult )
    {
      return true;
    }
    return DefaultEventProc( Event );
  }



  void Component::Enable( bool bEnable )
  {
    ComponentEvent   Event( CET_ENABLE );
    Event.Value = bEnable;
    ProcessEvent( Event );
  }



  Component* Component::GetComponentParent() const
  {
    return m_pParent;
  }



  Component* Component::Owner() const
  {
    return m_pOwner;
  }



  void Component::SetOwner( Component* pNewOwner )
  {
    m_pOwner = pNewOwner;
  }



  ComponentContainer* Component::GetParentContainer() const
  {
    return m_pParentContainer;
  }



  void Component::RaiseHandlerEvent( const GUI::OutputEvent& Event )
  {
    // gibt es einen Handler?
    Component::tMapEventHandler::iterator    itHandler( m_EventHandler.find( Event.Type ) );
    if ( itHandler != m_EventHandler.end() )
    {
      // make a copy of the list in case it's modified during handling
      Component::tEventHandlers    listHandler = itHandler->second;

      Component::tEventHandlers::iterator   itList( listHandler.begin() );
      while ( itList != listHandler.end() )
      {
        Component::tEventHandlerFunction   funcHandler = *itList;

        funcHandler( Event );

        ++itList;
      }
    }
  }




  void Component::GenerateEvent( OutputEventType Type,
                                  GR::up Param1, GR::up Param2, 
                                  GR::up Param3, GR::up Param4 )
  {
    ComponentContainer::GenerateEvent( this, m_ID, Type, Param1, Param2, Param3, Param4 );
  }



  void Component::GenerateEventForParent( OutputEventType Type,
                                           GR::up Param1, GR::up Param2, 
                                           GR::up Param3, GR::up Param4 )
  {
    if ( IsDestroyed() )
    {
      return;
    }
    ComponentContainer::GenerateEventForParent( this, m_ID, Type, Param1, Param2, Param3, Param4 );
  }



  void Component::RecursiveChildEvent( OutputEventType Event )
  {
    tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      auto  pChild = *it;

      pChild->RecursiveChildEvent( Event );

      ++it;
    }
    GenerateEvent( Event );
  }



  void Component::SetVisible( bool Visible )
  {
    if ( Visible )
    {
      m_ComponentFlags &= ~GUI::COMPFT_INVISIBLE;
      GenerateEvent( OET_VISIBILITY_CHANGED, 1 );
    }
    else
    {
      m_ComponentFlags |= GUI::COMPFT_INVISIBLE;

      // wird das fokussierte Element mitversteckt?
      ComponentContainer*    pTopLevelParent = GetTopLevelParent();
      if ( ( pTopLevelParent )
      &&   ( pTopLevelParent->GetFocusedComponent() ) )
      {
        if ( ( pTopLevelParent->GetFocusedComponent()->IsIndirectChild( this ) )
        ||   ( pTopLevelParent->GetFocusedComponent() == this ) )
        {
          Component* pNewComp = GetParentContainer()->FindNextTabComponent( pTopLevelParent->GetFocusedComponent() );

          if ( pNewComp )
          {
            pNewComp->SetFocus();
          }
        }
      }
      GenerateEvent( OET_VISIBILITY_CHANGED, 0 );
    }
  }



  bool Component::IsVisible()
  {
    if ( m_ComponentFlags & GUI::COMPFT_DESTROYED )
    {
      return false;
    }
    return !( m_ComponentFlags & GUI::COMPFT_INVISIBLE );
  }



  bool Component::IsEnabled()
  {
    if ( m_ComponentFlags & GUI::COMPFT_DESTROYED )
    {
      return false;
    }
    if ( m_ComponentFlags & GUI::COMPFT_DISABLED )
    {
      return false;
    }
    return !IsIndirectDisabled();
  }



  bool Component::IsFocused()
  {
    return !!( m_ComponentFlags & GUI::COMPFT_FOCUSED );
  }



  bool Component::IsActive()
  {
    return !!( m_ComponentFlags & GUI::COMPFT_ACTIVE );
  }



  bool Component::IsMouseInside()
  {
    if ( m_ComponentFlags & GUI::COMPFT_MOUSE_INSIDE )
    {
      return true;
    }
    return false;
  }



  bool Component::IsMouseInside( const GR::tPoint& MousePos )
  {
    if ( m_ComponentFlags & GUI::COMPFT_DONT_CATCH_MOUSE )
    {
      return false;
    }

    return m_ClientRect.Contains( MousePos );
  }



  bool Component::IsMouseInsideNonClientArea( const GR::tPoint& MousePos )
  {
    if ( m_ComponentFlags & GUI::COMPFT_DONT_CATCH_MOUSE )
    {
      return false;
    }

    if ( ( MousePos.x >= 0 )
    &&   ( MousePos.y >= 0 )
    &&   ( MousePos.x < m_Width )
    &&   ( MousePos.y < m_Height ) )
    {
      return true;
    }
    return false;
  }



  int Component::Width() const
  { 
    return m_Width;
  }



  int Component::Height() const
  { 
    return m_Height;
  }



  GR::tPoint Component::Position() const
  { 
    return m_Position;
  }



  GR::tPoint Component::Size() const
  {
    return GR::tPoint( m_Width, m_Height );
  }



  GR::tPoint Component::DragPosition() const
  { 
    return m_DragPos;
  }



  GR::String Component::GetCaption() const
  {
    return m_Caption;
  }



  bool Component::StartDragging()
  {
    if ( !( m_ComponentFlags & GUI::COMPFT_DRAGGED ) )
    {
      m_ComponentFlags |= GUI::COMPFT_DRAGGED;
      return true;
    }
    return false;
  }



  void Component::StopDragging()
  {
    if ( m_ComponentFlags & GUI::COMPFT_DRAGGED )
    {
      m_ComponentFlags &= ~GUI::COMPFT_DRAGGED;
    }
  }



  void Component::GetComponentRect( GR::tRect& Rect ) const
  {
    Rect.Set( 0, 0, m_Width, m_Height );
  }



  void Component::AdjustRect( GR::tPoint& Size, bool ClientToNC )
  {
    int     deltaX = GetBorderWidth( GUI::BT_EDGE_LEFT ) + GetBorderWidth( GUI::BT_EDGE_RIGHT );
    int     deltaY = GetBorderHeight( GUI::BT_EDGE_TOP ) + GetBorderHeight( GUI::BT_EDGE_BOTTOM );

    if ( ClientToNC )
    {
      Size.x += deltaX;
      Size.y += deltaY;
    }
    else
    {
      Size.x -= deltaX;
      Size.y -= deltaY;
    }
  }



  void Component::RecalcClientRect()
  {
    m_ClientRect.Set( 0, 0, m_Width, m_Height );

    m_ClientRect.Left   += GetBorderWidth( GUI::BT_EDGE_LEFT );
    m_ClientRect.Right  -= GetBorderWidth( GUI::BT_EDGE_RIGHT );
    m_ClientRect.Top    += GetBorderHeight( GUI::BT_EDGE_TOP );
    m_ClientRect.Bottom -= GetBorderHeight( GUI::BT_EDGE_BOTTOM );
  }



  GR::u32 Component::Id() const
  {
    return m_ID;
  }



  void Component::Id( GR::u32 NewId )
  {
    m_ID = NewId;
  }



  GR::u32 Component::HottipId() const
  {
    return m_HottipID;
  }



  void Component::HottipId( GR::u32 NewId )
  {
    m_HottipID = NewId;
  }



  void Component::SetModal()
  {
    ComponentContainer*   pTopContainer = GetTopLevelParent();

    if ( pTopContainer == NULL )
    {
      return;
    }
    pTopContainer->SetModalMode( this );
  }



  void Component::Update( float ElapsedTime )
  {
  }



  bool Component::IsIndirectChild( ComponentContainer* pContainer )
  {
    if ( pContainer == NULL )
    {
      return false;
    }

    if ( pContainer == this )
    {
      return true;
    }

    Component*   pDummyContainer = GetComponentParent();
    while ( pDummyContainer )
    {
      if ( (ComponentContainer*)pDummyContainer == pContainer )
      {
        return true;
      }
      pDummyContainer = pDummyContainer->GetComponentParent();
    }

    pDummyContainer = this;
    while ( pDummyContainer )
    {
      if ( (ComponentContainer*)pDummyContainer->Owner() == pContainer )
      {
        return true;
      }
      if ( ( pDummyContainer->GetComponentParent() )
      &&   ( pDummyContainer->GetComponentParent()->Owner() == pContainer ) )
      {
        return true;
      }
      if ( pDummyContainer->Owner() == NULL )
      {
        break;
      }
      pDummyContainer = pDummyContainer->Owner();
    }

    return false;
  }



  void Component::SetFocus()
  {
    if ( !m_pParentContainer )
    {
      return;
    }
    if ( !IsEnabled() )
    {
      return;
    }

    GetTopLevelParent()->SetFocus( this );
  }



  void Component::Close( const GR::u32 CloseCode )
  {
    GUI::ComponentEvent  closeEvent( CET_CLOSE, this );

    closeEvent.Value = CloseCode;
    PostComponentEvent( closeEvent );
  }



  void Component::PerformClose( const GR::u32 CloseCode )
  {
    if ( !m_pParentContainer )
    {
      return;
    }

    GenerateEventForParent( OET_CLOSE, CloseCode );

    GetParentContainer()->Delete( this );
  }



  void Component::SetFont( Interface::IFont* pFont )
  {
    m_pFont = pFont;
    ProcessEvent( ComponentEvent( CET_FONT_CHANGED, this ) );
  }



  void Component::SetUserData( GR::up dwData )
  {
    m_UserData = dwData;
  }



  GR::up Component::GetUserData() const
  {
    return m_UserData;
  }



  void Component::SetUserText( const GR::String& Text )
  {
    m_UserText = Text;
  }



  GR::String Component::GetUserText() const
  {
    return m_UserText;
  }



  bool Component::IsDestroyed()
  {
    return ( ( m_ComponentFlags & GUI::COMPFT_DESTROYED ) != 0 );
  }



  GR::u32 Component::ModifyStyle( GR::u32 Add, GR::u32 Remove )
  {
    m_Style &= ~Remove;
    m_Style |= Add;

    ProcessEvent( ComponentEvent( CET_STYLE_CHANGED, this ) );

    SetSize( Width(), Height() );

    return m_Style;
  }



  GR::u32 Component::Style() const
  {
    return m_Style;
  }



  GR::u32 Component::ModifyFlags( GR::u32 Add, GR::u32 Remove )
  {
    m_ComponentFlags &= ~Remove;
    m_ComponentFlags |= Add;

    ProcessEvent( ComponentEvent( CET_FLAGS_CHANGED, this ) );
    return m_ComponentFlags;
  }



  GR::u32 Component::Flags() const
  {
    return m_ComponentFlags;
  }



  void Component::ModifyVisualStyle( GR::u32 Add, GR::u32 Remove )
  {
    m_VisualStyle &= ~Remove;
    m_VisualStyle |= Add;

    ProcessEvent( ComponentEvent( CET_VISUAL_STYLE_CHANGED, this ) );
    SetSize( Width(), Height() );
  }



  GR::u32 Component::VisualStyle() const
  {
    return m_VisualStyle;
  }



  void Component::TextAlignment( GR::u32 Alignment )
  {
    m_TextAlignment = Alignment;
  }



  GR::u32 Component::TextAlignment() const
  {
    return m_TextAlignment;
  }



  void Component::ParseColorFromAttribute( GUI::eColorIndex Color, const GR::String& AttributeName, GR::Strings::XMLElement* pElement )
  {
    if ( pElement->HasAttribute( AttributeName ) )
    {
      SetColor( Color, GR::Convert::ToU32( pElement->Attribute( AttributeName ), 16 ) );
    }
  }



  void Component::ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
  {
    if ( pElement == NULL )
    {
      return;
    }

    SetLocation( GR::Convert::ToI32( pElement->Attribute( "X" ) ), GR::Convert::ToI32( pElement->Attribute( "Y" ) ) );
    if ( pElement->HasAttribute( "W" ) )
    {
      SetSize( GR::Convert::ToU32( pElement->Attribute( "W" ) ), GR::Convert::ToU32( pElement->Attribute( "H" ) ) );
    }
    else
    {
      SetSize( GR::Convert::ToU32( pElement->Attribute( "Width" ) ), GR::Convert::ToU32( pElement->Attribute( "Height" ) ) );
    }
    m_DefaultEventScript = pElement->Attribute( "DefaultEventScript" );
    SetCaption( pElement->Attribute( "Caption" ) );
    Id( GR::Convert::ToI32( pElement->Attribute( "ID" ) ) );
    HottipId( GR::Convert::ToI32( pElement->Attribute( "HottipID" ) ) );

    m_UserData = GR::Convert::ToIP( pElement->Attribute( "UserData" ) );
    m_UserText = pElement->Attribute( "UserText" );

    ModifyVisualStyle( GR::Convert::ToU32( pElement->Attribute( "Edge" ) ), 0xffffffff  );
    ModifyFlags( GR::Convert::ToU32( pElement->Attribute( "Flags" ) ), 0xffffffff );
    if ( pElement->HasAttribute( "Style" ) )
    {
      ModifyStyle( GR::Convert::ToU32( pElement->Attribute( "Style" ) ), 0xffffffff );
    }
    TextAlignment( GR::Convert::ToU32( pElement->Attribute( "TextAlignment" ) ) );

    if ( ( GR::Strings::CompareUpperCase( pElement->Attribute( "Enabled" ), "FALSE" ) == 0 )
    ||   ( GR::Strings::CompareUpperCase( pElement->Attribute( "Enabled" ), "NO" ) == 0 )
    ||   ( GR::Strings::CompareUpperCase( pElement->Attribute( "Enabled" ), "N" ) == 0 ) )
    {
      Enable( false );
    }
    if ( ( GR::Strings::CompareUpperCase( pElement->Attribute( "Visible" ), "FALSE" ) == 0 )
    ||   ( GR::Strings::CompareUpperCase( pElement->Attribute( "Visible" ), "NO" ) == 0 )
    ||   ( GR::Strings::CompareUpperCase( pElement->Attribute( "Visible" ), "N" ) == 0 ) )
    {
      SetVisible( false );
    }

    SetSize( Width(), Height() );

    ParseColorFromAttribute( GUI::COL_ACTIVEBORDER, "Color.ActiveBorder", pElement );
    ParseColorFromAttribute( GUI::COL_ACTIVECAPTION, "Color.ActiveCaption", pElement );
    ParseColorFromAttribute( GUI::COL_APPWORKSPACE, "Color.AppWorkspace", pElement );
    ParseColorFromAttribute( GUI::COL_BACKGROUND, "Color.Background", pElement );
    ParseColorFromAttribute( GUI::COL_BTNFACE, "Color.ButtonFace", pElement );
    ParseColorFromAttribute( GUI::COL_BTNHIGHLIGHT, "Color.ButtonHighlight", pElement );
    ParseColorFromAttribute( GUI::COL_BTNSHADOW, "Color.ButtonShadow", pElement );
    ParseColorFromAttribute( GUI::COL_BTNTEXT, "Color.ButtonText", pElement );
    ParseColorFromAttribute( GUI::COL_CAPTIONTEXT, "Color.CaptionText", pElement );
    ParseColorFromAttribute( GUI::COL_CURSOR, "Color.Cursor", pElement );
    ParseColorFromAttribute( GUI::COL_GRADIENTACTIVECAPTION, "Color.GradientActiveCaption", pElement );
    ParseColorFromAttribute( GUI::COL_GRADIENTINACTIVECAPTION, "Color.GradientInactiveCaption", pElement );
    ParseColorFromAttribute( GUI::COL_GRAYTEXT, "Color.GrayText", pElement );
    ParseColorFromAttribute( GUI::COL_HIGHLIGHT, "Color.Highlight", pElement );
    ParseColorFromAttribute( GUI::COL_HIGHLIGHTTEXT, "Color.HighlightText", pElement );
    ParseColorFromAttribute( GUI::COL_HOTLIGHT, "Color.Hotlight", pElement );
    ParseColorFromAttribute( GUI::COL_HOTLIGHTTEXT, "Color.HotlightText", pElement );
    ParseColorFromAttribute( GUI::COL_INACTIVEBORDER, "Color.InactiveBorder", pElement );
    ParseColorFromAttribute( GUI::COL_INACTIVECAPTION, "Color.InactiveCaption", pElement );
    ParseColorFromAttribute( GUI::COL_INACTIVECAPTIONTEXT, "Color.InactiveCaptionText", pElement );
    ParseColorFromAttribute( GUI::COL_INFOBK, "Color.InfoBack", pElement );
    ParseColorFromAttribute( GUI::COL_INFOTEXT, "Color.InfoText", pElement );
    ParseColorFromAttribute( GUI::COL_MENU, "Color.Menu", pElement );
    ParseColorFromAttribute( GUI::COL_MENUBAR, "Color.MenuBar", pElement );
    ParseColorFromAttribute( GUI::COL_MENUHILIGHT, "Color.MenuHighlight", pElement );
    ParseColorFromAttribute( GUI::COL_MENUTEXT, "Color.MenuText", pElement );
    ParseColorFromAttribute( GUI::COL_NC_AREA, "Color.NCArea", pElement );
    ParseColorFromAttribute( GUI::COL_SCROLLBAR, "Color.Scrollbar", pElement );
    ParseColorFromAttribute( GUI::COL_THIN_BORDER, "Color.ThinBorder", pElement );
    ParseColorFromAttribute( GUI::COL_3DDKSHADOW, "Color.3dDarkShadow", pElement );
    ParseColorFromAttribute( GUI::COL_3DLIGHT, "Color.3dLight", pElement );
    ParseColorFromAttribute( GUI::COL_WINDOW, "Color.Window", pElement );
    ParseColorFromAttribute( GUI::COL_WINDOWFRAME, "Color.WindowFrame", pElement );
    ParseColorFromAttribute( GUI::COL_WINDOWTEXT, "Color.WindowText", pElement );
  }



  GR::tRect Component::GetWindowRect() const
  {
    GR::tRect   windowRect;

    GetWindowRect( windowRect );

    return windowRect;
  }



  void Component::GetWindowRect( GR::tRect& Rect ) const
  {
    Rect = GR::tRect();

    Rect.Size( Width(), Height() );

    const Component*   pComponent = this;

    while ( pComponent )
    {
      Rect.Offset( pComponent->Position() );

      const Component* pPrevComponent = pComponent;

      pComponent = pComponent->GetComponentParent();
      if ( pComponent )
      {
        Rect.Offset( pComponent->GetClientOffset() );
      }
      else
      {
        ComponentContainer* pContainer = pPrevComponent->GetParentContainer();
      }
    }
  }



  void Component::PropertiesToXML( GR::Strings::XMLElement* pElement )
  {
    pElement->AddAttribute( "ID",           CMisc::printf( "%d", Id() ) );
    pElement->AddAttribute( "HottipID",     CMisc::printf( "%d", HottipId() ) );
    if ( pElement->Attribute( "Class" ) == "" )
    {
      pElement->AddAttribute( "Class",  "Component" );
    }
    pElement->AddAttribute( "X",        CMisc::printf( "%d", Position().x ) );
    pElement->AddAttribute( "Y",        CMisc::printf( "%d", Position().y ) );
    pElement->AddAttribute( "Width",    CMisc::printf( "%d", Width() ) );
    pElement->AddAttribute( "Height",   CMisc::printf( "%d", Height() ) );
    pElement->AddAttribute( "Caption",  GetCaption() );

    pElement->AddAttribute( "Edge",           CMisc::printf( "%d", m_VisualStyle ) );
    pElement->AddAttribute( "Style",          CMisc::printf( "%d", m_Style ) );
    pElement->AddAttribute( "Flags",          CMisc::printf( "%d", m_ComponentFlags ) );
    pElement->AddAttribute( "TextAlignment",  CMisc::printf( "%d", m_TextAlignment ) );
  }



  int Component::GetBorderWidth( const GUI::eBorderType eType ) const
  {
    // TODO - nur Breite, nicht alle Borders!
    if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
    {
      return 1;
    }
    else if ( ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
    ||        ( m_VisualStyle & GUI::VFT_RAISED_BORDER ) )
    {
      return 2;
    }

    //return 1;
    return 0;
  }



  int Component::GetBorderHeight( const GUI::eBorderType eType ) const
  {
    // TODO - nur Breite, nicht alle Borders!
    if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
    {
      return 1;
    }
    else if ( ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
    ||        ( m_VisualStyle & GUI::VFT_RAISED_BORDER ) )
    {
      return 2;
    }

    //return 1;
    return 0;
  }



  GR::u32 Component::GetColor( GUI::eColorIndex Index ) const
  {
    if ( ( Index >= GUI::COL_FIRST_ENTRY )
    &&   ( Index < GUI::COL_LAST_ENTRY ) )
    {
      return m_Colors[Index];
    }
    return 0;
  }



  bool Component::UseCustomColor( GUI::eColorIndex Index ) const
  {
    if ( ( Index >= GUI::COL_FIRST_ENTRY )
    &&   ( Index < GUI::COL_LAST_ENTRY ) )
    {
      return m_UseCustomColors[Index];
    }
    return false;
  }



  void Component::SetColor( GUI::eColorIndex Index, const GR::u32 Color, bool IsDefaultColor )
  {
    if ( ( Index >= GUI::COL_FIRST_ENTRY )
    &&   ( Index < GUI::COL_LAST_ENTRY ) )
    {
      m_Colors[Index]          = Color;
      m_UseCustomColors[Index] = !IsDefaultColor;
    }
  }



  void Component::AddQueryHandler( GUI::QueryEventType::Value Type, tQueryEventHandlerFunction funcHandler )
  {
    tMapQueryEventHandler::iterator    itMap( m_QueryEventHandler.find( Type ) );
    if ( itMap != m_QueryEventHandler.end() )
    {
      tQueryEventHandlers&    listHandler = itMap->second;

      tQueryEventHandlers::iterator   itList( listHandler.begin() );
      while ( itList != listHandler.end() )
      {
        if ( *itList == funcHandler )
        {
          // schon drin
          return;
        }
        ++itList;
      }
    }
    m_QueryEventHandler[Type].push_back( funcHandler );
  }



  void Component::RemoveQueryHandler( GUI::QueryEventType::Value Type, tQueryEventHandlerFunction funcHandler )
  {
    tMapQueryEventHandler::iterator    itMap( m_QueryEventHandler.find( Type ) );
    if ( itMap != m_QueryEventHandler.end() )
    {
      tQueryEventHandlers&    listHandler = itMap->second;

      listHandler.remove( funcHandler );
    }
  }



  void Component::QueryEvent( GUI::QueryEvent& Event )
  {
    Component::tMapQueryEventHandler::iterator    itHandler( m_QueryEventHandler.find( Event.Type ) );
    if ( itHandler != m_QueryEventHandler.end() )
    {
      Component::tQueryEventHandlers&    listHandler = itHandler->second;

      Component::tQueryEventHandlers::iterator   itList( listHandler.begin() );
      while ( itList != listHandler.end() )
      {
        Component::tQueryEventHandlerFunction   funcHandler = *itList;

        funcHandler( Event );

        ++itList;
      }
    }
  }



  void Component::AddHandler( OutputEventType Type, tEventHandlerFunction funcHandler )
  {
    tMapEventHandler::iterator    itMap( m_EventHandler.find( Type ) );
    if ( itMap != m_EventHandler.end() )
    {
      tEventHandlers&    listHandler = itMap->second;

      tEventHandlers::iterator   itList( listHandler.begin() );
      while ( itList != listHandler.end() )
      {
        if ( *itList == funcHandler )
        {
          // schon drin
          return;
        }
        ++itList;
      }
    }
    m_EventHandler[Type].push_back( funcHandler );
  }



  void Component::RemoveHandler( OutputEventType Type, tEventHandlerFunction funcHandler )
  {
    tMapEventHandler::iterator    itMap( m_EventHandler.find( Type ) );
    if ( itMap != m_EventHandler.end() )
    {
      tEventHandlers&    listHandler = itMap->second;

      listHandler.remove( funcHandler );
    }
  }



  void Component::SetCapture()
  {
    ComponentContainer*    pContainer = GetTopLevelParent();
    if ( pContainer )
    {
      pContainer->SetCaptureToComponent( this );
    }
  }



  bool Component::IsIndirectDisabled()
  {
    Component* pComponent = GetComponentParent();

    while ( pComponent != NULL )
    {
      if ( !pComponent->IsEnabled() )
      {
        return true;
      }
      pComponent = pComponent->GetComponentParent();
    }
    return false;
  }



  bool Component::IsIndirectHidden()
  {
    if ( !IsVisible() )
    {
      return true;
    }

    Component* pComponent = GetComponentParent();

    while ( pComponent != NULL )
    {
      if ( !pComponent->IsVisible() )
      {
        return true;
      }
      pComponent = pComponent->GetComponentParent();
    }
    return false;
  }



  void Component::Center( ComponentContainer* pParent )
  {
    if ( pParent == NULL )
    {
      pParent = GetComponentParent();
    }
    if ( pParent == NULL )
    {
      pParent = GetParentContainer();
    }
    if ( pParent == NULL )
    {
      return;
    }
    GR::tRect     RectClient;
    
    pParent->GetClientRect( RectClient );

    SetLocation( ( RectClient.Width() - Width() ) / 2, ( RectClient.Height() - Height() ) / 2 );
  }



  Component* Component::GetComponent( GR::u32 Id )
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
      ++it;
    }
    // no direct sibling found, search sub components
    it = m_Components.begin();
    while ( it != m_Components.end() )
    {
      Component*  pComp = *it;

      if ( pComp->IsDestroyed() )
      {
        ++it;
        continue;
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



  void Component::ScreenToLocal( GR::tPoint& Pos )
  {
    ComponentContainer::ScreenToLocal( Pos, this );
  }



  void Component::ScreenToNonClient( GR::tPoint& Pos )
  {
    ComponentContainer::ScreenToNonClient( Pos, this );
  }



  void Component::LocalToScreen( GR::tPoint& Pos )
  {
    ComponentContainer::LocalToScreen( Pos, this );
  }



  void Component::LocalToScreen( GR::tRect& Rect )
  {
    ComponentContainer::LocalToScreen( Rect, this );
  }



  void Component::RaiseDefaultEvent()
  {
    if ( m_DefaultEventScript.empty() )
    {
      return;
    }
    size_t  sepPos = m_DefaultEventScript.find( ' ' );
    if ( sepPos != GR::String::npos )
    {
      GLOBAL_QUEUE.PostEvent( m_DefaultEventScript.substr( 0, sepPos ).c_str(), m_DefaultEventScript.substr( sepPos + 1 ).c_str() );
    }
    else
    {
      GLOBAL_QUEUE.PostEvent( m_DefaultEventScript.c_str() );
    }
  }



  void Component::Display( GUI::ComponentDisplayerBase* pDisplayer )
  {
    dh::Log( "Component::Display - not overridden!" );
  }



  void Component::DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer )
  {
    dh::Log( "Component::DisplayNonClient - not overridden!" );
  }



  void Component::SetTextureSection( const GUI::eBorderType Type, const XTextureSection& Section, GR::u32 ColorKey )
  {
    m_TextureSection[Type] = std::make_pair( Section, ColorKey );

    GUI::tListComponents::iterator   it( m_Components.begin() );
    while ( it != m_Components.end() )
    {
      Component* pChild = *it;

      pChild->SetTextureSection( Type, Section, ColorKey );

      ++it;
    }

    RecalcClientRect();
  }



  void Component::SetCustomTextureSection( const GR::u32 Type, const XTextureSection& TexSection, GR::u32 dwColorKey )
  {
    m_CustomTextureSections[Type] = std::make_pair( TexSection, dwColorKey );

    GUI::ComponentEvent   eventChange( CET_CUSTOM_SECTION_CHANGED, this );
    eventChange.Value = Type;
    ProcessEvent( eventChange );
  }



  XTextureSection Component::TextureSection( const GUI::eBorderType Type ) const
  {
    return m_TextureSection[Type].first;
  }



  XTextureSection Component::CustomTextureSection( const GR::u32 dwType ) const
  {
    tMapCustomTextureSections::const_iterator   it( m_CustomTextureSections.find( dwType ) );
    if ( it == m_CustomTextureSections.end() )
    {
      return XTextureSection();
    }
    return it->second.first;
  }



  GR::u32 Component::CustomTSColorKey( const GR::u32 dwType ) const
  {
    tMapCustomTextureSections::const_iterator   it( m_CustomTextureSections.find( dwType ) );
    if ( it == m_CustomTextureSections.end() )
    {
      return 0;
    }
    return it->second.second;
  }



  GR::u32 Component::GetSysColor( GUI::eColorIndex colIndex ) const
  {
    return ComponentDisplayerBase::Instance().GetSysColor( colIndex );
  }



  void Component::Invalidate()
  {
    ComponentDisplayerBase::Instance().Invalidate( this );
  }



  void Component::SetBaseColors()
  {
    for ( auto color = GUI::COL_FIRST_ENTRY; color < GUI::COL_LAST_ENTRY; color = (GUI::eColorIndex)( (int)color + 1 ) )
    {
      SetColor( color, GetSysColor( color ) );
    }
  }



}




