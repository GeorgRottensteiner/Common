#ifndef GUI_COMPONENT_H
#define GUI_COMPONENT_H



#include <string>
#include <vector>

#include <Xtreme/XRenderer.h>
#include <Xtreme/XTextureSection.h>

#include <Controls\ComponentDisplayerBase.h>
#include <Controls/Component.h>



class GUIComponentDisplayer;

class GUIComponent : public GUI::Component
{

  private:

    XTextureSection             Section( Xtreme::Asset::IAssetLoader* pLoader, const GR::String& Image );






  public:

    DECLARE_CLONEABLE( GUIComponent, "GUI.Component" )



    GUIComponent( GR::u32 Id = 0 );
    GUIComponent( int X, int Y, int Width, int Height, GR::u32 Id = 0 );
    GUIComponent( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0 );

    virtual void                Display( GUI::ComponentDisplayerBase* pDisplayer );
    virtual void                DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual void                DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer );
    virtual void                DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );


    // Helper
    virtual void                PrepareStatesForAlpha( GUIComponentDisplayer& Displayer, GR::u32 Color );

    virtual void                SetFont( Interface::IFont* pFont );

    virtual void                AdjustRect( GR::tPoint& Size, bool ClientToNC );
    virtual void                RecalcClientRect();

    GR::u32                     GetSysColor( GUI::eColorIndex Color );

    virtual int                 GetBorderWidth( const GUI::eBorderType Type ) const;
    virtual int                 GetBorderHeight( const GUI::eBorderType Type ) const;

    virtual void                ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );

    
};


#endif // __GUI_BUTTON_H__



