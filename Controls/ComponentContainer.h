#ifndef COMPONENT_CONTAINER_H
#define COMPONENT_CONTAINER_H



#include <string>
#include <list>
#include <vector>

#include <Controls/InputEvent.h>
#include <Controls/OutputEvent.h>
#include <Interface/IEventProducer.h>
#include <Interface/IEventListener.h>



namespace Interface
{
  struct IFont;
}

namespace GUI
{
  class ComponentDisplayerBase;
}

enum ComponentEventType
{
  CET_NULL                      = 0,
  CET_MOUSE_IN                  = 0x00000001,
  CET_MOUSE_OUT                 = 0x00000002,
  CET_MOUSE_UPDATE              = 0x00000003,
  CET_MOUSE_WHEEL               = 0x00000004,
  CET_MOUSE_DOWN                = 0x00000005,
  CET_MOUSE_UP                  = 0x00000006,
  CET_NC_MOUSE_UPDATE           = 0x00000007,
  CET_MOUSE_RDOWN               = 0x00000008,
  CET_MOUSE_RUP                 = 0x00000009,
  CET_NC_MOUSE_DOWN             = 0x0000000A,
  CET_NC_MOUSE_UP               = 0x0000000B,
  CET_NC_MOUSE_RDOWN            = 0x0000000C,
  CET_NC_MOUSE_RUP              = 0x0000000D,
  CET_KEY_DOWN                  = 0x00000020,
  CET_KEY_UP                    = 0x00000021,
  CET_KEY_PRESSED               = 0x00000022,

  CET_FOCUS_GET                 = 0x00000030,
  CET_FOCUS_LOOSE               = 0x00000031,

  CET_SET_SIZE                  = 0x00000100,
  CET_SET_POSITION              = 0x00000101,
  CET_DESTROY                   = 0x00000102,     // die Komponente wird zerstört
  CET_ADDED                     = 0x00000103,     // Komponente in Container eingesetzt
  CET_REMOVED                   = 0x00000104,     // Komponente in Container eingesetzt
  CET_ENABLE                    = 0x00000105,     // Komponente wird enabled/disabled
  CET_SET_CLIENT_SIZE           = 0x00000106,
  CET_SET_CAPTION               = 0x00000107,     // Caption setzen
  CET_STYLE_CHANGED             = 0x00000108,
  CET_FLAGS_CHANGED             = 0x00000109,
  CET_VISUAL_STYLE_CHANGED      = 0x0000010A,
  CET_CUSTOM_SECTION_CHANGED    = 0x0000010B,
  CET_INIT_AFTER_LOAD           = 0x0000010C,     // init after GUILoader created, use for ListControl column header IDs
  CET_ADDED_CHILD               = 0x0000010D,     // i'm the container adding a child
  CET_REMOVED_CHILD             = 0x0000010E,     // i'm the container removing a child

  CET_ATTACH_COMPONENT          = 0x0000010F,     // request to attach component (hottip for example)
  CET_DETACH_COMPONENT          = 0x00000110,     // request to detach component (hottip for example)

  CET_EXTENTS_CHANGED           = 0x00000200,     // die Top-Parent-Fenstergröße hat sich geändert

  CET_DRAG_CONTENT_START        = 0x00000300,     // drag content drag is started
  CET_DRAG_CONTENT_CANCEL       = 0x00000301,     // drag content drag is aborted/cancelled
  CET_DRAG_CONTENT_DROP         = 0x00000302,     // drag content is dropped

  // Component-spezifische Events
  CET_BUTTON_PUSHED             = 0x00010000,     // Button gedrückt

  CET_SET_ITEM_WIDTH            = 0x00010100,
  CET_SET_ITEM_HEIGHT           = 0x00010101,

  CET_CLOSE                     = 0x00010200      // close for dialogs
};


namespace GUI
{
  class Component;

  typedef std::vector<GUI::Component*>    tListComponents;
}



namespace GUI
{
  struct DragContentInfo
  {
    Component*          pComponentDragSource;
    Component*          pComponentDragContent;
    GR::up              DragUserData;
    GR::tPoint          DragOffset;


    DragContentInfo() :
      pComponentDragSource( NULL ),
      pComponentDragContent( NULL ),
      DragUserData( 0 )
    {
    }
  };

  struct OwnerDrawInfo
  {
    Component*                    pComponent;
    GR::tPoint                    ComponentOffset;
    int                           ItemIndex;
    GR::tPoint                    ItemOffset;
    GR::tPoint                    ItemSize;
    GUI::ComponentDisplayerBase*  pDisplayer;

    OwnerDrawInfo() :
      pComponent( NULL ),
      ItemIndex( 0 ),
      pDisplayer( NULL )
    {
    }
  };

  class ComponentContainer : public EventProducer<GUI::OutputEvent>, 
                             public EventListener<GUI::OutputEvent>
  {

    protected:

      Component*                        m_pActiveComponent;
      Component*                        m_pMouseOverComponent;
      ComponentContainer*               m_pParentContainer;
      Component*                        m_pCapturingComponent;
      Component*                        m_pModalComponent;
      Component*                        m_pFocusedComponent;


      GR::tRect                         m_ClientRect;

      bool                              m_ContainerProcessingDisabled;


      GR::u32                           m_LastMouseButtons;


      void                              DeleteDanglingControls( bool Recursive = true );

      std::list<ComponentEvent>         m_ComponentEventQueue;

      bool                              m_ShuttingDown;


    public:

      Component*                        m_pDraggingComponent;
      Component*                        m_pDraggingComponentContent;
      Component*                        m_pDraggingComponentSource;

      tListComponents                   m_Components;

      tListComponents                   m_ComponentsToDelete;


      ComponentContainer();
      virtual ~ComponentContainer();

      // Container-Funktionalität
      virtual void                      Add( Component *pComponent );
      virtual void                      Remove( Component *pComponent );
      virtual void                      RemoveAllControls();
      virtual void                      Delete( Component *pComponent );
      virtual void                      DeleteAllControls( bool bDeleteOnlyHidden = false );

      Component*                        GetComponent( GR::u32 Id );
      Component*                        GetComponentAt( const GR::tPoint& ptPosition, bool bRecursive = true, bool bIgnoreMouseIgnorer = false );

      Component*                        GetFirstChildComponent() const;
      Component*                        GetPrevChildComponent( Component* pComp ) const;
      Component*                        GetNextChildComponent( Component* pComp ) const;

      virtual void                      ToggleAllControlsVisibility();

      bool                              IsMouseOverComponent()
      {
        return ( m_pMouseOverComponent != NULL );
      }

      Component*                        MouseOverComponent()
      {
        return m_pMouseOverComponent;
      }

      virtual void                      EnableProcessing( bool bEnable = true );

      virtual ComponentContainer*       GetTopLevelParent();

      virtual void                      SetModalMode( Component* pComponent = NULL );
      virtual bool                      ModalDisabled( Component* pComponent );

      virtual void                      GenerateEvent( Component* pParentComponent, GR::u32 Id, OutputEventType oeType, 
                                                       GR::up dwParam1 = 0, GR::up dwParam2 = 0, 
                                                       GR::up dwParam3 = 0, GR::up dwParam4 = 0 );
      virtual void                      GenerateEventForParent( Component* pParentComponent, 
                                                                GR::u32 Id,
                                                                OutputEventType oeType, 
                                                                GR::up dwParam1 = 0, GR::up dwParam2 = 0, 
                                                                GR::up dwParam3 = 0, GR::up dwParam4 = 0 );

      void                              PostComponentEvent( const GUI::ComponentEvent& Event );
      void                              ProcessComponentEventQueue();

      virtual bool                      ProcessEvent( const GUI::ComponentEvent& Event );
      virtual bool                      ProcessEvent( const GUI::OutputEvent& Event );

      void                              SetCaptureToComponent( Component* pComponent );
      void                              ReleaseCapture();

      // Offset
      void                              ScreenToLocal( GR::tPoint& pt, Component* pComponent );
      void                              ScreenToNonClient( GR::tPoint& pt, Component* pComponent );
      void                              LocalToScreen( GR::tPoint& pt, Component* pComponent );
      void                              LocalToScreen( GR::tRect& rc, Component* pComponent );

      virtual void                      GetClientRect( GR::tRect& rectClient ) const;
      virtual GR::tRect                 GetClientRect() const;
      virtual GR::tPoint                GetClientOffset() const;

      void                              Dump( int iLevel );

      void                              ReplaceFont( Interface::IFont* pOldFont, Interface::IFont* pFont );

      // wird vom Component Displayer überschrieben
      virtual void                      SetFocus( Component* pComponent );
      virtual Component*                GetFocusedComponent();

      Component*                        GetModalComponent();

      Component*                        FindNextTabComponent( Component* pOldFocusedComponent, bool bPrevious = false );

  };

}


#endif // COMPONENT_CONTAINER_H



