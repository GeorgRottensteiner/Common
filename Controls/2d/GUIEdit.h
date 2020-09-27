#ifndef GUI_EDIT_H
#define GUI_EDIT_H



#include <string>
#include <vector>

#include <Controls\AbstractEdit.h>
#include "GUIComponent.h"
#include "GUIScrollbar.h"
#include "GUISlider.h"
#include "GUIButton.h"



class GR::Graphic::GFXPage;



class GUIEdit : public AbstractEdit<GUIComponent, GUIScrollbar, GUISlider, GUIButton>
{

  public:

    DECLARE_CLONEABLE( GUIEdit, "Edit" )


    GUIEdit( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, 
              GR::u32 dwId = 0, GR::u32 efType = GUIEdit::ECS_DEFAULT );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_EDIT_H



