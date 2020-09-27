#ifndef COMPONENT_DISPLAYER_BASE_H
#define COMPONENT_DISPLAYER_BASE_H

#include <OS/OS.h>

#include <string>
#include <vector>
#if OPERATING_SYSTEM != OS_ANDROID
#include <chrono>
#endif

#include <Controls/ComponentContainer.h>
#include <Controls/Component.h>

#include <Lang/Service.h>
#include <Interface/IRenderFrame.h>

#include <Grafik/GFXHelper.h>

#include <debug/debugclient.h>

#include <Xtreme/XInput.h>

#include <Misc/CloneFactory.h>

#include <Lang/FastDelegate.h>

#include <String/XML.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetFont.h>

#undef DrawText



namespace Interface
{
  struct IFont;
}


namespace GUI
{
  void WrapText( Interface::IFont* pFont, const GR::String& strText, GR::tRect& rectText, std::vector<GR::String>& vectText, bool bCalcRect = false );
  void BreakText( Interface::IFont* pFont, const GR::String& strText, GR::tRect& rectResult, std::vector<GR::String>& vectText, bool bCalcRect = false );
  GR::tPoint TextOffset( Interface::IFont* pFont, const GR::String& strText, GR::u32 textAlignment, const GR::tRect& Rect );
  GR::u64 GetTicks();
  GR::u64 GetDoubleClickTimeMS();



  class ComponentDisplayerBase : public GUI::ComponentContainer, 
                                 public CloneFactory, 
                                 public EventListener<Xtreme::tInputEvent>, 
                                 public GR::Service::Service
  {

    public:

      typedef fastdelegate::FastDelegate2<GR::Strings::XMLElement*,Component*>     tElementCreatedFromXMLListener;


      void*                 m_pRenderer;


    protected:

      typedef std::map<GR::u32, std::pair<XTextureSection, GR::u32> > tMapCustomTextureSections;

      tMapCustomTextureSections                     m_CustomTextureSections;

      std::pair<XTextureSection, GR::u32>           m_DefaultTextureSection[GUI::BT_EDGE_LAST_ENTRY];

      std::list<tElementCreatedFromXMLListener>     m_ElementCreatedListeners;

      Component*                                    m_pCurrentlyDrawnComponent;


    public:
      std::list<GR::tRect>  m_ClipRects;
    protected:

      std::list<GR::tPoint> m_Offsets;

      float                 m_ElapsedTimeSinceLastFrame;

      Interface::IFont*     m_pDefaultGUIFont;

      Xtreme::XInput*       m_pInputClass;

      bool                  m_Suspending,
                            m_NothingIsVisible;

      GR::u32               m_Colors[GUI::COL_LAST_ENTRY];

      GR::tPoint            m_PhysicalSize;
      GR::tPoint            m_VirtualSize;

      // only applied to input
      GR::tRect             m_ExternalArea;

      static ComponentDisplayerBase* s_pInstance;



      void RetrieveDefaultAssets( Xtreme::Asset::XAssetLoader* pLoader );
      
      virtual bool OnServiceNotify( const char* Event, GR::IService* pServiceSender );

      void OnFrameEvent( const GR::tRenderFrameEvent& Event );

      virtual void          OnServiceGotSet( const char* Service, GR::IService* pService );
      virtual void          OnServiceGotUnset( const char* Service, GR::IService* pService );
      virtual void          OnSet();
      virtual void          OnUnset();



    public:



      ComponentDisplayerBase();
      virtual ~ComponentDisplayerBase();



      GR::u32 DefaultSysColor( GR::u32 ColorIndex );

      bool NothingIsVisible();

      void AddElementCreatedFromXMLListener( tElementCreatedFromXMLListener Listener );
      void RemoveElementCreatedFromXMLListener( tElementCreatedFromXMLListener Listener );

      void Suspend( bool Suspend = true );

      void SysColor( GUI::eColorIndex colIndex, GR::u32 dwColor );
      GR::u32 GetSysColor( GUI::eColorIndex colIndex );

      static ComponentDisplayerBase& Instance();

      virtual void          SetClipping( int iX, int iY, int iWidth, int iHeight ) = 0;
      virtual void          SetOffset( int iX, int iY ) = 0;
      virtual GR::tPoint    GetOffset() = 0;
      virtual void          PushClipValues() = 0;
      virtual void          StoreClipValues( int iX, int iY, int iWidth, int iHeight, int iXOffset, int iYOffset );
      virtual void          PopClipValues();
      virtual void          GetStoredClipValues( int& iX, int& iY, int& iWidth, int& iHeight, int& iXOffset, int& iYOffset );

      virtual void DeleteComponent( Component* pComponent );
      virtual void DeleteComponent( GR::u32 dwId );
      virtual void DisplayComponent( Component* pComponent );
      virtual void DisplayComponentFull( Component* pComponent, const GR::tPoint& Offset, const GR::tRect& ParentClipRect );

      virtual void DisplayChilds( Component* pParentComponent );

      void SetExtents( int Width, int Height );

      void NotifyComponent( Component* pComponent, const GUI::ComponentEvent& Event );
      void NotifyAllComponents( const GUI::ComponentEvent& Event );

      void DisplayComponent( float fElapsedTime, Component* pRootComponent );
      void UpdateComponent( GUI::Component* pComponent, const float fElapsedTime );
      void UpdateAllControls( const float fElapsedTime );

      void DisplayAllControls();
      void DisplayComponentFromParent( Component* pParentComponent );
      
      virtual void ProcessActions();

      GR::tPoint ScreenToVirtual( const GR::tPoint& ScreenCoordinates );
      void ScreenToVirtual( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height );
      GR::tPoint VirtualToScreen( const GR::tPoint& VirtualCoordinates );
      void VirtualToScreen( GR::tRect& VirtualCoordinates );
      void VirtualToScreen( GR::i32& X, GR::i32& Y, GR::i32& Width, GR::i32& Height );



      int TextLength( Interface::IFont* pFont, const GR::String& Text );
      int TextHeight( Interface::IFont* pFont, const GR::String& Text );

      virtual bool ProcessEvent( const Xtreme::tInputEvent& Event );

      bool IsVisible( Component* pComponent );
      virtual bool ProcessEvent( const GUI::ComponentEvent& Event );

      void SetDefaultFont( Interface::IFont* pFont );
      Interface::IFont* DefaultFont() const;

      Component* ComponentFromXML( GR::Strings::XMLElement* pElement );
      GR::Strings::XMLElement* ComponentToXML( Component* pComponent );

      Component* FindNextTabComponentByDir( GUI::ComponentContainer* pParentComponent, Component* pOldFocusedComponent, GR::u16 wChar );

      void SetScreenSizes( const GR::tPoint& PhysicalSize, const GR::tPoint& VirtualSize = GR::tPoint() );

      GR::tPoint VirtualSize() const;
      GR::tPoint PhysicalSize() const;

      void SetExternalArea( const GR::tRect& Area );

      void SetDefaultTextureSection( GUI::eBorderType Type, const XTextureSection& Section, GR::u32 ColorKey = 0 );
      void SetCustomTextureSection( const GR::u32 Type, const XTextureSection& Section, GR::u32 ColorKey = 0 );


      virtual void DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;
      virtual void DrawRect( int X, int Y, int Width, int Height, GR::u32 Color ) = 0;
      virtual void DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color ) = 0;
      virtual void DrawTextureSection( int X,
                               int Y,
                               const XTextureSection& TexSection,
                               GR::u32 Color = -1,
                               int AlternativeWidth = -1,
                               int AlternativeHeight = -1,
                               GR::u32 AlternativeFlags = -1 ) = 0;
      virtual void DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color = 0xffffffff ) = 0;
      virtual void DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color = 0xffffffff ) = 0;
      virtual void DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color = 0xffffffff ) = 0;

      virtual void DrawText( Interface::IFont* pFont,
                     const GR::String& Text,
                     const GR::tRect& Rect,
                     GR::u32 TextAlignment = GUI::AF_DEFAULT,
                     GR::u32 Color = 0xffffffff ) = 0;
      virtual void DrawText( Interface::IFont* pFont,
                     int XOffset, int YOffset,
                     const GR::String& Text,
                     GR::u32 TextAlignment = GUI::AF_DEFAULT,
                     GR::u32 Color = 0xffffffff,
                     const GR::tRect* pRect = NULL ) = 0;

      virtual void DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge ) = 0;
      virtual void DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge, const std::vector<std::pair<XTextureSection, GR::u32> >& Sections ) = 0;
      virtual void DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 = 0 ) = 0;

  };

}




#endif // COMPONENT_DISPLAYER_BASE_H



