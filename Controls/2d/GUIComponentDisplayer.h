#ifndef GUI_COMPONENT_DISPLAYER_H
#define GUI_COMPONENT_DISPLAYER_H



#include <Controls/Component.h>
#include <Controls/ComponentDisplayerBase.h>

#include <Grafik/GfxPage.h>
#include <Grafik/ContextDescriptor.h>

#include "GUIComponent.h"

#include <string>
#include <vector>



#define GUI_IMPLEMENT_CLONEABLE( xClass, strClassName ) class C##xClass##StaticRegistrar \
                                      { \
                                        public: \
                                          C##xClass##StaticRegistrar() \
                                          {\
                                          GUIComponentDisplayer::Instance().RegisterObject( strClassName, xClass::CreateNewInstance ); \
                                          }\
                                      };\
                                      C##xClass##StaticRegistrar register##xClass##;

namespace GR
{
  namespace Graphic
  {
    class GFXPage;
  }
}



class GUIComponentDisplayer : public GUI::ComponentDisplayerBase
{

  protected:

    GR::tRect                       m_Clipping;

    GR::Graphic::ContextDescriptor  m_DefaultTextureSection[GUI::BT_EDGE_LAST_ENTRY];


    GUIComponentDisplayer();


  public:

    GR::Graphic::GFXPage*           m_pPage;

    static GUIComponentDisplayer&   Instance();

    virtual ~GUIComponentDisplayer();

    

    virtual void                    PushClipValues();
    virtual void                    SetClipping( int iX1, int iY1, int iX2, int iY2 );
    virtual void                    SetOffset( int iX, int iY );
    virtual GR::tPoint              GetOffset();

    void                            SetDefaultTextureSection( GUI::eBorderType dwType, GR::Graphic::ContextDescriptor& CDSection );

    friend class GUIComponent;

    virtual void DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 );
    virtual void DrawRect( int X, int Y, int Width, int Height, GR::u32 Color );
    virtual void DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color );
    virtual void DrawTextureSection( int X,
                                     int Y,
                                     const XTextureSection& TexSection,
                                     GR::u32 Color = -1,
                                     int AlternativeWidth = -1,
                                     int AlternativeHeight = -1,
                                     GR::u32 AlternativeFlags = -1 );
    virtual void DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color = 0xffffffff );
    virtual void DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color = 0xffffffff );
    virtual void DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color = 0xffffffff );

    virtual void DrawText( Interface::IFont* pFont,
                           const GR::String& Text,
                           const GR::tRect& Rect,
                           GR::u32 TextAlignment = GUI::AF_DEFAULT,
                           GR::u32 Color = 0xffffffff );
    virtual void DrawText( Interface::IFont* pFont,
                           int XOffset, int YOffset,
                           const GR::String& Text,
                           GR::u32 TextAlignment = GUI::AF_DEFAULT,
                           GR::u32 Color = 0xffffffff,
                           const GR::tRect* pRect = NULL );

    virtual void DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge );
    virtual void DrawEdge( GR::u32 edgeType, const GR::tRect& rectEdge, const std::vector<std::pair<XTextureSection, GR::u32> >& Sections );
    virtual void DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 = 0 );


};



#endif // GUI_COMPONENT_DISPLAYER_H



