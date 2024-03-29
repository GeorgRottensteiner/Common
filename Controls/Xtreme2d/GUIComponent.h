#ifndef GUI_COMPONENT_H
#define GUI_COMPONENT_H



#include <string>
#include <vector>

#include <Xtreme/X2dFont.h>
#include <Xtreme/XTextureSection.h>
#include <Xtreme/XMultiTexture.h>

#include <Controls/ComponentDisplayerBase.h>
#include <Controls/Component.h>



namespace Interface
{
  struct IFont;
}



class GUIComponentDisplayer;

class GUIComponent : public GUI::Component
{

  protected:

    GR::tRect                             m_Clipping;

    XMultiTexture*                        m_pBackground;


  public:


    GUIComponent( GR::u32 Id = 0 );
    GUIComponent( int X, int Y, int Width, int Height, GR::u32 Id = 0 );
    GUIComponent( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0 );

    virtual void                DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void                Display( GUI::ComponentDisplayerBase* pDisplayer );

    virtual void                DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );
    virtual void                DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer );

    virtual int                 GetBorderWidth( const GUI::eBorderType eType ) const;
    virtual int                 GetBorderHeight( const GUI::eBorderType eType ) const;

    void                        SetBackground( XMultiTexture* pBack );

    XTextureSection             Section( Xtreme::Asset::IAssetLoader* pLoader, const GR::String& Image );
    virtual void                ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );

};


#endif // GUI_COMPONENT_H



