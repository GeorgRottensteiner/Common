#ifndef GUI_SCREEN_H
#define GUI_SCREEN_H



#include <Controls/Component.h>

#include "GUIComponent.h"



namespace GR
{
  namespace Graphic
  {
    class GFXPage;
  }
}



class GUIScreen : public GUIComponent
{

  public:

    DECLARE_CLONEABLE( GUIScreen, "Screen" )


    GUIScreen( int iNewX = 0, int iNewY = 0, 
                int iNewWidth = 0, int iNewHeight = 0, 
                DWORD dwId = 0 );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_SCREEN_H



