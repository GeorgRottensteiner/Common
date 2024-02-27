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


    GUIEdit( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 EditFlags = 0, GR::u32 Id = 0 );

    virtual void        DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_EDIT_H



