#ifndef GUI_LABEL_H
#define GUI_LABEL_H



#include <string>
#include <vector>

#include <Controls/Component.h>
#include <Controls\AbstractLabel.h>
#include "GUIComponent.h"



class GR::Graphic::GFXPage;



class GUILabel : public AbstractLabel<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( GUILabel, "Label" )


    GUILabel( int X = 0, int Y = 0, 
              int Width = 0, int Height = 0, 
              const GR::String& Caption = GR::String(),
              GR::u32 Type = GUI::AF_VCENTER,
              GR::u32 Id = 0 );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_LABEL_H



