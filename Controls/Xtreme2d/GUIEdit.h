#ifndef GUI_EDIT_H
#define GUI_EDIT_H



#include <string>
#include <vector>

#include <Controls\AbstractEdit.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"
#include "GUISlider.h"
#include "GUIButton.h"



class GUIEdit : public AbstractEdit<GUIComponent, GUIScrollBar, GUISlider, GUIButton>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIEdit, "Edit" )


    GUIEdit( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 efType = 0, GR::u32 dwId = 0 );

    virtual void        DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_EDIT_H



