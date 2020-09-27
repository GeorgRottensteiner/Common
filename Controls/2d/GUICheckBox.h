#ifndef GUI_CHECKBOX_H
#define GUI_CHECKBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractCheckBox.h>
#include "GUIComponent.h"



class GR::Graphic::GFXPage;



class GUICheckBox : public AbstractCheckBox<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( GUICheckBox, "CheckBox" )


    GUICheckBox( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwID = 0 );
    GUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwID = 0 );

    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_CHECKBOX_H



