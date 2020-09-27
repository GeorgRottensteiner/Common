#ifndef GUI_SCREEN_H
#define GUI_SCREEN_H



#include <string>
#include <vector>

#include <Xtreme/XHugeTexture.h>

#include "GUIComponent.h"



class GUIScreen : public GUIComponent
{

  protected:



  public:

    DECLARE_CLONEABLE( GUIScreen, "Screen" )


    GR::String                  m_Image;

    XHugeTexture                m_Background;



    GUIScreen( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 Id = 0 );


    virtual void                ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );


    virtual void                DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );
};


#endif // __GUI_SCREEN_H__



