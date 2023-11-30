#ifndef GUI_DIALOG_H
#define GUI_DIALOG_H



#include <Controls\AbstractDialog.h>
#include "GUIComponent.h"



class GR::Graphic::GFXPage;



class GUIDialog : public AbstractDialog<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( GUIDialog, "Dialog" )


    GUIDialog( int iNewX = 0, int iNewY = 0, 
                int iNewWidth = 0, int iNewHeight = 0, 
                const GR::String& strCaption = GR::String(),
                GR::u32 dwId = 0, 
                GR::u32 dfType = DS_CAPTION | DS_CLIPDRAG );


    virtual void        DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage );
    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_DIALOG_H



