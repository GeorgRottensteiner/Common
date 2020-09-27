#ifndef GUI_COMPONENT_H
#define GUI_COMPONENT_H



#include <string>
#include <vector>

#include <Grafik/Font.h>
#include <Grafik/ContextDescriptor.h>

#include <Controls/ComponentDisplayerBase.h>
#include <Controls/Component.h>



namespace Interface
{
  struct IFont;
}



namespace GR
{
  namespace Graphic
  {
    class Image;
    class GFXPage;
  }
}



class GUIComponent : public GUI::Component
{

  protected:

    GR::Graphic::Image*         m_pImageBackground;


    GR::tRect                   m_rectClipping;

    GR::Graphic::ContextDescriptor  m_TextureSection[GUI::BT_EDGE_LAST_ENTRY];


  public:

    DECLARE_CLONEABLE( GUIComponent, "Component" )


    GUIComponent( GR::u32 dwId = 0 );
    GUIComponent( int iX, int iY, int iWidth, int iHeight, GR::u32 dwId = 0 );
    GUIComponent( int iX, int iY, int iWidth, int iHeight, const GR::String& strCaption, GR::u32 dwId = 0 );

    virtual void                DrawEdge( GR::Graphic::GFXPage* pPage, GR::u32 edgeType, GR::tRect& rectEdge );
    virtual void                DrawText( GR::Graphic::GFXPage* pPage, const char* szText,
                                          GR::tRect& rectText, GR::u32 tatType = GUI::AF_DEFAULT );
    virtual void                DrawFocusRect( GR::Graphic::GFXPage* pPage, const GR::tRect& rcFocus );
    virtual void                DisplayOnPage( GR::Graphic::GFXPage* pPage );
    virtual void                Display( GUI::ComponentDisplayerBase* pDisplayer );

    virtual void                DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage );
    virtual void                DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer );

    const GR::u32               GetSysColor( GUI::eColorIndex colIndex ) const;

    void                        SetBackground( GR::Graphic::Image* pImage = NULL );

    virtual int                 GetBorderWidth( const GUI::eBorderType eType ) const;
    virtual int                 GetBorderHeight( const GUI::eBorderType eType ) const;

    void                        SetTextureSection( const GUI::eBorderType eType, GR::Graphic::ContextDescriptor& CDSection );

};


#endif // GUI_COMPONENT_H



