#ifndef COMPONENT_H
#define COMPONENT_H



#include <Controls/InputEvent.h>
#include <Controls/ComponentContainer.h>
#include <Controls/ComponentEnums.h>
#include <Controls/OutputEvent.h>
#include <Controls/QueryEvent.h>

#include <Interface/ICloneAble.h>
#include <Interface/IEventListener.h>
#include <Interface/IService.h>

#include <Lang/FastDelegate.h>

#include <Xtreme/XTextureSection.h>

#include <string>
#include <list>
#include <map>
#include <vector>



namespace GR
{
  namespace Strings
  {
    class XMLElement;
  }
}



namespace Interface
{
  struct IFont;
}



namespace GUI
{
  class ComponentDisplayerBase;



  class Component : public ComponentContainer, 
                    public ICloneAble, 
                    public EventListener<GUI::ComponentEvent>
  {

    public:

      DECLARE_CLONEABLE( Component, "Component" )

      typedef fastdelegate::FastDelegate1<const GUI::OutputEvent&> tEventHandlerFunction;
      typedef fastdelegate::FastDelegate1<GUI::QueryEvent&> tQueryEventHandlerFunction;

      typedef std::list<tEventHandlerFunction>              tEventHandlers;
      typedef std::list<tQueryEventHandlerFunction>         tQueryEventHandlers;

      typedef std::map<GR::u32,tEventHandlers>              tMapEventHandler;
      typedef std::map<GR::u32,tQueryEventHandlers>         tMapQueryEventHandler;

    protected:

      typedef std::map<GR::u32, std::pair<XTextureSection, GR::u32> >   tMapCustomTextureSections;

      std::vector<std::pair<XTextureSection, GR::u32> >     m_TextureSection;

      tMapCustomTextureSections                             m_CustomTextureSections;



      GR::u32                 m_VisualStyle;

      GR::u32                 m_Style;

      GR::ip                  m_UserData;
      GR::String              m_UserText;

      GR::tPoint              m_Position,
                              m_DragPos,
                              m_DoubleTextOffset;

      int                     m_Width,
                              m_Height;

      GR::u32                 m_ComponentFlags;

      GR::String              m_Caption;
      GR::String              m_DefaultEventScript;

      GR::u32                 m_ID;
      GR::u32                 m_HottipID;

      GR::u32                 m_TextAlignment;

      GR::u32                 m_Colors[GUI::COL_LAST_ENTRY];
      bool                    m_UseCustomColors[GUI::COL_LAST_ENTRY];

      GR::up                  m_ItemData;

      GR::tPoint              m_MouseDownPos;

      Component*              m_pParent;
      Component*              m_pOwner;

      tMapEventHandler        m_EventHandler;

      tMapQueryEventHandler   m_QueryEventHandler;

      void                    ParseColorFromAttribute( GUI::eColorIndex Color, const GR::String& AttributeName, GR::Strings::XMLElement* pElement );



    public:

      Interface::IFont*       m_pFont;



      Component( GR::u32 Id = 0 );
      Component( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id = 0 );
      Component( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Id = 0 );
      virtual ~Component();

      // Container-Funktionalität
      void Add( Component *pComponent );
      void Remove( Component *pComponent );
      void RemoveAllControls();

      void                  SetUserData( GR::up dwData );
      GR::up                GetUserData() const;

      void                  SetUserText( const GR::String& Text );
      GR::String            GetUserText() const;


      virtual Component*    GetComponent( GR::u32 Id );
      void                  AddHandler( OutputEventType Type, tEventHandlerFunction funcHandler );
      void                  RemoveHandler( OutputEventType Type, tEventHandlerFunction funcHandler );
      void                  RaiseHandlerEvent( const GUI::OutputEvent& Event );

      using ComponentContainer::GenerateEvent;
      using ComponentContainer::GenerateEventForParent;
      virtual void GenerateEvent( OutputEventType Type,
                                  GR::up Param1 = 0, GR::up Param2 = 0, 
                                  GR::up Param3 = 0, GR::up Param4 = 0 );
      virtual void GenerateEventForParent( OutputEventType Type,
                                           GR::up Param1 = 0, GR::up Param2 = 0, 
                                           GR::up Param3 = 0, GR::up Param4 = 0 );

      void                  RecursiveChildEvent( OutputEventType Event );

      virtual bool          ProcessEvent( const GUI::ComponentEvent& Event );
      bool                  DefaultEventProc( const GUI::ComponentEvent& Event );

      void                  AddQueryHandler( GUI::QueryEventType::Value Type, tQueryEventHandlerFunction funcHandler );
      void                  RemoveQueryHandler( GUI::QueryEventType::Value Type, tQueryEventHandlerFunction funcHandler );
      void                  QueryEvent( GUI::QueryEvent& Event );


      int                   Width() const;
      int                   Height() const;
      GR::tPoint            Position() const;
      GR::tPoint            Size() const;
      GR::tPoint            DragPosition() const;
      virtual GR::String    GetCaption() const;
      Component*            GetComponentParent() const;
      ComponentContainer*   GetParentContainer() const;
      void                  GetComponentRect( GR::tRect& rectComp ) const;
      void                  Center( ComponentContainer* pParent = NULL );

      void                  ScreenToLocal( GR::tPoint& Pos );
      void                  ScreenToNonClient( GR::tPoint& Pos );
      void                  LocalToScreen( GR::tPoint& Pos );
      void                  LocalToScreen( GR::tRect& Rect );


      Component*            Owner() const;
      void                  SetOwner( Component* pNewOwner );

      void                  SetModal();

      void                  ModifyVisualStyle( GR::u32 Add, GR::u32 Remove = 0 );
      GR::u32               VisualStyle() const;

      virtual GR::u32       ModifyStyle( GR::u32 Add, GR::u32 Remove = 0 );
      GR::u32               Style() const;

      virtual GR::u32       ModifyFlags( GR::u32 Add, GR::u32 Remove = 0 );
      GR::u32               Flags() const;

      virtual void          TextAlignment( GR::u32 Alignment );
      GR::u32               TextAlignment() const;

      virtual void          AdjustRect( GR::tPoint& Size, bool ClientToNC );
      virtual void          RecalcClientRect();

      virtual void          Update( float ElapsedTime );
      

      void                  SetVisible( bool Visible = true );
      bool                  IsVisible();
      bool                  IsEnabled();
      bool                  IsFocused();
      bool                  IsActive();
      bool                  IsDestroyed();
      bool                  IsIndirectDisabled();
      bool                  IsIndirectHidden();

      virtual bool          IsMouseInside();
      virtual bool          IsMouseInside( const GR::tPoint& MousePos );

      virtual bool          IsMouseInsideNonClientArea( const GR::tPoint& MousePos );

      void                  SetLocation( int NewX, int NewY );
      virtual void          SetSize( int NewWidth, int NewHeight );
      void                  SetClientSize( int NewWidth, int NewHeight );
      virtual void          SetCaption( const GR::Char* Caption );
      virtual void          SetCaption( const GR::String& Caption );

      virtual int           GetBorderWidth( const GUI::eBorderType Type ) const;
      virtual int           GetBorderHeight( const GUI::eBorderType Type ) const;


      using ComponentContainer::SetFocus;
      void                  SetFocus();

      void                  GetWindowRect( GR::tRect& Rect ) const;
      GR::tRect             GetWindowRect() const; 
      
      void                  Close( const GR::u32 CloseCode = 0 );
      void                  PerformClose( const GR::u32 CloseCode );

      void                  Enable( bool Enable = true );
      GR::u32               Id() const;
      void                  Id( GR::u32 NewId );
      GR::u32               HottipId() const;
      void                  HottipId( GR::u32 NewId );

      bool                  StartDragging();
      void                  StopDragging();

      bool                  IsIndirectChild( ComponentContainer* pContainer );

      virtual void          ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );
      virtual void          PropertiesToXML( GR::Strings::XMLElement* pElement );

      // Font-spezifisch
      virtual void          SetFont( Interface::IFont* pFont );

      // Farben
      GR::u32               GetColor( GUI::eColorIndex Index ) const;
      GR::u32               GetSysColor( GUI::eColorIndex colIndex ) const;
      bool                  UseCustomColor( GUI::eColorIndex Index ) const;
      void                  SetColor( GUI::eColorIndex Index, const GR::u32 Color, bool IsDefaultColor = false );
      void                  SetBaseColors();

      void                  SetCapture();

      friend class ComponentContainer;
      friend class ComponentDisplayerBase;

      void                  RaiseDefaultEvent();

      void                  Invalidate();


      virtual void          Display( ComponentDisplayerBase* pDisplayer );
      virtual void          DisplayNonClient( ComponentDisplayerBase* pDisplayer );

      void                  SetTextureSection( const GUI::eBorderType eType, const XTextureSection& Section, GR::u32 ColorKey = 0xff000000 );
      virtual void          SetCustomTextureSection( const GR::u32 Type, const XTextureSection& TexSection, GR::u32 ColorKey = 0xff000000 );

      XTextureSection       TextureSection( const GUI::eBorderType Type ) const;
      XTextureSection       CustomTextureSection( const GR::u32 Type ) const;
      GR::u32               CustomTSColorKey( const GR::u32 Type ) const;

      static bool           IsInputEvent( GR::u32 ComponentEventType );

  };

}

#endif // COMPONENT_H



