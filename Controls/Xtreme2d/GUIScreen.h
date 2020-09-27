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



    GUIScreen( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );


    virtual void              ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );


};


#endif // GUI_SCREEN_H



