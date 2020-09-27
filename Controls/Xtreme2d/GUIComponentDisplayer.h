#ifndef GUI_COMPONENT_DISPLAYER_H
#define GUI_COMPONENT_DISPLAYER_H


#include <Controls/Component.h>
#include <Controls\ComponentDisplayerBase.h>
#include <Xtreme/XTextureSection.h>
#include <Xtreme/XHugeTexture2d.h>

#include <Xtreme/X2dRenderer.h>

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
                                      C##xClass##StaticRegistrar register##xClass \
                                      ;

class GUIComponentDisplayer : public GUI::ComponentDisplayerBase
{

  protected:

    GR::tRect                       m_rectClipping;

    
    GUIComponentDisplayer();


  public:

    int                             m_iOffsetX,
                                    m_iOffsetY;

    X2dRenderer*                    m_pActualRenderer;

    XHugeTexture                    m_Background;

    IEventProducer<GR::Gamebase::tXFrameEvent>*     m_pEventProducer;



    static GUIComponentDisplayer&   Instance();

    virtual ~GUIComponentDisplayer();



    virtual void                    DisplayBackground();

    virtual void                    PushClipValues();
    virtual void                    SetClipping( int iX1, int iY1, int iX2, int iY2 );
    virtual void                    SetOffset( int iX, int iY );
    virtual GR::tPoint              GetOffset();

    void SetRenderer( X2dRenderer* pRenderer );

    friend class GUIComponent;


    void DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 );
    void DrawRect( int X, int Y, int Width, int Height, GR::u32 Color );
    void DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color );
    void DrawTextureSection( int X,
                             int Y,
                             const XTextureSection& TexSection,
                             GR::u32 Color = -1,
                             int AlternativeWidth = -1,
                             int AlternativeHeight = -1,
                             GR::u32 AlternativeFlags = -1 );
    void DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color = 0xffffffff );
    void DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color = 0xffffffff );
    void DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color = 0xffffffff );

    void DrawText( Interface::IFont* pFont,
                   const GR::String& Text,
                   const GR::tRect& Rect,
                   GR::u32 TextAlignment = GUI::AF_DEFAULT,
                   GR::u32 Color = 0xffffffff );
    void DrawText( Interface::IFont* pFont,
                   int XOffset, int YOffset,
                   const GR::String& Text,
                   GR::u32 TextAlignment = GUI::AF_DEFAULT,
                   GR::u32 Color = 0xffffffff,
                   const GR::tRect* pRect = NULL );

    void DrawEdge( GR::u32 VisualStyle, const GR::tRect& rectEdge );
    void DrawEdge( GR::u32 VisualStyle, const GR::tRect& rectEdge, const std::vector<std::pair<XTextureSection, GR::u32> >& Sections );
    void DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 = 0 );

    void DrawFocusRect( const GR::tRect& rcFocus, GR::u32 VisualStyle = 0 );

};



#endif // GUI_COMPONENT_DISPLAYER_H



