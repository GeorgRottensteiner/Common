#ifndef GUI_COMPONENT_DISPLAYER_H
#define GUI_COMPONENT_DISPLAYER_H



#include <Xtreme/XRenderer.h>
#include <Xtreme/XTextureSection.h>
#include <Xtreme/XHugeTexture.h>

#include <Controls/Component.h>
#include <Controls/ComponentDisplayerBase.h>

#include "GUIComponent.h"

#include <string>
#include <vector>



#define GUI_IMPLEMENT_CLONEABLE( xClass, strClassName ) class C##xClass##StaticRegistrar \
                                      { \
                                        public: \
                                          C##xClass##StaticRegistrar() \
                                          {\
                                          GUIComponentDisplayer::Instance().RegisterObject( strClassName, xClass::CreateNewInstance, false ); \
                                          }\
                                      };\
                                      C##xClass##StaticRegistrar register##xClass;



class GUIComponentDisplayer : public GUI::ComponentDisplayerBase
{

  protected:


    GR::tRect                       m_Clipping;
    GR::tPoint                      m_CurrentOffset;

    XViewport                       m_DisplayViewport;


  public:

    XRenderer*                      m_pActualRenderer;

    XHugeTexture                    m_Background;

    IEventProducer<GR::Gamebase::tXFrameEvent>*     m_pEventProducer;



    GUIComponentDisplayer();
    virtual ~GUIComponentDisplayer();


    virtual void                    DisplayBackground();
    virtual void                    DisplayAllControls();

    virtual void                    SetClipping( int iX1, int iY1, int iX2, int iY2 );
    virtual void                    SetOffset( int iX, int iY );
    virtual GR::tPoint              GetOffset();
    virtual void                    PushClipValues();

    void                            SetViewport( const XViewport& Viewport );
    XViewport                       Viewport();

    void                            SetRenderer( XRenderer* pRenderer );

    virtual void                    DrawEdge( GR::u32 VisualStyle, const GR::tRect& rectEdge );
    virtual void                    DrawEdge( GR::u32 VisualStyle,
                                              const GR::tRect& rectEdge, 
                                              const std::vector<std::pair<XTextureSection, GR::u32> >& Sections );


    virtual void                    DrawText( Interface::IFont* pFont,
                                              int XOffset, int YOffset,
                                              const GR::String& Text,
                                              GR::u32 TextAlignment = GUI::AF_DEFAULT,
                                              GR::u32 Color = 0xffffffff,
                                              const GR::tRect* pRect = NULL );

    virtual void                    DrawText( Interface::IFont* pFont,
                                              const GR::String& Text,
                                              const GR::tRect& Rect,
                                              GR::u32 TextAlignment = GUI::AF_DEFAULT,
                                              GR::u32 Color = 0xffffffff );

    virtual void                    DrawTextScaled( Interface::IFont* pFont,
                                                    int XOffset, int YOffset,
                                                    const GR::String& Text,
                                                    GR::u32 TextAlignment = GUI::AF_DEFAULT,
                                                    GR::u32 Color = 0xffffffff,
                                                    GR::tRect* pRect = NULL,
                                                    GR::f32 ScaleFaktor = 1.0f );

    virtual void                    DrawFocusRect( const GUIComponent& Component, const GR::tRect& rc );


    virtual void                    DrawTextureSection( int X, 
                                                        int Y, 
                                                        const XTextureSection& TexSection, 
                                                        GR::u32 Color = -1, 
                                                        int AlternativeWidth = -1, 
                                                        int AlternativeHeight = -1, 
                                                        GR::u32 AlternativeFlags = -1 );

    virtual void                    DrawTiled( int OffsetX,
                                       int OffsetY, 
                                       const GR::tRect& Rect, 
                                       XTextureSection Section, 
                                       GR::u32 Color );

    virtual void                    DrawTextureSectionHRepeat( int X, int Y, int Width, const XTextureSection& Section, GR::u32 Color );
    virtual void                    DrawTextureSectionVRepeat( int X, int Y, int Height, const XTextureSection& Section, GR::u32 Color );
    virtual void                    DrawTextureSectionHVRepeat( int X, int Y, int Width, int Height, const XTextureSection& Section, GR::u32 Color );

    virtual void                    DrawQuad( int X, int Y, int Width, int Height, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 );
    virtual void                    DrawRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color );
    virtual void                    DrawRect( int X, int Y, int Width, int Height, GR::u32 Color );
    virtual void                    DrawLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color, GR::u32 Color2 = 0 );

    int                             FontSpacing( Interface::IFont* pFont );
    int                             TextHeight( Interface::IFont* pFont );
    int                             TextHeight( Interface::IFont* pFont, const GR::String& Text );
    int                             TextLength( Interface::IFont* pFont, const GR::String& Text );



    static GUIComponentDisplayer&   Instance();

    friend class GUIComponent;
};



#endif // GUI_COMPONENT_DISPLAYER_H



