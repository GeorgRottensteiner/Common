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


    GUILabel( int iNewX = 0, int iNewY = 0, 
              int iNewWidth = 0, int iNewHeight = 0, 
              const GR::UTF8String& strCaption = GR::UTF8String(),
              GR::u32 lfType = GUI::AF_VCENTER,
              GR::u32 dwId = 0 );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_LABEL_H



