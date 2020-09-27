#ifndef GUI_RADIOBUTTON_H
#define GUI_RADIOBUTTON_H



#include <string>
#include <vector>

#include <Controls\AbstractRadioButton.h>
#include "GUIComponent.h"



namespace GR
{
  namespace Graphic
  {
    class GFXPage;
  };
};



class CGUIRadioButton : public CAbstractRadioButton<CGUIComponent>
{

  public:

    DECLARE_CLONEABLE( CGUIRadioButton, "RadioButton" )


    CGUIRadioButton( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwID = 0 );
    CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwID = 0 );

    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_RADIOBUTTON_H



