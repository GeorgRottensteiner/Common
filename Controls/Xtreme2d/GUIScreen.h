#ifndef GUI_SCREEN_H
#define GUI_SCREEN_H



#include <string>
#include <vector>

#include "GUIComponent.h"



class GUIScreen : public GUIComponent
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIScreen, "Screen" )


    GR::String               m_Image;



    GUIScreen( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );


    virtual void              ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );


};


#endif // GUI_SCREEN_H



